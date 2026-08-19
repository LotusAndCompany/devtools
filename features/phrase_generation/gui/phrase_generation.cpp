#include "phrase_generation.h"

#include "features/framework/gui/design_system.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
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
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QVBoxLayout>

phraseGeneration::phraseGeneration(QWidget *parent) : QWidget(parent)
{
    buildUi();

    connect(add_button, &QPushButton::clicked, this, &phraseGeneration::handleAddButtonClick);
    connect(save_button, &QPushButton::clicked, this, &phraseGeneration::handleSaveButtonClick);
    connect(copy_button, &QPushButton::clicked, this, &phraseGeneration::handleCopyButtonClick);
    connect(delete_button, &QPushButton::clicked, this, &phraseGeneration::handleDeleteButtonClick);
    connect(title_tree_widget, &QTreeWidget::itemClicked, this,
            &phraseGeneration::handleTitleTreeWidgetItemClick);

    title_tree_widget->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    setupShortcuts();
    loadTitles();
}

void phraseGeneration::buildUi()
{
    setObjectName(QStringLiteral("phraseGeneration"));

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
}

void phraseGeneration::createWidgets()
{
    add_button = new QPushButton(this);
    add_button->setObjectName(QStringLiteral("addButton"));
    DevTools::Ui::configurePrimaryButton(add_button);
    {
        QIcon const icon = QIcon::fromTheme(QStringLiteral("add"));
        if (!icon.isNull()) {
            add_button->setIcon(icon);
        }
    }

    tree_separator = new QFrame(this);
    DevTools::Ui::configureDivider(tree_separator);

    title_tree_widget = new QTreeWidget(this);
    title_tree_widget->setObjectName(QStringLiteral("titleTreeWidget"));
    title_tree_widget->setHeaderHidden(true);
    title_tree_widget->setColumnCount(1);

    template_title = new QLineEdit(this);
    template_title->setObjectName(QStringLiteral("templateTitle"));

    delete_button = new QPushButton(this);
    delete_button->setObjectName(QStringLiteral("deleteButton"));
    DevTools::Ui::configureCompactButton(delete_button);
    {
        QIcon const icon = QIcon::fromTheme(QStringLiteral("delete"));
        if (!icon.isNull()) {
            delete_button->setIcon(icon);
        }
    }

    editor_separator = new QFrame(this);
    DevTools::Ui::configureDivider(editor_separator);

    template_text = new QPlainTextEdit(this);
    template_text->setObjectName(QStringLiteral("templateText"));
    DevTools::Ui::configureCodeEditor(template_text);

    copy_button = new QPushButton(this);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    DevTools::Ui::configureCompactButton(copy_button);
    {
        QIcon const icon = QIcon::fromTheme(QStringLiteral("content_copy"));
        if (!icon.isNull()) {
            copy_button->setIcon(icon);
        }
    }

    save_button = new QPushButton(this);
    save_button->setObjectName(QStringLiteral("saveButton"));
    DevTools::Ui::configureCompactButton(save_button);
}

void phraseGeneration::layoutWidgets()
{
    auto *root_layout = new QHBoxLayout(this);
    DevTools::Ui::applyPageLayout(root_layout);

    editor_group = new QGroupBox(this);
    editor_group->setObjectName(QStringLiteral("editorGroup"));
    auto *editor_panel = new QVBoxLayout(editor_group);
    DevTools::Ui::applyPanelLayout(editor_panel);

    auto *title_row = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(title_row);
    title_row->addWidget(template_title, 1);
    editor_panel->addLayout(title_row);

    editor_panel->addWidget(editor_separator);
    editor_panel->addWidget(template_text, 1);

    auto *action_row = new QHBoxLayout();
    action_row->addWidget(delete_button);
    action_row->addWidget(copy_button);
    action_row->addWidget(save_button);
    DevTools::Ui::configureActionBar(action_row, DevTools::Ui::ActionBarAlignment::Trailing);
    editor_panel->addLayout(action_row);

    tree_group = new QGroupBox(this);
    tree_group->setObjectName(QStringLiteral("treeGroup"));
    auto *tree_panel = new QVBoxLayout(tree_group);
    DevTools::Ui::applyPanelLayout(tree_panel);
    tree_panel->addWidget(add_button);
    tree_panel->addWidget(tree_separator);
    tree_panel->addWidget(title_tree_widget, 1);

    root_layout->addWidget(editor_group, DevTools::Ui::Metrics::MAIN_PANEL_STRETCH);
    root_layout->addWidget(tree_group, DevTools::Ui::Metrics::SIDE_PANEL_STRETCH);
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
    }
}

bool phraseGeneration::hasUnsavedChanges() const
{
    QString const current_title = template_title->text();
    QString const current_text = template_text->toPlainText();

    if (currentFile.isEmpty()) {
        return !current_title.isEmpty() || !current_text.isEmpty();
    }

    QString saved_title;
    QString const saved_text = loadContent(currentFile, &saved_title);
    return current_title != saved_title || current_text != saved_text;
}

void phraseGeneration::selectTreeItemByFilename(const QString &filename)
{
    for (int i = 0; i < title_tree_widget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *current = title_tree_widget->topLevelItem(i);
        if (current->data(0, Qt::UserRole).toString() == filename) {
            title_tree_widget->setCurrentItem(current);
            return;
        }
    }
}

bool phraseGeneration::confirmDiscard()
{
    if (!hasUnsavedChanges()) {
        return true;
    }

    auto const result = QMessageBox::question(
        this, tr("Unsaved Changes"), tr("You have unsaved changes. Save them?"),
        QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);

    return result == QMessageBox::Save ? (handleSaveButtonClick(), !hasUnsavedChanges()) : true;
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
    if (!confirmDiscard()) {
        return;
    }
    if (hasUnsavedChanges()) {
        return;
    }
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

    if (content.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Text cannot be empty."));
        return;
    }

    if (!currentFile.isEmpty()) {
        QFile file("content/" + currentFile);
        if (file.exists()) {
            file.remove();
        }
    }

    QString const saved_filename = saveContent(title, content);
    loadTitles();
    selectTreeItemByFilename(saved_filename);

    currentFile = saved_filename;
}

QString phraseGeneration::saveContent(const QString &title, const QString &content)
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
    return filename;
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
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

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
    if (!confirmDiscard()) {
        if (!currentFile.isEmpty()) {
            selectTreeItemByFilename(currentFile);
        } else {
            title_tree_widget->setCurrentItem(nullptr);
        }
        return;
    }
    QString title;
    QString const content = loadContent(filename, &title);

    template_title->setText(title);
    template_text->setPlainText(content);

    currentFile = filename;

    selectTreeItemByFilename(filename);
}
