#ifndef PHRASE_GENERATION_H
#define PHRASE_GENERATION_H

#include <QWidget>

class QFrame;
class QGroupBox;
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
    void handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column);

private:
    void buildUi();
    void createWidgets();
    void layoutWidgets();
    void setupShortcuts();
    void retranslateUi();
    void loadTitles();
    void selectTreeItemByFilename(const QString &filename);
    [[nodiscard]] bool hasUnsavedChanges() const;
    bool confirmDiscard();
    static QString loadContent(const QString &filename, QString *title = nullptr);
    static QString saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &filename);

    QLineEdit *template_title{nullptr};
    QPushButton *delete_button{nullptr};
    QPushButton *copy_button{nullptr};
    QPushButton *add_button{nullptr};
    QGroupBox *editor_group{nullptr};
    QGroupBox *tree_group{nullptr};
    QFrame *tree_separator{nullptr};
    QFrame *editor_separator{nullptr};
    QPlainTextEdit *template_text{nullptr};
    QTreeWidget *title_tree_widget{nullptr};
    QPushButton *save_button{nullptr};

    QString currentFile;

protected:
    void changeEvent(QEvent *event) override;
};

#endif // PHRASE_GENERATION_H
