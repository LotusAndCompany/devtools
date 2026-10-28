#ifndef QT_WEBENGINE_ADAPTER_H
#define QT_WEBENGINE_ADAPTER_H

#include <QWebEngineView>

class NativeBridge;

/**
 * @brief Hosts the bundled frontend or the exact loopback development origin.
 */
class QtWebEngineAdapter : public QWebEngineView
{
public:
    explicit QtWebEngineAdapter(QWidget *parent = nullptr);
    [[nodiscard]] NativeBridge *nativeBridge() const { return native_bridge_; }

private:
    NativeBridge *native_bridge_{nullptr};
};

#endif // QT_WEBENGINE_ADAPTER_H
