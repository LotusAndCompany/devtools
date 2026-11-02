#ifndef WK_WEBVIEW_ADAPTER_H
#define WK_WEBVIEW_ADAPTER_H

#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>
#include <QWidget>

#include <functional>
#include <memory>

class NativeBridge;
class QWindow;

/**
 * @brief Hosts the bundled React frontend in a public-API WKWebView inside Qt Widgets.
 */
class WKWebViewAdapter : public QWidget
{
    Q_OBJECT

public:
    explicit WKWebViewAdapter(QWidget *parent = nullptr);
    ~WKWebViewAdapter() override;

    [[nodiscard]] NativeBridge *nativeBridge() const { return native_bridge_; }

    // Native delegate entry points implemented by the WKWebView adapter.
    [[nodiscard]] bool isAllowedNavigation(const QString &url, bool mainFrame) const;
    void handleScriptMessage(const QString &sourceUrl, bool mainFrame, const QVariantMap &message);
    [[nodiscard]] bool canAcceptDroppedFilePaths(const QStringList &paths) const;
    void routeDroppedFilePaths(const QStringList &paths);
    void dispatchNativeEvent(const QVariantMap &event);
    void notifyFrontendLoadFinished(bool success);
    void notifyNavigationRejected();

Q_SIGNALS:
    void frontendLoadFinished(bool success);
    void navigationRejected();

private:
    friend class TestWKWebViewFrontend;

    [[nodiscard]] bool extractFrontendBundle();
    [[nodiscard]] static QString droppedSQLiteFilePath(const QStringList &paths);
    [[nodiscard]] static QString droppedImageFilePath(const QStringList &paths);
    void evaluateJavaScriptForTesting(const QString &script,
                                      std::function<void(const QVariant &)> callback);

    std::unique_ptr<QTemporaryDir> bundle_directory_;
    QWidget *window_container_{nullptr};
    NativeBridge *native_bridge_{nullptr};
    QString index_file_path_;
    QString development_server_url_;
    void *native_storage_{nullptr};
};

#endif // WK_WEBVIEW_ADAPTER_H
