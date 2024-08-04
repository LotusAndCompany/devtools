#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
// #include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QClipboard>

namespace Ui {
class home;
}

class home : public QWidget
{
    Q_OBJECT

public:
    explicit home(QWidget *parent = nullptr);
    ~home();

private slots:

    void on_addButton_clicked();

    // void on_showTitleListButton_clicked();

    // void on_showTitleTreeWidgetButton_clicked();

    // void on_closeTitleListButton_clicked();

    // void on_closeTitleTreeWidgetButton_clicked();

    void on_saveButton_clicked();

    // void on_titleList_itemClicked(QListWidgetItem *item);

    void on_titleTreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_deleteButton_clicked();

    void on_copyButton_clicked();

    void copyContent();

    void on_toggleTreeButton_clicked();

private:
    Ui::home *ui;
    void loadTitles();
    void saveContent(const QString &title, const QString &content);
    QString loadContent(const QString &title);
    void deleteContent(const QString &title);
};

#endif // HOME_H
