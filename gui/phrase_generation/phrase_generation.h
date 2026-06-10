#ifndef PHRASE_GENERATION_H
#define PHRASE_GENERATION_H

#include <QWidget>

class QFrame;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

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
    void copyContent();

private:
    void buildUi();
    void createTopBarWidgets();
    void createBodyWidgets();
    void layoutWidgets();
    void retranslateUi();
    void applyStyles();
    void loadTitles();
    void onListItemClicked(QFrame *item);
    static QString loadContent(const QString &filename, QString *title = nullptr);
    static void saveContent(const QString &title, const QString &content);
    void deleteContent(const QString &filename);

    QLineEdit *template_title{nullptr};
    QPushButton *delete_button{nullptr};
    QPushButton *copy_button{nullptr};
    QPushButton *add_button{nullptr};
    QPushButton *toggle_tree_button{nullptr};
    QFrame *toolbar_frame{nullptr};
    QFrame *editor_frame{nullptr};
    QPlainTextEdit *template_text{nullptr};
    QPushButton *save_button{nullptr};
    QWidget *sidebar_container{nullptr};
    QFrame *list_header{nullptr};
    QLabel *list_header_label{nullptr};
    QScrollArea *list_scroll{nullptr};
    QWidget *list_content{nullptr};
    QVBoxLayout *list_layout{nullptr};
    QFrame *current_list_item{nullptr};

    QString currentFile;

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // PHRASE_GENERATION_H
