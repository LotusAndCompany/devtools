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

    void handleAddButtonClick();
    void handleSaveButtonClick();
    void handleCopyButtonClick();
    void handleDeleteButtonClick();
    void handleToggleTreeButtonClick();
    void handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column);
    void copyContent();

private:
    Ui::home *ui;
    void loadTitles();
    // QString loadContent(const QString &title);
    QString loadContent(const QString &filename, QString *title = nullptr);
    void saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &title);
    QString currentFile; //現在のファイル名を保持

};

#endif // HOME_H
