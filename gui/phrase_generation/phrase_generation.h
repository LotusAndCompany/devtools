#ifndef PHRASE_GENERATION_H
#define PHRASE_GENERATION_H

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QWidget>

namespace Ui {
class phraseGeneration;
}

class phraseGeneration : public QWidget
{
    Q_OBJECT

public:
    explicit phraseGeneration(QWidget *parent = nullptr);
    ~phraseGeneration() override;

private slots:

    void handleAddButtonClick();
    void handleSaveButtonClick();
    void handleCopyButtonClick();
    void handleDeleteButtonClick();
    void handleToggleTreeButtonClick();
    void handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column);
    void copyContent();

private:
    Ui::phraseGeneration *ui;
    void loadTitles();
    static QString loadContent(const QString &filename, QString *title = nullptr);
    static void saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &title);
    QString currentFile; // 現在のファイル名を保持

protected:
    void changeEvent(QEvent *event) override;
};

#endif // PHRASE_GENERATION_H
