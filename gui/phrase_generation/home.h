#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
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
    void on_saveButton_clicked();
    void on_copyButton_clicked();
    void on_deleteButton_clicked();
    void on_toggleTreeButton_clicked();
    void on_titleTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void copyContent();

private:
    Ui::home *ui;
    void loadTitles();
    QString loadContent(const QString &title);
    void saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &title);
};

#endif // HOME_H
