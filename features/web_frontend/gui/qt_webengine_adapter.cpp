#include "qt_webengine_adapter.h"

#include "features/framework/core/local_file_selection_service.h"
#include "features/image/core/image_processing_service.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QChildEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWidget>

#include <utility>

#ifndef NDEBUG
#include <QProcessEnvironment>
#endif

namespace {
bool isDevelopmentHttpUrl(const QUrl &url)
{
    return url.scheme() == QStringLiteral("http") && url.host() == QStringLiteral("127.0.0.1") &&
           url.port() == 5173;
}

bool isDevelopmentWebSocketUrl(const QUrl &url)
{
    return url.scheme() == QStringLiteral("ws") && url.host() == QStringLiteral("127.0.0.1") &&
           url.port() == 5173;
}

bool isBundledResourceUrl(const QUrl &url)
{
    const QString path = url.path();
    return url.scheme() == QStringLiteral("qrc") &&
           (path.startsWith(QStringLiteral("/frontend/")) ||
            path.startsWith(QStringLiteral("/qtwebchannel/")) ||
            path == QStringLiteral("/logo/dev-tools_logo.png"));
}

class FrontendRequestInterceptor final : public QWebEngineUrlRequestInterceptor
{
public:
    explicit FrontendRequestInterceptor(bool developmentEnabled, QObject *parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent), development_enabled_(developmentEnabled)
    {}

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        const QUrl url = info.requestUrl();
        if (isBundledResourceUrl(url)) {
            return;
        }
        if (development_enabled_ && (isDevelopmentHttpUrl(url) || isDevelopmentWebSocketUrl(url))) {
            return;
        }
        info.block(true);
    }

private:
    bool development_enabled_;
};

class FrontendPage final : public QWebEnginePage
{
public:
    FrontendPage(QWebEngineProfile *profile, bool developmentEnabled, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent), development_enabled_(developmentEnabled)
    {}

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType navigationType,
                                 bool isMainFrame) override
    {
        Q_UNUSED(navigationType);
        if (!isMainFrame) {
            return url == QUrl(QStringLiteral("about:srcdoc"));
        }
        if (url.scheme() == QStringLiteral("qrc") && url.host().isEmpty()) {
            return url.path() == QStringLiteral("/frontend/index.html");
        }
        return development_enabled_ && isDevelopmentHttpUrl(url);
    }

    QWebEnginePage *createWindow(WebWindowType windowType) override
    {
        Q_UNUSED(windowType);
        return nullptr;
    }

private:
    bool development_enabled_;
};

bool developmentServerEnabled()
{
#ifndef NDEBUG
    const QByteArray configured = QProcessEnvironment::systemEnvironment()
                                      .value(QStringLiteral("DEVTOOLS_WEB_UI_DEV_SERVER_URL"))
                                      .toUtf8();
    return configured == QByteArrayLiteral("http://127.0.0.1:5173");
#else
    return false;
#endif
}

QUrl initialFrontendUrl(bool developmentEnabled)
{
    if (developmentEnabled) {
#ifndef NDEBUG
        return {QStringLiteral("http://127.0.0.1:5173")};
#endif
    }
    return {QStringLiteral("qrc:/frontend/index.html")};
}
} // namespace

QtWebEngineAdapter::QtWebEngineAdapter(QWidget *parent) : QWebEngineView(parent)
{
    static const bool resourcesInitialized = []() {
        Q_INIT_RESOURCE(frontend);
        return true;
    }();
    Q_UNUSED(resourcesInitialized);

    const bool developmentEnabled = developmentServerEnabled();
    auto *profile = new QWebEngineProfile;
    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    profile->setUrlRequestInterceptor(new FrontendRequestInterceptor(developmentEnabled, profile));

    auto *page = new FrontendPage(profile, developmentEnabled, this);
    profile->setParent(page);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    page->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    setPage(page);

    auto *channel = new QWebChannel(page);
    native_bridge_ = new NativeBridge(this, channel);
    channel->registerObject(QStringLiteral("nativeBridge"), native_bridge_);
    page->setWebChannel(channel);

    installDropEventFilters(this);
    setUrl(initialFrontendUrl(developmentEnabled));
}

void QtWebEngineAdapter::installDropEventFilters(QObject *object)
{
    if (object == nullptr) {
        return;
    }
    auto *widget = qobject_cast<QWidget *>(object);
    if (widget != nullptr && widget != this && widget->isWindow()) {
        return;
    }
    object->installEventFilter(this);
    if (widget != nullptr) {
        widget->setAcceptDrops(true);
    }
    for (QObject *child : object->children()) {
        installDropEventFilters(child);
    }
}

bool QtWebEngineAdapter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ChildAdded) {
        const auto *childEvent = dynamic_cast<QChildEvent *>(event);
        if (childEvent == nullptr) {
            return QWebEngineView::eventFilter(watched, event);
        }
        QPointer<QObject> child = childEvent->child();
        QTimer::singleShot(0, this, [this, child = std::move(child)]() {
            if (child) {
                installDropEventFilters(child);
            }
        });
        return QWebEngineView::eventFilter(watched, event);
    }

    if (event->type() == QEvent::DragEnter) {
        auto *dragEvent = dynamic_cast<QDragEnterEvent *>(event);
        if (dragEvent != nullptr && (!droppedSQLiteFilePath(dragEvent->mimeData()).isEmpty() ||
                                     !droppedImageFilePath(dragEvent->mimeData()).isEmpty())) {
            dragEvent->acceptProposedAction();
            return true;
        }
    } else if (event->type() == QEvent::Drop) {
        auto *dropEvent = dynamic_cast<QDropEvent *>(event);
        if (dropEvent != nullptr) {
            const QMimeData *const mimeData = dropEvent->mimeData();
            const QString databasePath = droppedSQLiteFilePath(mimeData);
            if (!databasePath.isEmpty()) {
                native_bridge_->openDroppedSQLiteFile(databasePath);
                dropEvent->acceptProposedAction();
                return true;
            }
            const QString imagePath = droppedImageFilePath(mimeData);
            if (!imagePath.isEmpty()) {
                native_bridge_->openDroppedImageFile(imagePath);
                dropEvent->acceptProposedAction();
                return true;
            }
        }
    }
    return QWebEngineView::eventFilter(watched, event);
}

QString QtWebEngineAdapter::droppedSQLiteFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }
    return LocalFileSelectionService::firstExistingLocalFile(
        mimeData->urls(),
        {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")});
}

QString QtWebEngineAdapter::droppedImageFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }
    for (const QUrl &url : mimeData->urls()) {
        if (url.isLocalFile() && ImageProcessingService::isReadableImageFile(url.toLocalFile())) {
            return url.toLocalFile();
        }
    }
    return {};
}
