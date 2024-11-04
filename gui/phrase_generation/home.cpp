#include "home.h"
#include "ui_home.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QUuid>

home::home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::home)
{
    ui->setupUi(this);

    connect(ui->addButton, &QPushButton::clicked, this, &home::handleAddButtonClick);
    connect(ui->saveButton, &QPushButton::clicked, this, &home::handleSaveButtonClick);
    connect(ui->copyButton, &QPushButton::clicked, this, &home::handleCopyButtonClick);
    connect(ui->deleteButton, &QPushButton::clicked, this, &home::handleDeleteButtonClick);
    connect(ui->toggleTreeButton, &QPushButton::clicked, this, &home::handleToggleTreeButtonClick);
    connect(ui->titleTreeWidget, &QTreeWidget::itemClicked, this, &home::handleTitleTreeWidgetItemClick);

    QGridLayout *gridLayout = new QGridLayout(this);

    QWidget *horizontalWidget = new QWidget(this);
    horizontalWidget->setLayout(ui->horizontalLayout);
    gridLayout->addWidget(horizontalWidget, 0, 0, 1, 6);
    gridLayout->addWidget(ui->line, 1, 0, 1, 6);
    gridLayout->addWidget(ui->templateText, 2, 0, 7, 6);
    gridLayout->addWidget(ui->titleTreeWidget, 2, 5, 7, 1);
    gridLayout->addWidget(ui->saveButton, 8, 5, Qt::AlignBottom | Qt::AlignRight);

    this->setLayout(gridLayout);

    ui->titleTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->titleTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->titleTreeWidget->setColumnWidth(1, 40);

    ui->titleTreeWidget->setVisible(false);
    loadTitles();
}

home::~home()
{
    delete ui;
}

void home::loadTitles()
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

        QPushButton *copyButton = new QPushButton("Copy", ui->titleTreeWidget);
        connect(copyButton, &QPushButton::clicked, this, &home::copyContent);

        ui->titleTreeWidget->setItemWidget(item, 1, copyButton);
    }
}

QString home::loadContent(const QString &filename, QString *title)
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

void home::handleAddButtonClick()
{
    currentFile.clear();
    ui->templateText->clear();
    ui->templateTitle->clear();
}

void home::handleSaveButtonClick()
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

void home::saveContent(const QString &title, const QString &content)
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

void home::handleCopyButtonClick()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString content = ui->templateText->toPlainText();
    clipboard->setText(content);
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}

void home:: handleDeleteButtonClick()
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

void home::deleteContent(const QString &filename)
{
    QFile file("content/" + filename);
    if (!file.remove()) {
        QMessageBox::warning(this, "Error", "Failed to delete the file.");
    }
}

void home::handleToggleTreeButtonClick()
{
    bool isVisible = ui->titleTreeWidget->isVisible();
    ui->titleTreeWidget->setVisible(!isVisible);

    // ボタンのテキストを切り替える
    if (ui->titleTreeWidget->isVisible()) {
        ui->toggleTreeButton->setText("X");
        this->layout()->removeWidget(ui->templateText);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->templateText, 2, 0, 7, 5);
        this->layout()->removeWidget(ui->saveButton);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->saveButton, 8, 4, Qt::AlignBottom | Qt::AlignRight);
    } else {
        ui->toggleTreeButton->setText("三");
        this->layout()->removeWidget(ui->templateText);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->templateText, 2, 0, 7, 6);
        this->layout()->removeWidget(ui->saveButton);
        static_cast<QGridLayout*>(this->layout())->addWidget(ui->saveButton, 8, 5, Qt::AlignBottom | Qt::AlignRight);
    }
}

void home::handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column)
{
    QString filename = item->data(0, Qt::UserRole).toString();
    QString title;
    QString content = loadContent(filename, &title);

    ui->templateTitle->setText(title);
    ui->templateText->setPlainText(content);

    currentFile = filename;
}

void home::copyContent()
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
