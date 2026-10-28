#include "qt_webengine_adapter.h"

#include "features/web_frontend/gui/native_bridge.h"

#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>

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
            path.startsWith(QStringLiteral("/qtwebchannel/")));
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
            return false;
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

    setUrl(initialFrontendUrl(developmentEnabled));
}
