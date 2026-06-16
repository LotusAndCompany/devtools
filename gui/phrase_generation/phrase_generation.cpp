#include "phrase_generation.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>

#include <algorithm>

namespace {
static constexpr const char *SAVE_BUTTON_STYLE =
    "margin-right: 30px;\n"
    "margin-bottom: 15px;\n"
    "padding-top: 3px;\n"
    "padding-bottom: 3px;\n"
    "padding-left: 10px;\n"
    "padding-right: 10px;\n"
    "border-radius: 6px;\n"
    "background-color: rgb(175, 174, 177);";
static constexpr const char *TEMPLATE_TITLE_STYLE = "QLineEdit {\n    padding: 2px 0px 2px 2px;\n}";
static constexpr const char *TEMPLATE_TEXT_STYLE =
    "QPlainTextEdit {\n    padding: 0px 0px 5px 0px;\n}";

constexpr int DEFAULT_WIDTH = 1012;
constexpr int DEFAULT_HEIGHT = 633;
constexpr int MIN_WIDTH = 300;
constexpr int MIN_HEIGHT = 200;
constexpr int TREE_FIXED_COLUMN_WIDTH = 40;
constexpr int TREE_COLUMN_PADDING = 8;
constexpr int LIGHTNESS_THRESHOLD = 128;

constexpr int TITLE_FONT_POINTSIZE = 28;
constexpr int TEXT_FONT_POINTSIZE = 20;
constexpr int ADD_FONT_POINTSIZE = 30;
constexpr int TOGGLE_FONT_POINTSIZE = 27;

QString treeStyleSheet(const QColor &border_color)
{
    return QString(
               "QTreeWidget::item { border-bottom: 1px solid %1; padding: 5px; }"
               "QTreeWidget::item:selected { background-color: #0078d7; color: "
               "#ffffff; }")
        .arg(border_color.name());
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
    connect(title_tree_widget, &QTreeWidget::itemClicked, this,
            &phraseGeneration::handleTitleTreeWidgetItemClick);

    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    title_tree_widget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    title_tree_widget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    adjustTreeColumnWidth();

    // ダークモードかライトモードかを判定してQTreeWidgetのリストの要素のボーダーカラーを決める
    QPalette const palette = this->palette();
    QColor const baseColor = palette.color(QPalette::Base);
    QColor const borderColor =
        (baseColor.lightness() > LIGHTNESS_THRESHOLD) ? Qt::black : Qt::white;
    title_tree_widget->setStyleSheet(treeStyleSheet(borderColor));

    title_tree_widget->setVisible(false);
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
} // namespace

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

    // 各 tree item の Copy ボタンも再翻訳
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
    // 翻訳された Copy ボタンが切れないように sample の sizeHint から算出する
    QPushButton const sample(tr("Copy"));
    int const min_width =
        std::max(TREE_FIXED_COLUMN_WIDTH, sample.sizeHint().width() + TREE_COLUMN_PADDING);
    title_tree_widget->setColumnWidth(1, min_width);
}

void phraseGeneration::createTopBarWidgets()
{
    template_title = new QLineEdit(this);
    template_title->setObjectName(QStringLiteral("templateTitle"));
    template_title->setEnabled(true);
    {
        QFont font;
        font.setPointSize(TITLE_FONT_POINTSIZE);
        template_title->setFont(font);
    }
    template_title->setStyleSheet(QString::fromUtf8(TEMPLATE_TITLE_STYLE));

    delete_button = new QPushButton(this);
    delete_button->setObjectName(QStringLiteral("deleteButton"));
    delete_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

    copy_button = new QPushButton(this);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    copy_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

    add_button = new QPushButton(this);
    add_button->setObjectName(QStringLiteral("addButton"));
    add_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    {
        QFont font;
        font.setPointSize(ADD_FONT_POINTSIZE);
        add_button->setFont(font);
    }
    {
        QIcon const icon = QIcon::fromTheme(QStringLiteral("add"));
        if (!icon.isNull()) {
            add_button->setIcon(icon);
        } else {
            add_button->setText(QStringLiteral("+"));
        }
    }

    toggle_tree_button = new QPushButton(this);
    toggle_tree_button->setObjectName(QStringLiteral("toggleTreeButton"));
    toggle_tree_button->setSizePolicy(makePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    {
        QFont font(QStringLiteral("Symbol"));
        font.setPointSize(TOGGLE_FONT_POINTSIZE);
        font.setBold(true);
        toggle_tree_button->setFont(font);
    }
    applyToggleButtonIcon(QStringLiteral("menu"), QStringLiteral("☰"));
}

void phraseGeneration::createBodyWidgets()
{
    line = new QFrame(this);
    line->setObjectName(QStringLiteral("line"));
    line->setSizePolicy(makePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    template_text = new QPlainTextEdit(this);
    template_text->setObjectName(QStringLiteral("templateText"));
    template_text->setEnabled(true);
    {
        QFont font;
        font.setPointSize(TEXT_FONT_POINTSIZE);
        template_text->setFont(font);
    }
    template_text->setStyleSheet(QString::fromUtf8(TEMPLATE_TEXT_STYLE));

    title_tree_widget = new QTreeWidget(this);
    title_tree_widget->setObjectName(QStringLiteral("titleTreeWidget"));
    title_tree_widget->setSizePolicy(
        makePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding));
    title_tree_widget->setHeaderHidden(true);
    title_tree_widget->setColumnCount(2);

    save_button = new QPushButton(this);
    save_button->setObjectName(QStringLiteral("saveButton"));
    save_button->setSizePolicy(makePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    {
        QFont font;
        font.setKerning(true);
        save_button->setFont(font);
    }
    save_button->setStyleSheet(QString::fromUtf8(SAVE_BUTTON_STYLE));
}

void phraseGeneration::layoutWidgets()
{
    auto *grid_layout = new QGridLayout(this);
    grid_layout->setObjectName(QStringLiteral("gridLayout"));

    grid_layout->addWidget(template_title, 0, 0, 1, 2);
    grid_layout->addWidget(delete_button, 0, 2, 1, 1);
    grid_layout->addWidget(copy_button, 0, 3, 1, 1);
    grid_layout->addWidget(add_button, 0, 4, 1, 1);
    grid_layout->addWidget(toggle_tree_button, 0, 6, 1, 1);
    grid_layout->addWidget(line, 1, 0, 1, 7);
    grid_layout->addWidget(template_text, 2, 0, 7, 7);
    grid_layout->addWidget(title_tree_widget, 2, 5, 7, 2);
    grid_layout->addWidget(save_button, 8, 5, 1, 1);

    // ストレッチ係数を設定
    grid_layout->setColumnStretch(0, 3); // 左側に多くスペースを割り当てる
    grid_layout->setColumnStretch(5, 1); // titleTreeWidget の列
}

// カラーテーマ / 言語変更時に走る処理
void phraseGeneration::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::PaletteChange: {
        QPalette const palette = this->palette();
        QPalette templateTextPalette = template_text->palette();
        templateTextPalette.setColor(QPalette::Base, palette.color(QPalette::Base));
        template_text->setPalette(templateTextPalette);

        QColor const baseColor = palette.color(QPalette::Base);
        QColor const borderColor =
            (baseColor.lightness() > LIGHTNESS_THRESHOLD) ? Qt::black : Qt::white;
        title_tree_widget->setStyleSheet(treeStyleSheet(borderColor));
        break;
    }
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

        // UUIDをユーザーデータとして保持
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
}

void phraseGeneration::handleSaveButtonClick()
{
    QString const title = template_title->text();
    QString const content = template_text->toPlainText();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Title cannot be empty.");
        return;
    }

    // 既存ファイルを選択している場合は一旦削除したのちに新しい内容で更新
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

    // UUIDを生成
    QString const uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // UUIDでファイルを一意化
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
    QTreeWidgetItem const *item = title_tree_widget->currentItem();
    if (item == nullptr) {
        QMessageBox::warning(this, "Warning", "No title selected.");
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
        QMessageBox::warning(this, "Error", "Failed to delete the file.");
    }
}

void phraseGeneration::handleToggleTreeButtonClick()
{
    bool const isVisible = title_tree_widget->isVisible();
    title_tree_widget->setVisible(!isVisible);

    auto *grid = qobject_cast<QGridLayout *>(this->layout());
    if (grid == nullptr) {
        return;
    }

    // ボタンのアイコン/テキストを切り替える
    if (title_tree_widget->isVisible()) {
        applyToggleButtonIcon(QStringLiteral("close"), QStringLiteral("✕"));
        grid->removeWidget(template_text);
        grid->addWidget(template_text, 2, 0, 7, 5);
        grid->removeWidget(save_button);
        grid->addWidget(save_button, 8, 4, 1, 1);
    } else {
        applyToggleButtonIcon(QStringLiteral("menu"), QStringLiteral("☰"));
        grid->removeWidget(template_text);
        grid->addWidget(template_text, 2, 0, 7, 7);
        grid->removeWidget(save_button);
        grid->addWidget(save_button, 8, 5, 1, 1);
    }
}

void phraseGeneration::applyToggleButtonIcon(const QString &theme_name,
                                             const QString &fallback_text)
{
    QIcon const icon = QIcon::fromTheme(theme_name);
    if (!icon.isNull()) {
        toggle_tree_button->setIcon(icon);
        toggle_tree_button->setText(QString());
    } else {
        toggle_tree_button->setIcon(QIcon());
        toggle_tree_button->setText(fallback_text);
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

    // ボタンから直接アイテムを取得
    QTreeWidgetItem const *item = nullptr;
    for (int i = 0; i < title_tree_widget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *currentItem = title_tree_widget->topLevelItem(i);
        if (title_tree_widget->itemWidget(currentItem, 1) == button) {
            item = currentItem;
            break;
        }
    }

    if (item == nullptr) {
        QMessageBox::warning(this, "Error", "Unable to find the corresponding item.");
        return;
    }

    QString const filename = item->data(0, Qt::UserRole).toString();

    QString title;
    // 本文のみ取得
    QString content = loadContent(filename, &title);

    // もし改行で始まっていたら取り除く
    if (content.startsWith("\n")) {
        content.remove(0, 1);
    }

    // コピー
    QApplication::clipboard()->setText(content);
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}
