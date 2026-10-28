#ifndef QT_WEBENGINE_ADAPTER_H
#define QT_WEBENGINE_ADAPTER_H

#include <QWebEngineView>

/**
 * @brief Hosts the bundled frontend or the exact loopback development origin.
 */
class QtWebEngineAdapter : public QWebEngineView
{
public:
    explicit QtWebEngineAdapter(QWidget *parent = nullptr);
};

#endif // QT_WEBENGINE_ADAPTER_H
