#ifndef PHRASE_GENERATION_H
#define PHRASE_GENERATION_H

#include <QWidget>

class QFrame;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class phraseGeneration : public QWidget
{
    Q_OBJECT

public:
    explicit phraseGeneration(QWidget *parent = nullptr);
    ~phraseGeneration() override = default;
    phraseGeneration(const phraseGeneration &) = delete;
    phraseGeneration &operator=(const phraseGeneration &) = delete;
    phraseGeneration(phraseGeneration &&) = delete;
    phraseGeneration &operator=(phraseGeneration &&) = delete;

private slots:

    void handleAddButtonClick();
    void handleSaveButtonClick();
    void handleCopyButtonClick();
    void handleDeleteButtonClick();
    void handleToggleTreeButtonClick();
    void handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column);
    void copyContent();

private:
    void buildUi();
    void createTopBarWidgets();
    void createBodyWidgets();
    void layoutWidgets();
    void retranslateUi();
    void adjustTreeColumnWidth();
    void loadTitles();
    static QString loadContent(const QString &filename, QString *title = nullptr);
    static void saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &filename);

    QLineEdit *template_title{nullptr};
    QPushButton *delete_button{nullptr};
    QPushButton *copy_button{nullptr};
    QPushButton *add_button{nullptr};
    QPushButton *toggle_tree_button{nullptr};
    QFrame *line{nullptr};
    QPlainTextEdit *template_text{nullptr};
    QTreeWidget *title_tree_widget{nullptr};
    QPushButton *save_button{nullptr};

    QString currentFile; // 現在のファイル名を保持

protected:
    void changeEvent(QEvent *event) override;
};

#endif // PHRASE_GENERATION_H
