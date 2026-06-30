#include "phrase_generation.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSizePolicy>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QVBoxLayout>

#include <algorithm>

namespace {
const char *const TEMPLATE_TITLE_STYLE = "QLineEdit {\n    padding: 2px 0px 2px 2px;\n}";
const char *const TEMPLATE_TEXT_STYLE = "QPlainTextEdit {\n    padding: 0px 0px 5px 0px;\n}";

constexpr int DEFAULT_WIDTH = 1012;
constexpr int DEFAULT_HEIGHT = 633;
constexpr int MIN_WIDTH = 300;
constexpr int MIN_HEIGHT = 200;
constexpr int TREE_FIXED_COLUMN_WIDTH = 40;
constexpr int TREE_COLUMN_PADDING = 8;

constexpr int TITLE_FONT_POINTSIZE = 14;
constexpr int TEXT_FONT_POINTSIZE = 14;
constexpr int ADD_FONT_POINTSIZE = 14;

constexpr int TREE_PANEL_STRETCH = 1;
constexpr int EDITOR_PANEL_STRETCH = 3;
} // namespace

phraseGeneration::phraseGeneration(QWidget *parent) : QWidget(parent)
{
    buildUi();

    connect(add_button, &QPushButton::clicked, this, &phraseGeneration::handleAddButtonClick);
    connect(save_button, &QPushButton::clicked, this, &phraseGeneration::handleSaveButtonClick);
    connect(copy_button, &QPushButton::clicked, this, &phraseGeneration::handleCopyButtonClick);
    connect(delete_button, &QPushButton::clicked, this, &phraseGeneration::handleDeleteButtonClick);
    connect(title_tree_widget, &QTreeWidget::itemClicked, this,
            &phraseGeneration::handleTitleTreeWidgetItemClick);

    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    title_tree_widget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    title_tree_widget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    adjustTreeColumnWidth();

    setupShortcuts();
    loadTitles();
}

namespace {
QSizePolicy makePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
{
    QSizePolicy policy(horizontal, vertical);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    return policy;
}

QFrame *makeSeparator(QWidget *parent)
{
    auto *sep = new QFrame(parent);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    sep->setSizePolicy(makePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    return sep;
}
} // namespace

void phraseGeneration::buildUi()
{
    setObjectName(QStringLiteral("phraseGeneration"));
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    createWidgets();
    layoutWidgets();
    retranslateUi();
}

void phraseGeneration::retranslateUi()
{
    setWindowTitle(tr("Form"));
    editor_group->setTitle(tr("Editor"));
    tree_group->setTitle(tr("Templates"));
    template_title->setPlaceholderText(tr("Title"));
    template_text->setPlaceholderText(tr("Text"));
    delete_button->setText(tr("Delete"));
    copy_button->setText(tr("Copy"));
    save_button->setText(tr("Save"));

    QString const add_label = tr("Add");
    add_button->setText(add_label);
    add_button->setToolTip(tr("Add new template"));
    add_button->setAccessibleName(tr("Add new template"));
    add_button->setAccessibleDescription(tr("Add new template"));

    for (int i = 0; i < title_tree_widget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = title_tree_widget->topLevelItem(i);
        auto *btn = qobject_cast<QPushButton *>(title_tree_widget->itemWidget(item, 1));
        if (btn != nullptr) {
            btn->setText(tr("Copy"));
        }
    }
}

void phraseGeneration::adjustTreeColumnWidth()
{
    QPushButton const sample(tr("Copy"));
    int const min_width =
        std::max(TREE_FIXED_COLUMN_WIDTH, sample.sizeHint().width() + TREE_COLUMN_PADDING);
    title_tree_widget->setColumnWidth(1, min_width);
}

void phraseGeneration::createWidgets()
{
    add_button = new QPushButton(this);
    add_button->setObjectName(QStringLiteral("addButton"));
    add_button->setSizePolicy(makePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    {
        QFont font;
        font.setPointSize(ADD_FONT_POINTSIZE);
        add_button->setFont(font);
    }
    {
        QIcon const icon = QIcon::fromTheme(QStringLiteral("add"));
        if (!icon.isNull()) {
            add_button->setIcon(icon);
        }
    }

    tree_separator = makeSeparator(this);

    title_tree_widget = new QTreeWidget(this);
    title_tree_widget->setObjectName(QStringLiteral("titleTreeWidget"));
    title_tree_widget->setSizePolicy(makePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    title_tree_widget->setHeaderHidden(true);
    title_tree_widget->setColumnCount(2);

    template_title = new QLineEdit(this);
    template_title->setObjectName(QStringLiteral("templateTitle"));
    {
        QFont font;
        font.setPointSize(TITLE_FONT_POINTSIZE);
        template_title->setFont(font);
    }
    template_title->setStyleSheet(QString::fromUtf8(TEMPLATE_TITLE_STYLE));

    delete_button = new QPushButton(this);
    delete_button->setObjectName(QStringLiteral("deleteButton"));
    delete_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

    editor_separator = makeSeparator(this);

    template_text = new QPlainTextEdit(this);
    template_text->setObjectName(QStringLiteral("templateText"));
    {
        QFont font;
        font.setPointSize(TEXT_FONT_POINTSIZE);
        template_text->setFont(font);
    }
    template_text->setStyleSheet(QString::fromUtf8(TEMPLATE_TEXT_STYLE));

    copy_button = new QPushButton(this);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    copy_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

    save_button = new QPushButton(this);
    save_button->setObjectName(QStringLiteral("saveButton"));
    save_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
}

void phraseGeneration::layoutWidgets()
{
    auto *root_layout = new QHBoxLayout(this);
    root_layout->setContentsMargins(0, 0, 0, 0);
    root_layout->setSpacing(6);

    editor_group = new QGroupBox(this);
    editor_group->setObjectName(QStringLiteral("editorGroup"));
    auto *editor_panel = new QVBoxLayout(editor_group);
    editor_panel->setContentsMargins(6, 6, 6, 6);
    editor_panel->setSpacing(6);

    auto *title_row = new QHBoxLayout();
    title_row->setContentsMargins(0, 0, 0, 0);
    title_row->setSpacing(6);
    title_row->addWidget(template_title, 1);
    title_row->addWidget(delete_button);
    editor_panel->addLayout(title_row);

    editor_panel->addWidget(editor_separator);
    editor_panel->addWidget(template_text, 1);

    auto *action_row = new QHBoxLayout();
    action_row->setContentsMargins(0, 0, 0, 0);
    action_row->setSpacing(6);
    action_row->addStretch(1);
    action_row->addWidget(copy_button);
    action_row->addWidget(save_button);
    editor_panel->addLayout(action_row);

    tree_group = new QGroupBox(this);
    tree_group->setObjectName(QStringLiteral("treeGroup"));
    auto *tree_panel = new QVBoxLayout(tree_group);
    tree_panel->setContentsMargins(6, 6, 6, 6);
    tree_panel->setSpacing(6);
    tree_panel->addWidget(add_button);
    tree_panel->addWidget(tree_separator);
    tree_panel->addWidget(title_tree_widget, 1);

    root_layout->addWidget(editor_group, EDITOR_PANEL_STRETCH);
    root_layout->addWidget(tree_group, TREE_PANEL_STRETCH);
}

void phraseGeneration::setupShortcuts()
{
    auto *save_sc = new QShortcut(QKeySequence::Save, this);
    connect(save_sc, &QShortcut::activated, this, &phraseGeneration::handleSaveButtonClick);

    auto *new_sc = new QShortcut(QKeySequence::New, this);
    connect(new_sc, &QShortcut::activated, this, &phraseGeneration::handleAddButtonClick);

    auto *copy_sc = new QShortcut(QKeySequence(tr("Ctrl+Shift+C")), this);
    connect(copy_sc, &QShortcut::activated, this, &phraseGeneration::handleCopyButtonClick);

    auto *delete_sc = new QShortcut(QKeySequence::Delete, this);
    connect(delete_sc, &QShortcut::activated, this, &phraseGeneration::handleDeleteButtonClick);
}

void phraseGeneration::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        adjustTreeColumnWidth();
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

void phraseGeneration::loadTitles()
{
    title_tree_widget->clear();
    QDir const directory("content");
    QStringList const files = directory.entryList(QStringList() << "*.txt", QDir::Files);
    // NOLINTNEXTLINE(misc-const-correctness)
    foreach (QString filename, files) {
        QString title;
        QString const content = loadContent(filename, &title);

        auto *item = new QTreeWidgetItem(title_tree_widget);
        item->setText(0, title);

        item->setData(0, Qt::UserRole, filename);

        auto *copyButton = new QPushButton(tr("Copy"), title_tree_widget);
        connect(copyButton, &QPushButton::clicked, this, &phraseGeneration::copyContent);

        title_tree_widget->setItemWidget(item, 1, copyButton);
    }
}

QString phraseGeneration::loadContent(const QString &filename, QString *title)
{
    QFile file("content/" + filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        if (title != nullptr) {
            *title = in.readLine();
        }
        return in.readAll();
    }
    return "";
}

void phraseGeneration::handleAddButtonClick()
{
    currentFile.clear();
    template_text->clear();
    template_title->clear();
    template_title->setFocus();
}

void phraseGeneration::handleSaveButtonClick()
{
    QString const title = template_title->text();
    QString const content = template_text->toPlainText();

    if (title.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Title cannot be empty."));
        return;
    }

    if (!currentFile.isEmpty()) {
        QFile file("content/" + currentFile);
        if (file.exists()) {
            file.remove();
        }
    }

    saveContent(title, content);
    loadTitles();
    template_title->clear();
    template_text->clear();

    currentFile.clear();
}

void phraseGeneration::saveContent(const QString &title, const QString &content)
{
    QDir().mkpath("content");

    QString const uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString const filename = uuid + ".txt";
    QFile file("content/" + filename);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << title << "\n" << content;
        file.close();
    }
}

void phraseGeneration::handleCopyButtonClick()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString const content = template_text->toPlainText();
    clipboard->setText(content);
    QMessageBox::information(this, tr("Copied"), tr("Text copied to clipboard."));
}

void phraseGeneration::handleDeleteButtonClick()
{
    QTreeWidgetItem const *item = title_tree_widget->currentItem();
    if (item == nullptr) {
        QMessageBox::warning(this, tr("Warning"), tr("No title selected."));
        return;
    }

    QString const itemTitle = item->text(0);
    auto const result = QMessageBox::question(
        this, tr("Confirm Delete"), tr("Are you sure you want to delete \"%1\"?").arg(itemTitle),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    QString const filename = item->data(0, Qt::UserRole).toString();
    deleteContent(filename);
    loadTitles();
    template_title->clear();
    template_text->clear();
}

void phraseGeneration::deleteContent(const QString &filename)
{
    QFile file("content/" + filename);
    if (!file.remove()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete the file."));
    }
}

void phraseGeneration::handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int /*column*/)
{
    QString const filename = item->data(0, Qt::UserRole).toString();
    QString title;
    QString const content = loadContent(filename, &title);

    template_title->setText(title);
    template_text->setPlainText(content);

    currentFile = filename;
}

void phraseGeneration::copyContent()
{
    auto const *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr) {
        return;
    }

    QTreeWidgetItem const *item = nullptr;
    for (int i = 0; i < title_tree_widget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *currentItem = title_tree_widget->topLevelItem(i);
        if (title_tree_widget->itemWidget(currentItem, 1) == button) {
            item = currentItem;
            break;
        }
    }

    if (item == nullptr) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to find the corresponding item."));
        return;
    }

    QString const filename = item->data(0, Qt::UserRole).toString();

    QString title;
    QString content = loadContent(filename, &title);

    if (content.startsWith("\n")) {
        content.remove(0, 1);
    }

    QApplication::clipboard()->setText(content);
    QMessageBox::information(this, tr("Copied"), tr("Text copied to clipboard."));
}
