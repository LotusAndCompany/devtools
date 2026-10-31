#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QtGlobal>

class QAction;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QEvent;
class QMenu;
class QMimeData;
class QtWebEngineAdapter;

/**
 * @brief Main application window that hosts the React frontend.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    QtWebEngineAdapter *web_frontend_{nullptr};
    QMenu *menu_file_{nullptr};
    QMenu *menu_devtools_{nullptr};
    QAction *action_settings_{nullptr};
    QAction *action_about_{nullptr};
    QAction *action_close_{nullptr};
    QAction *action_show_window_{nullptr};

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private:
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
#ifdef Q_OS_MACOS
    void closeEvent(QCloseEvent *event) override;
#endif
    void setupCentralWidget();
    void setupMenuBar();
    void retranslateUi();
    static QString droppedSQLiteFilePath(const QMimeData *mimeData);
    static QString droppedImageFilePath(const QMimeData *mimeData);
    static void onLanguageChanged(const QString &languageCode);

private slots:
    void onActionTriggered(QAction *action);
};

#endif // MAIN_WINDOW_H
