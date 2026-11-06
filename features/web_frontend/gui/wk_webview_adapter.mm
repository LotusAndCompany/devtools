#include "wk_webview_adapter.h"

#include "features/framework/core/local_file_selection_service.h"
#include "features/image/core/image_processing_service.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QProcessEnvironment>
#include <QUrl>
#include <QWindow>

#include <AppKit/AppKit.h>
#include <WebKit/WebKit.h>

class WKWebViewAdapter;

@interface DevToolsWKBridgeDelegate : NSObject <WKNavigationDelegate, WKUIDelegate,
                                               WKScriptMessageHandler>
@property(nonatomic, assign) WKWebViewAdapter *adapter;
@end

@interface DevToolsWKWebView : WKWebView
@property(nonatomic, assign) WKWebViewAdapter *adapter;
@end

struct WKWebViewStorage
{
    __strong DevToolsWKWebView *web_view = nil;
    __strong DevToolsWKBridgeDelegate *delegate = nil;
};

namespace {
constexpr const char *MESSAGE_HANDLER_NAME = "devtoolsNativeBridge";
constexpr const char *DEV_SERVER_URL = "http://127.0.0.1:5173";
constexpr qsizetype WK_START_OPERATION_ENVELOPE_BYTES =
    sizeof("{\"type\":\"startOperation\",\"request\":}") - 1;

struct ParsedScriptMessage
{
    QVariantMap value;
    QString oversizedRequestId;
};

NSString *toNSString(const QString &value)
{
    const QByteArray utf8 = value.toUtf8();
    return [NSString stringWithUTF8String:utf8.constData()];
}

QString toQString(NSString *value)
{
    return value == nil ? QString() : QString::fromUtf8(value.UTF8String);
}

NSArray<NSURL *> *fileUrlsFromPasteboard(NSPasteboard *pasteboard)
{
    if (pasteboard == nil) {
        return @[];
    }
    NSDictionary *const options = @{NSPasteboardURLReadingFileURLsOnlyKey : @YES};
    NSArray *const objects = [pasteboard readObjectsForClasses:@[[NSURL class]] options:options];
    return objects == nil ? @[] : objects;
}

QStringList localFilePaths(NSArray<NSURL *> *urls)
{
    QStringList paths;
    for (NSURL *url in urls) {
        if (url.isFileURL) {
            paths.append(toQString(url.path));
        }
    }
    return paths;
}

ParsedScriptMessage variantMapFromMessage(id body)
{
    if (![body isKindOfClass:[NSDictionary class]]) {
        return {};
    }
    NSError *error = nil;
    NSData *const data = [NSJSONSerialization dataWithJSONObject:body
                                                         options:NSJSONWritingFragmentsAllowed
                                                           error:&error];
    if (data == nil) {
        return {};
    }
    NSDictionary *const dictionary = (NSDictionary *)body;
    NSString *const type = [dictionary objectForKey:@"type"];
    const bool isStartOperation = [type isKindOfClass:[NSString class]] &&
                                  [type isEqualToString:@"startOperation"];
    // Allow room for WK's outer transport envelope and JSON escaping while
    // serializing the JS object. NativeBridge validates the inner request at 2 MiB.
    const qsizetype maxMessageBytes =
        NativeBridge::MAX_MESSAGE_BYTES * (isStartOperation ? 2 : 1) +
        (isStartOperation ? WK_START_OPERATION_ENVELOPE_BYTES : 0);
    if (data.length > static_cast<NSUInteger>(maxMessageBytes)) {
        NSString *requestId = nil;
        NSDictionary *const request = [dictionary objectForKey:@"request"];
        if (isStartOperation && [request isKindOfClass:[NSDictionary class]]) {
            id const candidate = [request objectForKey:@"requestId"];
            if ([candidate isKindOfClass:[NSString class]]) {
                requestId = candidate;
            }
        }
        return {{}, toQString(requestId)};
    }
    const QByteArray json(static_cast<const char *>(data.bytes), static_cast<qsizetype>(data.length));
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    return {parseError.error == QJsonParseError::NoError && document.isObject()
                ? document.object().toVariantMap()
                : QVariantMap{},
            {}};
}
} // namespace

@implementation DevToolsWKBridgeDelegate

- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)message
{
    Q_UNUSED(userContentController);
    if (self.adapter == nullptr) {
        return;
    }
    const QString sourceUrl = toQString(message.frameInfo.request.URL.absoluteString);
    if (!message.frameInfo.isMainFrame || !self.adapter->isAllowedNavigation(sourceUrl, true)) {
        return;
    }
    const ParsedScriptMessage parsed = variantMapFromMessage(message.body);
    if (!parsed.oversizedRequestId.isEmpty()) {
        self.adapter->handleOversizedScriptMessage(sourceUrl, message.frameInfo.isMainFrame,
                                                   parsed.oversizedRequestId);
    } else {
        self.adapter->handleScriptMessage(sourceUrl, message.frameInfo.isMainFrame, parsed.value);
    }
}

- (void)webView:(WKWebView *)webView
    decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
                    decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
{
    Q_UNUSED(webView);
    const QString url = toQString(navigationAction.request.URL.absoluteString);
    const bool allowed = self.adapter != nullptr &&
                         self.adapter->isAllowedNavigation(
                             url, navigationAction.targetFrame.isMainFrame);
    if (!allowed && self.adapter != nullptr) {
        self.adapter->notifyNavigationRejected();
    }
    decisionHandler(allowed ? WKNavigationActionPolicyAllow : WKNavigationActionPolicyCancel);
}

- (void)webView:(WKWebView *)webView
    decidePolicyForNavigationResponse:(WKNavigationResponse *)navigationResponse
                    decisionHandler:(void (^)(WKNavigationResponsePolicy))decisionHandler
{
    Q_UNUSED(webView);
    const QString url = toQString(navigationResponse.response.URL.absoluteString);
    const bool allowed = self.adapter != nullptr &&
                         self.adapter->isAllowedNavigation(url,
                                                           navigationResponse.forMainFrame);
    if (!allowed && self.adapter != nullptr) {
        self.adapter->notifyNavigationRejected();
    }
    decisionHandler(allowed ? WKNavigationResponsePolicyAllow : WKNavigationResponsePolicyCancel);
}

- (WKWebView *)webView:(WKWebView *)webView
    createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration
               forNavigationAction:(WKNavigationAction *)navigationAction
                    windowFeatures:(WKWindowFeatures *)windowFeatures
{
    Q_UNUSED(webView);
    Q_UNUSED(configuration);
    Q_UNUSED(navigationAction);
    Q_UNUSED(windowFeatures);
    if (self.adapter != nullptr) {
        self.adapter->notifyNavigationRejected();
    }
    return nil;
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    Q_UNUSED(webView);
    Q_UNUSED(navigation);
    if (self.adapter != nullptr) {
        self.adapter->notifyFrontendLoadFinished(true);
    }
}

- (void)webView:(WKWebView *)webView
    didFailNavigation:(WKNavigation *)navigation
          withError:(NSError *)error
{
    Q_UNUSED(webView);
    Q_UNUSED(navigation);
    Q_UNUSED(error);
    if (self.adapter != nullptr) {
        self.adapter->notifyFrontendLoadFinished(false);
    }
}

- (void)webView:(WKWebView *)webView
    didFailProvisionalNavigation:(WKNavigation *)navigation
                       withError:(NSError *)error
{
    Q_UNUSED(webView);
    Q_UNUSED(navigation);
    Q_UNUSED(error);
    if (self.adapter != nullptr) {
        self.adapter->notifyFrontendLoadFinished(false);
    }
}

@end

@implementation DevToolsWKWebView

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration
{
    self = [super initWithFrame:frame configuration:configuration];
    if (self != nil) {
        [self registerForDraggedTypes:@[NSPasteboardTypeFileURL]];
        self.allowsMagnification = NO;
        self.allowsBackForwardNavigationGestures = NO;
    }
    return self;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    if (self.adapter == nullptr) {
        return NSDragOperationNone;
    }
    const QStringList paths = localFilePaths(fileUrlsFromPasteboard(sender.draggingPasteboard));
    return self.adapter->canAcceptDroppedFilePaths(paths) ? NSDragOperationCopy
                                                          : NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return [self draggingEntered:sender];
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender
{
    return [self draggingEntered:sender] != NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    if (self.adapter == nullptr) {
        return NO;
    }
    const QStringList paths = localFilePaths(fileUrlsFromPasteboard(sender.draggingPasteboard));
    if (!self.adapter->canAcceptDroppedFilePaths(paths)) {
        return NO;
    }
    self.adapter->routeDroppedFilePaths(paths);
    return YES;
}

@end

WKWebViewAdapter::WKWebViewAdapter(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("WKWebViewAdapter"));
    native_bridge_ = new NativeBridge(this, this);

    connect(native_bridge_, &NativeBridge::operationStarted, this,
            [this](const QString &requestId, const QString &operation) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("started")},
                                     {QStringLiteral("requestId"), requestId},
                                     {QStringLiteral("operation"), operation}});
            });
    connect(native_bridge_, &NativeBridge::operationProgress, this,
            [this](const QString &requestId, int percent, const QString &message) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("progress")},
                                     {QStringLiteral("requestId"), requestId},
                                     {QStringLiteral("percent"), percent},
                                     {QStringLiteral("message"), message}});
            });
    connect(native_bridge_, &NativeBridge::operationCompleted, this,
            [this](const QString &requestId, const QVariantMap &result) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("completed")},
                                     {QStringLiteral("requestId"), requestId},
                                     {QStringLiteral("result"), result}});
            });
    connect(native_bridge_, &NativeBridge::operationFailed, this,
            [this](const QString &requestId, const QVariantMap &error) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("failed")},
                                     {QStringLiteral("requestId"), requestId},
                                     {QStringLiteral("error"), error}});
            });
    connect(native_bridge_, &NativeBridge::operationCancelled, this,
            [this](const QString &requestId) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("cancelled")},
                                     {QStringLiteral("requestId"), requestId}});
            });
    connect(native_bridge_, &NativeBridge::imageDropAvailable, this, [this]() {
        dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("imageDropAvailable")}});
    });
    connect(native_bridge_, &NativeBridge::navigationRequested, this,
            [this](const QString &route) {
                dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("navigate")},
                                     {QStringLiteral("route"), route}});
            });

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    if (!extractFrontendBundle()) {
        layout->addWidget(new QLabel(tr("Unable to prepare the React frontend."), this));
        return;
    }

    auto *const storage = new WKWebViewStorage;
    native_storage_ = storage;
    storage->delegate = [DevToolsWKBridgeDelegate new];
    storage->delegate.adapter = this;

    WKWebViewConfiguration *const configuration = [WKWebViewConfiguration new];
    WKUserContentController *const userContentController = [WKUserContentController new];
    [userContentController addScriptMessageHandler:storage->delegate
                                      contentWorld:[WKContentWorld pageWorld]
                                              name:@(MESSAGE_HANDLER_NAME)];
    configuration.userContentController = userContentController;
    configuration.websiteDataStore = [WKWebsiteDataStore nonPersistentDataStore];
    configuration.preferences.javaScriptCanOpenWindowsAutomatically = NO;

    storage->web_view = [[DevToolsWKWebView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)
                                                   configuration:configuration];
    storage->web_view.adapter = this;
    storage->web_view.navigationDelegate = storage->delegate;
    storage->web_view.UIDelegate = storage->delegate;

    QWindow *const foreignWindow = QWindow::fromWinId(reinterpret_cast<WId>(
        (__bridge void *)storage->web_view));
    if (foreignWindow == nullptr) {
        storage->web_view = nil;
        storage->delegate.adapter = nullptr;
        delete storage;
        native_storage_ = nullptr;
        layout->addWidget(new QLabel(tr("Unable to start the native web view."), this));
        return;
    }
    window_container_ = QWidget::createWindowContainer(foreignWindow, this);
    window_container_->setFocusPolicy(Qt::StrongFocus);
    layout->addWidget(window_container_);

    const QUrl initialUrl = development_server_url_.isEmpty()
                                ? QUrl::fromLocalFile(index_file_path_)
                                : QUrl(development_server_url_);
    if (initialUrl.isLocalFile()) {
        NSURL *const fileUrl = [NSURL fileURLWithPath:toNSString(index_file_path_)];
        NSURL *const readAccessUrl = [NSURL fileURLWithPath:toNSString(bundle_directory_->path())
                                               isDirectory:YES];
        [storage->web_view loadFileURL:fileUrl allowingReadAccessToURL:readAccessUrl];
    } else {
        [storage->web_view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:toNSString(
                                                                        development_server_url_)]]];
    }
}

WKWebViewAdapter::~WKWebViewAdapter()
{
    auto *const storage = static_cast<WKWebViewStorage *>(native_storage_);
    if (storage == nullptr) {
        return;
    }
    storage->delegate.adapter = nullptr;
    storage->web_view.adapter = nullptr;
    storage->web_view.navigationDelegate = nil;
    storage->web_view.UIDelegate = nil;
    [storage->web_view.configuration.userContentController
        removeScriptMessageHandlerForName:@(MESSAGE_HANDLER_NAME)
                            contentWorld:[WKContentWorld pageWorld]];
    delete window_container_;
    window_container_ = nullptr;
    [storage->web_view stopLoading];
    delete storage;
}

bool WKWebViewAdapter::extractFrontendBundle()
{
    Q_INIT_RESOURCE(frontend);
    bundle_directory_ = std::make_unique<QTemporaryDir>(
        QDir::tempPath() + QStringLiteral("/DevTools-frontend-XXXXXX"));
    if (!bundle_directory_->isValid()) {
        return false;
    }

    const QString assetDirectory = QDir(bundle_directory_->path()).filePath(QStringLiteral("assets"));
    if (!QDir().mkpath(assetDirectory)) {
        return false;
    }
    const auto copyResource = [](const QString &resourcePath, const QString &outputPath) {
        QFile::remove(outputPath);
        if (!QFile::copy(resourcePath, outputPath)) {
            qWarning() << "Could not extract frontend resource" << resourcePath;
            return false;
        }
        QFile::setPermissions(outputPath, QFileDevice::ReadOwner);
        return true;
    };
    if (!copyResource(QStringLiteral(":/frontend/index.html"),
                      QDir(bundle_directory_->path()).filePath(QStringLiteral("index.html"))) ||
        !copyResource(QStringLiteral(":/frontend/assets/app.js"),
                      QDir(assetDirectory).filePath(QStringLiteral("app.js"))) ||
        !copyResource(QStringLiteral(":/frontend/assets/style.css"),
                      QDir(assetDirectory).filePath(QStringLiteral("style.css")))) {
        return false;
    }
    index_file_path_ = QDir(bundle_directory_->path()).filePath(QStringLiteral("index.html"));

#ifndef NDEBUG
    const QString configured = QProcessEnvironment::systemEnvironment().value(
        QStringLiteral("DEVTOOLS_WEB_UI_DEV_SERVER_URL"));
    if (configured == QString::fromLatin1(DEV_SERVER_URL)) {
        development_server_url_ = configured;
    }
#endif
    return true;
}

bool WKWebViewAdapter::isAllowedNavigation(const QString &url, bool mainFrame) const
{
    const QUrl parsed(url);
    if (!mainFrame) {
        return parsed.scheme() == QStringLiteral("about") &&
               parsed.path() == QStringLiteral("srcdoc") && parsed.query().isEmpty();
    }
    if (!development_server_url_.isEmpty()) {
        return parsed.scheme() == QStringLiteral("http") &&
               parsed.host() == QStringLiteral("127.0.0.1") && parsed.port() == 5173 &&
               (parsed.path() == QStringLiteral("/") ||
                parsed.path() == QStringLiteral("/index.html")) &&
               parsed.userInfo().isEmpty() && parsed.query().isEmpty();
    }
    return parsed.isLocalFile() && parsed.toLocalFile() == index_file_path_ &&
           parsed.userInfo().isEmpty() && parsed.query().isEmpty();
}

void WKWebViewAdapter::handleScriptMessage(const QString &sourceUrl, bool mainFrame,
                                           const QVariantMap &message)
{
    if (!mainFrame || !isAllowedNavigation(sourceUrl, true) || native_bridge_ == nullptr) {
        return;
    }
    const QString type = message.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("handshake")) {
        dispatchNativeEvent({{QStringLiteral("type"), QStringLiteral("ready")},
                             {QStringLiteral("contractVersion"),
                              native_bridge_->contractVersion()}});
    } else if (type == QStringLiteral("startOperation")) {
        native_bridge_->startOperation(message.value(QStringLiteral("request")).toMap());
    } else if (type == QStringLiteral("cancelOperation")) {
        native_bridge_->cancelOperation(message.value(QStringLiteral("requestId")).toString());
    }
}

void WKWebViewAdapter::handleOversizedScriptMessage(const QString &sourceUrl, bool mainFrame,
                                                     const QString &requestId)
{
    if (!mainFrame || !isAllowedNavigation(sourceUrl, true) || native_bridge_ == nullptr) {
        return;
    }
    native_bridge_->rejectOversizedRequest(requestId);
}

bool WKWebViewAdapter::canAcceptDroppedFilePaths(const QStringList &paths) const
{
    return !droppedSQLiteFilePath(paths).isEmpty() || !droppedImageFilePath(paths).isEmpty();
}

void WKWebViewAdapter::routeDroppedFilePaths(const QStringList &paths)
{
    const QString databasePath = droppedSQLiteFilePath(paths);
    if (!databasePath.isEmpty()) {
        native_bridge_->openDroppedSQLiteFile(databasePath);
        return;
    }
    const QString imagePath = droppedImageFilePath(paths);
    if (!imagePath.isEmpty()) {
        native_bridge_->openDroppedImageFile(imagePath);
    }
}

void WKWebViewAdapter::dispatchNativeEvent(const QVariantMap &event)
{
    auto *const storage = static_cast<WKWebViewStorage *>(native_storage_);
    if (storage == nullptr || storage->web_view == nil) {
        return;
    }
    const QByteArray json = QJsonDocument(QJsonObject::fromVariantMap(event))
                                .toJson(QJsonDocument::Compact);
    NSData *const data = [NSData dataWithBytes:json.constData() length:static_cast<NSUInteger>(
                                                                   json.size())];
    NSError *error = nil;
    id const nativeEvent = [NSJSONSerialization JSONObjectWithData:data
                                                           options:NSJSONReadingFragmentsAllowed
                                                             error:&error];
    if (error != nil || ![nativeEvent isKindOfClass:[NSDictionary class]]) {
        qWarning() << "Could not serialize a native bridge event for WKWebView";
        return;
    }
    [storage->web_view
        callAsyncJavaScript:@"window.dispatchEvent(new CustomEvent('devtools-native-event', { detail: event }));"
                  arguments:@{@"event" : nativeEvent}
                     inFrame:nil
              inContentWorld:[WKContentWorld pageWorld]
           completionHandler:^(id result, NSError *javascriptError) {
               Q_UNUSED(result);
               if (javascriptError != nil) {
                   qWarning() << "WKWebView did not accept a native bridge event";
               }
           }];
}

void WKWebViewAdapter::notifyFrontendLoadFinished(bool success)
{
    emit frontendLoadFinished(success);
}

void WKWebViewAdapter::notifyNavigationRejected()
{
    emit navigationRejected();
}

QString WKWebViewAdapter::droppedSQLiteFilePath(const QStringList &paths)
{
    QList<QUrl> urls;
    urls.reserve(paths.size());
    for (const QString &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    return LocalFileSelectionService::firstExistingLocalFile(
        urls, {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")});
}

QString WKWebViewAdapter::droppedImageFilePath(const QStringList &paths)
{
    for (const QString &path : paths) {
        if (ImageProcessingService::isReadableImageFile(path)) {
            return path;
        }
    }
    return {};
}

void WKWebViewAdapter::evaluateJavaScriptForTesting(
    const QString &script, std::function<void(const QVariant &)> callback)
{
    auto *const storage = static_cast<WKWebViewStorage *>(native_storage_);
    if (storage == nullptr || storage->web_view == nil) {
        callback({});
        return;
    }
    [storage->web_view evaluateJavaScript:toNSString(script)
                                  inFrame:nil
                          inContentWorld:[WKContentWorld pageWorld]
                      completionHandler:^(id result, NSError *error) {
                          Q_UNUSED(error);
                          if ([result isKindOfClass:[NSString class]]) {
                              callback(toQString(result));
                          } else if ([result isKindOfClass:[NSNumber class]]) {
                              callback([result boolValue]);
                          } else {
                              callback({});
                          }
                      }];
}
