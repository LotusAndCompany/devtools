#include "phrase_generation.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStyle>
#include <QTextStream>
#include <QUuid>
#include <QVBoxLayout>

#include <algorithm>

namespace {

constexpr int DEFAULT_WIDTH = 1012;
constexpr int DEFAULT_HEIGHT = 633;
constexpr int MIN_WIDTH = 300;
constexpr int MIN_HEIGHT = 200;
constexpr int LIGHTNESS_THRESHOLD = 128;
constexpr int TREE_SIDEBAR_WIDTH = 320;
constexpr int LIST_ITEM_SPACING = 4;

constexpr int TITLE_FONT_POINTSIZE = 16;
constexpr int TEXT_FONT_POINTSIZE = 14;
constexpr int BTN_HEIGHT = 40;

QString darkButtonStyleSheet()
{
    return QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: 1px solid #B8B9B6;"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  color: #FFFFFF;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #2E2E2E; }");
}

QString darkTitleStyleSheet()
{
    return QStringLiteral(
        "QLineEdit {"
        "  border: 1px solid #2E2E2E;"
        "  border-radius: 20px;"
        "  padding: 8px 16px;"
        "  background-color: #111111;"
        "  color: #FFFFFF;"
        "  font-size: 16px;"
        "}");
}

QString darkEditorStyleSheet()
{
    return QStringLiteral(
        "#editorFrame {"
        "  border: 1px solid #2E2E2E;"
        "  border-radius: 16px;"
        "  background-color: #1A1A1A;"
        "}"
        "QPlainTextEdit {"
        "  border: none;"
        "  background-color: transparent;"
        "  color: #B8B9B6;"
        "  font-size: 14px;"
        "  padding: 16px;"
        "}");
}

QString darkToolbarStyleSheet()
{
    return QStringLiteral(
        "#toolbarFrame {"
        "  border-bottom: 1px solid #2E2E2E;"
        "  background-color: transparent;"
        "}");
}

QString darkSidebarStyleSheet()
{
    return QStringLiteral(
        "#sidebarContainer {"
        "  border-left: 1px solid #2E2E2E;"
        "  background-color: #111111;"
        "}"
        "#listHeader {"
        "  border-bottom: 1px solid #2E2E2E;"
        "  background-color: transparent;"
        "}"
        "#listHeader QLabel {"
        "  color: #FFFFFF;"
        "}");
}

QString darkListItemStyleSheet()
{
    return QStringLiteral(
        "QFrame[filename] {"
        "  background-color: #383838;"
        "  border-radius: 8px;"
        "}"
        "QFrame[filename][selected=true] {"
        "  background-color: #4A4A4A;"
        "}"
        "QLabel#itemTitle {"
        "  color: #FFFFFF;"
        "  font-size: 14px;"
        "  background: transparent;"
        "}"
        "QPushButton#itemCopyBtn {"
        "  background: transparent;"
        "  border: 1px solid #B8B9B6;"
        "  border-radius: 4px;"
        "  padding: 4px 10px;"
        "  color: #B8B9B6;"
        "  font-size: 12px;"
        "}"
        "QPushButton#itemCopyBtn:hover {"
        "  background-color: #4A4A4A;"
        "  color: #FFFFFF;"
        "}");
}

QString lightButtonStyleSheet()
{
    return QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: 1px solid palette(mid);"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  color: palette(text);"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: palette(highlight); }");
}

QString lightTitleStyleSheet()
{
    return QStringLiteral(
        "QLineEdit {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 20px;"
        "  padding: 8px 16px;"
        "  background-color: palette(base);"
        "  color: palette(text);"
        "  font-size: 16px;"
        "}");
}

QString lightEditorStyleSheet()
{
    return QStringLiteral(
        "#editorFrame {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 16px;"
        "  background-color: palette(base);"
        "}"
        "QPlainTextEdit {"
        "  border: none;"
        "  background-color: transparent;"
        "  color: palette(text);"
        "  font-size: 14px;"
        "  padding: 16px;"
        "}");
}

QString lightToolbarStyleSheet()
{
    return QStringLiteral(
        "#toolbarFrame {"
        "  border-bottom: 1px solid palette(mid);"
        "  background-color: transparent;"
        "}");
}

QString lightSidebarStyleSheet()
{
    return QStringLiteral(
        "#sidebarContainer {"
        "  border-left: 1px solid palette(mid);"
        "  background-color: palette(base);"
        "}"
        "#listHeader {"
        "  border-bottom: 1px solid palette(mid);"
        "  background-color: transparent;"
        "}"
        "#listHeader QLabel {"
        "  color: palette(text);"
        "}");
}

QString lightListItemStyleSheet()
{
    return QStringLiteral(
        "QFrame[filename] {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 8px;"
        "  background-color: palette(base);"
        "}"
        "QFrame[filename][selected=true] {"
        "  background-color: #0078d7;"
        "}"
        "QLabel#itemTitle {"
        "  color: palette(text);"
        "  background: transparent;"
        "  font-size: 14px;"
        "}"
        "QPushButton#itemCopyBtn {"
        "  background: transparent;"
        "  border: 1px solid palette(mid);"
        "  border-radius: 4px;"
        "  padding: 4px 10px;"
        "  color: palette(text);"
        "  font-size: 12px;"
        "}"
        "QPushButton#itemCopyBtn:hover {"
        "  background-color: palette(highlight);"
        "}");
}

QSizePolicy makePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
{
    QSizePolicy policy(horizontal, vertical);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    return policy;
}

QString loadTitleOnly(const QString &filename)
{
    QFile file("content/" + filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        return in.readLine().trimmed();
    }
    return {};
}
} // namespace

phraseGeneration::phraseGeneration(QWidget *parent) : QWidget(parent)
{
    buildUi();

    connect(add_button, &QPushButton::clicked, this, &phraseGeneration::handleAddButtonClick);
    connect(save_button, &QPushButton::clicked, this, &phraseGeneration::handleSaveButtonClick);
    connect(copy_button, &QPushButton::clicked, this, &phraseGeneration::handleCopyButtonClick);
    connect(delete_button, &QPushButton::clicked, this, &phraseGeneration::handleDeleteButtonClick);
    connect(toggle_tree_button, &QPushButton::clicked, this,
            &phraseGeneration::handleToggleTreeButtonClick);

    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    applyStyles();
    loadTitles();
}

void phraseGeneration::buildUi()
{
    setObjectName(QStringLiteral("phraseGeneration"));
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    createTopBarWidgets();
    createBodyWidgets();
    layoutWidgets();
    retranslateUi();
}

void phraseGeneration::retranslateUi()
{
    setWindowTitle(tr("Form"));
    template_title->setPlaceholderText(tr("Title"));
    template_text->setPlaceholderText(tr("Text"));
    delete_button->setText(tr("Delete"));
    copy_button->setText(tr("Copy"));
    save_button->setText(tr("Save"));

    QString const add_label = tr("Add new template");
    add_button->setToolTip(add_label);
    add_button->setAccessibleName(add_label);
    add_button->setAccessibleDescription(add_label);

    QString const toggle_label = tr("Toggle template list");
    toggle_tree_button->setToolTip(toggle_label);
    toggle_tree_button->setAccessibleName(toggle_label);
    toggle_tree_button->setAccessibleDescription(toggle_label);

    list_header_label->setText(tr("Saved Phrases"));

    for (int i = 0; i < list_layout->count(); ++i) {
        QLayoutItem *layoutItem = list_layout->itemAt(i);
        if (layoutItem == nullptr || layoutItem->widget() == nullptr) {
            continue;
        }
        auto *frame = qobject_cast<QFrame *>(layoutItem->widget());
        if (frame == nullptr) {
            continue;
        }
        auto *copy_btn = frame->findChild<QPushButton *>(QStringLiteral("itemCopyBtn"));
        if (copy_btn != nullptr) {
            copy_btn->setText(tr("Copy"));
        }
    }
}

void phraseGeneration::createTopBarWidgets()
{
    toolbar_frame = new QFrame(this);
    toolbar_frame->setObjectName(QStringLiteral("toolbarFrame"));
    toolbar_frame->setFixedHeight(56);

    template_title = new QLineEdit(toolbar_frame);
    template_title->setObjectName(QStringLiteral("templateTitle"));
    template_title->setEnabled(true);
    template_title->setFixedHeight(BTN_HEIGHT);
    {
        QFont font;
        font.setPointSize(TITLE_FONT_POINTSIZE);
        font.setWeight(QFont::DemiBold);
        template_title->setFont(font);
    }

    auto createButton = [this](const QString &objName) -> QPushButton * {
        auto *btn = new QPushButton(toolbar_frame);
        btn->setObjectName(objName);
        btn->setFixedHeight(BTN_HEIGHT);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    delete_button = createButton(QStringLiteral("deleteButton"));
    copy_button = createButton(QStringLiteral("copyButton"));
    add_button = createButton(QStringLiteral("addButton"));
    add_button->setText(QStringLiteral("+"));

    toggle_tree_button = createButton(QStringLiteral("toggleTreeButton"));
    toggle_tree_button->setText(QStringLiteral("≡"));
}

void phraseGeneration::createBodyWidgets()
{
    editor_frame = new QFrame(this);
    editor_frame->setObjectName(QStringLiteral("editorFrame"));
    {
        auto *editor_inner_layout = new QVBoxLayout(editor_frame);
        editor_inner_layout->setContentsMargins(0, 0, 0, 0);

        template_text = new QPlainTextEdit(editor_frame);
        template_text->setObjectName(QStringLiteral("templateText"));
        template_text->setEnabled(true);
        template_text->setFrameShape(QFrame::NoFrame);
        template_text->setAttribute(Qt::WA_MacShowFocusRect, false);
        {
            QFont font;
            font.setPointSize(TEXT_FONT_POINTSIZE);
            template_text->setFont(font);
        }

        editor_inner_layout->addWidget(template_text, 1);
    }

    sidebar_container = new QWidget(this);
    sidebar_container->setObjectName(QStringLiteral("sidebarContainer"));
    sidebar_container->setFixedWidth(TREE_SIDEBAR_WIDTH);
    {
        auto *sidebar_layout = new QVBoxLayout(sidebar_container);
        sidebar_layout->setContentsMargins(0, 0, 0, 0);
        sidebar_layout->setSpacing(0);

        list_header = new QFrame(sidebar_container);
        list_header->setObjectName(QStringLiteral("listHeader"));
        list_header->setFixedHeight(48);

        list_header_label = new QLabel(tr("Saved Phrases"), list_header);
        {
            QFont font;
            font.setPointSize(14);
            font.setWeight(QFont::DemiBold);
            list_header_label->setFont(font);
        }

        auto *header_layout = new QHBoxLayout(list_header);
        header_layout->setContentsMargins(16, 0, 16, 0);
        header_layout->addWidget(list_header_label);

        sidebar_layout->addWidget(list_header);

        list_scroll = new QScrollArea(sidebar_container);
        list_scroll->setWidgetResizable(true);
        list_scroll->setFrameShape(QFrame::NoFrame);
        list_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_scroll->setSizePolicy(makePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

        list_content = new QWidget(list_scroll);
        list_layout = new QVBoxLayout(list_content);
        list_layout->setContentsMargins(0, 0, 0, 0);
        list_layout->setSpacing(LIST_ITEM_SPACING);

        list_scroll->setWidget(list_content);
        sidebar_layout->addWidget(list_scroll, 1);
    }

    sidebar_container->setVisible(false);

    save_button = new QPushButton(this);
    save_button->setObjectName(QStringLiteral("saveButton"));
    save_button->setFixedHeight(BTN_HEIGHT);
    save_button->setCursor(Qt::PointingHandCursor);
}

void phraseGeneration::layoutWidgets()
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setObjectName(QStringLiteral("mainLayout"));
    main_layout->setContentsMargins(0, 6, 6, 6);

    auto *toolbar_layout = new QHBoxLayout(toolbar_frame);
    toolbar_layout->setObjectName(QStringLiteral("toolbarLayout"));
    toolbar_layout->setContentsMargins(24, 0, 24, 0);
    toolbar_layout->setSpacing(8);
    toolbar_layout->addWidget(template_title, 1);
    toolbar_layout->addWidget(delete_button);
    toolbar_layout->addWidget(copy_button);
    toolbar_layout->addWidget(add_button);
    toolbar_layout->addWidget(toggle_tree_button);

    main_layout->addWidget(toolbar_frame);

    auto *body_layout = new QHBoxLayout();
    body_layout->setObjectName(QStringLiteral("bodyLayout"));
    body_layout->setSpacing(0);

    auto *editor_layout = new QVBoxLayout();
    editor_layout->setObjectName(QStringLiteral("editorLayout"));
    editor_layout->setContentsMargins(24, 24, 24, 24);
    editor_layout->setSpacing(16);
    editor_layout->addWidget(editor_frame, 1);
    editor_layout->addWidget(save_button, 0, Qt::AlignRight);

    body_layout->addLayout(editor_layout, 1);
    body_layout->addWidget(sidebar_container);

    main_layout->addLayout(body_layout, 1);
}

void phraseGeneration::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::PaletteChange:
        applyStyles();
        break;
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

bool phraseGeneration::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        while (widget != nullptr) {
            if (widget->parent() == list_content) {
                auto *frame = qobject_cast<QFrame *>(widget);
                if (frame != nullptr) {
                    onListItemClicked(frame);
                    return true;
                }
            }
            widget = widget->parentWidget();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void phraseGeneration::onListItemClicked(QFrame *item)
{
    if (current_list_item != nullptr) {
        current_list_item->setProperty("selected", false);
        current_list_item->style()->unpolish(current_list_item);
        current_list_item->style()->polish(current_list_item);
    }
    current_list_item = item;
    item->setProperty("selected", true);
    item->style()->unpolish(item);
    item->style()->polish(item);

    QString const filename = item->property("filename").toString();
    QString title;
    QString const content = loadContent(filename, &title);

    template_title->setText(title);
    template_text->setPlainText(content);

    currentFile = filename;
}

void phraseGeneration::loadTitles()
{
    current_list_item = nullptr;

    QLayoutItem *child;
    while ((child = list_layout->takeAt(0)) != nullptr) {
        if (child->widget() != nullptr) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    QDir const directory("content");
    QStringList const files = directory.entryList(QStringList() << "*.txt", QDir::Files);
    foreach (const QString &filename, files) {
        QString const title = loadTitleOnly(filename);

        auto *item_frame = new QFrame(list_content);
        item_frame->setProperty("filename", filename);
        item_frame->setCursor(Qt::PointingHandCursor);
        item_frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        item_frame->installEventFilter(this);

        auto *item_layout = new QHBoxLayout(item_frame);
        item_layout->setContentsMargins(16, 12, 16, 12);
        item_layout->setSpacing(8);

        auto *title_label = new QLabel(title, item_frame);
        title_label->setObjectName(QStringLiteral("itemTitle"));

        auto *copy_btn = new QPushButton(tr("Copy"), item_frame);
        copy_btn->setObjectName(QStringLiteral("itemCopyBtn"));
        copy_btn->setCursor(Qt::PointingHandCursor);
        connect(copy_btn, &QPushButton::clicked, this, &phraseGeneration::copyContent);

        item_layout->addWidget(title_label, 1);
        item_layout->addWidget(copy_btn);

        list_layout->addWidget(item_frame);
    }
    list_layout->addStretch();
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
    if (current_list_item != nullptr) {
        current_list_item->setProperty("selected", false);
        current_list_item->style()->unpolish(current_list_item);
        current_list_item->style()->polish(current_list_item);
        current_list_item = nullptr;
    }
}

void phraseGeneration::handleSaveButtonClick()
{
    QString const title = template_title->text();
    QString const content = template_text->toPlainText();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Title cannot be empty.");
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
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}

void phraseGeneration::handleDeleteButtonClick()
{
    if (current_list_item == nullptr) {
        QMessageBox::warning(this, "Warning", "No title selected.");
        return;
    }

    QString const filename = current_list_item->property("filename").toString();
    deleteContent(filename);
    loadTitles();
    template_title->clear();
    template_text->clear();
}

void phraseGeneration::deleteContent(const QString &filename)
{
    QFile file("content/" + filename);
    if (!file.remove()) {
        QMessageBox::warning(this, "Error", "Failed to delete the file.");
    }
}

void phraseGeneration::handleToggleTreeButtonClick()
{
    bool const isVisible = sidebar_container->isVisible();
    sidebar_container->setVisible(!isVisible);

    if (sidebar_container->isVisible()) {
        toggle_tree_button->setText(QStringLiteral("✕"));
    } else {
        toggle_tree_button->setText(QStringLiteral("≡"));
    }
}

void phraseGeneration::applyStyles()
{
    QPalette const palette = this->palette();
    QColor const baseColor = palette.color(QPalette::Base);
    bool const isDark = baseColor.lightness() < LIGHTNESS_THRESHOLD;

    if (isDark) {
        QString const btnStyle = darkButtonStyleSheet();
        for (auto *btn :
             {delete_button, copy_button, add_button, toggle_tree_button, save_button}) {
            btn->setStyleSheet(btnStyle);
        }
        template_title->setStyleSheet(darkTitleStyleSheet());
        editor_frame->setStyleSheet(darkEditorStyleSheet());
        toolbar_frame->setStyleSheet(darkToolbarStyleSheet());
        sidebar_container->setStyleSheet(darkSidebarStyleSheet());
        list_content->setStyleSheet(darkListItemStyleSheet());
    } else {
        QString const btnStyle = lightButtonStyleSheet();
        for (auto *btn :
             {delete_button, copy_button, add_button, toggle_tree_button, save_button}) {
            btn->setStyleSheet(btnStyle);
        }
        template_title->setStyleSheet(lightTitleStyleSheet());
        editor_frame->setStyleSheet(lightEditorStyleSheet());
        toolbar_frame->setStyleSheet(lightToolbarStyleSheet());
        sidebar_container->setStyleSheet(lightSidebarStyleSheet());
        list_content->setStyleSheet(lightListItemStyleSheet());
    }
}

void phraseGeneration::copyContent()
{
    auto const *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr) {
        return;
    }

    auto *item_frame = qobject_cast<QFrame *>(button->parent());
    if (item_frame == nullptr) {
        return;
    }

    QString const filename = item_frame->property("filename").toString();

    QString title;
    QString content = loadContent(filename, &title);

    if (content.startsWith("\n")) {
        content.remove(0, 1);
    }

    QApplication::clipboard()->setText(content);
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}
