#include "phrase_generation.h"
#include "ui_phrase_generation.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QUuid>
#include <QGridLayout>

phraseGeneration::phraseGeneration(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::phraseGeneration)
{
    ui->setupUi(this);

    connect(ui->addButton, &QPushButton::clicked, this, &phraseGeneration::handleAddButtonClick);
    connect(ui->saveButton, &QPushButton::clicked, this, &phraseGeneration::handleSaveButtonClick);
    connect(ui->copyButton, &QPushButton::clicked, this, &phraseGeneration::handleCopyButtonClick);
    connect(ui->deleteButton, &QPushButton::clicked, this, &phraseGeneration::handleDeleteButtonClick);
    connect(ui->toggleTreeButton, &QPushButton::clicked, this, &phraseGeneration::handleToggleTreeButtonClick);
    connect(ui->titleTreeWidget, &QTreeWidget::itemClicked, this, &phraseGeneration::handleTitleTreeWidgetItemClick);

    QGridLayout *gridLayout = new QGridLayout(this);

    gridLayout->addWidget(ui->templateTitle, 0, 0, 1, 2);
    gridLayout->addWidget(ui->deleteButton, 0, 2, 1, 1);
    gridLayout->addWidget(ui->copyButton, 0, 3, 1, 1);
    gridLayout->addWidget(ui->addButton, 0, 4, 1, 1);
    gridLayout->addWidget(ui->toggleTreeButton, 0, 6, 1, 1);
    gridLayout->addWidget(ui->line, 1, 0, 1, 7);
    gridLayout->addWidget(ui->templateText, 2, 0, 7, 7);
    gridLayout->addWidget(ui->titleTreeWidget, 2, 5, 7, 2);
    gridLayout->addWidget(ui->saveButton, 8, 5, 1, 1);

    // ストレッチ係数を設定
    gridLayout->setColumnStretch(0, 3); // 左側に多くスペースを割り当てる
    gridLayout->setColumnStretch(5, 1); // titleTreeWidget の列

    this->setMinimumSize(300, 200);

    this->setLayout(gridLayout);


    ui->titleTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->titleTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->titleTreeWidget->setColumnWidth(1, 40);

    ui->titleTreeWidget->setVisible(false);
    loadTitles();
}

phraseGeneration::~phraseGeneration()
{
    delete ui;
}

//テキスト表示部分がカラーテーマ変更に追従しないため明示的に設定
void phraseGeneration::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange) {
        QPalette palette = this->palette();
        QPalette templateTextPalette = ui->templateText->palette();
        templateTextPalette.setColor(QPalette::Base, palette.color(QPalette::Base));
        ui->templateText->setPalette(templateTextPalette);
    }
    QWidget::changeEvent(event);
}

void phraseGeneration::loadTitles()
{
    ui->titleTreeWidget->clear();
    QDir directory("content");
    QStringList files = directory.entryList(QStringList() << "*.txt", QDir::Files);
    foreach(QString filename, files) {
        QString title;
        QString content = loadContent(filename, &title);

        QTreeWidgetItem *item = new QTreeWidgetItem(ui->titleTreeWidget);
        item->setText(0, title);

        // UUIDをユーザーデータとして保持
        item->setData(0, Qt::UserRole, filename);

        QPushButton *copyButton = new QPushButton(tr("Copy"), ui->titleTreeWidget);
        connect(copyButton, &QPushButton::clicked, this, &phraseGeneration::copyContent);

        ui->titleTreeWidget->setItemWidget(item, 1, copyButton);
    }
}

QString phraseGeneration::loadContent(const QString &filename, QString *title)
{
    QFile file("content/" + filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        if (title) {
            *title = in.readLine();
        }
        return in.readAll();
    }
    return "";
}

void phraseGeneration::handleAddButtonClick()
{
    currentFile.clear();
    ui->templateText->clear();
    ui->templateTitle->clear();
}

void phraseGeneration::handleSaveButtonClick()
{
    QString title = ui->templateTitle->text();
    QString content = ui->templateText->toPlainText();

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
    ui->templateTitle->clear();
    ui->templateText->clear();

    currentFile.clear();
}

void phraseGeneration::saveContent(const QString &title, const QString &content)
{
    QDir().mkpath("content");

    // UUIDを生成
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // UUIDでファイルを一意化
    QString filename = uuid + ".txt";
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
    QString content = ui->templateText->toPlainText();
    clipboard->setText(content);
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}

void phraseGeneration:: handleDeleteButtonClick()
{
    QTreeWidgetItem *item = ui->titleTreeWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "No title selected.");
        return;
    }

    QString filename = item->data(0, Qt::UserRole).toString();
    deleteContent(filename);
    loadTitles();
    ui->templateTitle->clear();
    ui->templateText->clear();
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
    bool isVisible = ui->titleTreeWidget->isVisible();
    ui->titleTreeWidget->setVisible(!isVisible);

    // ボタンのテキストを切り替える
    if (ui->titleTreeWidget->isVisible()) {
        ui->toggleTreeButton->setIcon(QIcon::fromTheme("close"));
        this->layout()->removeWidget(ui->templateText);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->templateText, 2, 0, 7, 5);
        this->layout()->removeWidget(ui->saveButton);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->saveButton, 8, 4, 1, 1);
    } else {
        ui->toggleTreeButton->setIcon(QIcon::fromTheme("menu"));
        this->layout()->removeWidget(ui->templateText);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->templateText, 2, 0, 7, 7);
        this->layout()->removeWidget(ui->saveButton);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->saveButton, 8, 5, 1, 1);
    }
}

void phraseGeneration::handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column)
{
    QString filename = item->data(0, Qt::UserRole).toString();
    QString title;
    QString content = loadContent(filename, &title);

    ui->templateTitle->setText(title);
    ui->templateText->setPlainText(content);

    currentFile = filename;
}

void phraseGeneration::copyContent()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        // ボタンから直接アイテムを取得
        QWidget *parentWidget = button->parentWidget();
        QTreeWidgetItem *item = nullptr;

        for (int i = 0; i < ui->titleTreeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem *currentItem = ui->titleTreeWidget->topLevelItem(i);
            if (ui->titleTreeWidget->itemWidget(currentItem, 1) == button) {
                item = currentItem;
                break;
            }
        }

        if (item) {
            QString filename = item->data(0, Qt::UserRole).toString();
            QString content = loadContent(filename);
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(content);
            QMessageBox::information(this, "Copied", "Text copied to clipboard.");
        } else {
            QMessageBox::warning(this, "Error", "Unable to find the corresponding item.");
        }
    }
}
