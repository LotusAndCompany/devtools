#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QListWidgetItem>
#include <QClipboard>
#include "ItemWidget.h"

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

    void on_showTitleListButton_clicked();

    void on_closeTitleListButton_clicked();

    void on_saveButton_clicked();

    void on_titleList_itemClicked(QListWidgetItem *item);

    void on_deleteButton_clicked();

    void on_copyButton_clicked();

    void on_listCopyButton_clicked(const QString &title);

private:
    Ui::home *ui;
    void loadTitles();
    void saveContent(const QString &title, const QString &content);
    QString loadContent(const QString &title);
    void deleteContent(const QString &title);
};

#endif // HOME_H
