#include "home.h"
#include "ui_home.h"
// #include "ItemWidget.h"

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

    ui->titleTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->titleTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->titleTreeWidget->setColumnWidth(1, 40);

    ui->titleTreeWidget->setStyleSheet(
        "QTreeWidget {"
        "   background-color: #555555;"
        "   color: #333333;"
        "}"
        "QTreeWidget::item {"
        "   background-color: #999999;"
        "   color: #000000;"
        "   border-bottom: 1px solid #d3d3d3;"
        "}"
        "QTreeWidget::item:selected {"
        "   background-color: #0078d7;"
        "   color: #ffffff;"
        "}"
    );

    ui->titleTreeWidget->setVisible(false);
    connect(ui->copyButton, &QPushButton::clicked, this, &home::on_copyButton_clicked);
    loadTitles();
}

home::~home()
{
    delete ui;
}

void home::on_addButton_clicked()
{
    ui->templateText->clear();
    ui->templateTitle->clear();
}


// void home::on_showTitleListButton_clicked()
// {
//     ui->titleList->setHidden(false);
//     ui->closeTitleListButton->setHidden(false);
//     // ui->label->setHidden(false);
//     // ui->label_2->setHidden(false);
//     // ui->label_3->setHidden(false);
//     // ui->label_4->setHidden(false);
// }

// void home::on_showTitleTreeWidgetButton_clicked()
// {
//     ui->titleTreeWidget->setHidden(false);
//     ui->closeTitleTreeWidgetButton->setHidden(false);
// }

// void home::on_closeTitleListButton_clicked()
// {
//     ui->titleList->setHidden(true);
//     ui->closeTitleTreeWidgetButton->setHidden(true);
//     // ui->label->setHidden(true);
//     // ui->label_2->setHidden(true);
//     // ui->label_3->setHidden(true);
//     // ui->label_4->setHidden(true);
// }

// void home::on_closeTitleTreeWidgetButton_clicked()
// {
//     ui->titleTreeWidget->setHidden(true);
//     ui->closeTitleTreeWidgetButton->setHidden(true);
// }

// void home::loadTitles()
// {
//     ui->titleList->clear();
//     QDir directory("content");
//     QStringList files = directory.entryList(QStringList() << "*.txt", QDir::Files);
//     foreach(QString filename, files) {
//         ui->titleList->addItem(filename.chopped(4));
//     }
// }

// void home::loadTitles()
// {
//     ui->titleList->clear();
//     QDir directory("content");
//     QStringList files = directory.entryList(QStringList() << "*.txt", QDir::Files);
//     foreach(QString filename, files) {
//         QString title = filename.chopped(4);
//         QListWidgetItem *item = new QListWidgetItem(ui->titleList);
//         ItemWidget *itemWidget = new ItemWidget(title, this);
//         connect(itemWidget, &ItemWidget::listCopyButtonClicked, this, &home::on_copyButton_clicked);
//         item->setSizeHint(itemWidget->sizeHint());
//         ui->titleList->setItemWidget(item, itemWidget);
//     }
// }

// void home::loadTitles()
// {
//     ui->titleList->clear();
//     QDir directory("content");
//     QStringList files = directory.entryList(QStringList() << "*.txt", QDir::Files);
//     foreach(QString filename, files) {
//         ui->titleList->addItem(filename.chopped(4));
//     }
// }

void home::loadTitles()
{
    ui->titleTreeWidget->clear();
    QDir directory("content");
    QStringList files = directory.entryList(QStringList() << "*.txt", QDir::Files);
    foreach(QString filename, files) {
        // QString title = filename.chopped(4);
        QString title = filename.section('_', 0, 0);

        QTreeWidgetItem *item = new QTreeWidgetItem(ui->titleTreeWidget);
        item->setText(0, title);

        // ファイル名全体をユーザーデータとして保持
        item->setData(0, Qt::UserRole, filename);

        QPushButton *copyButton = new QPushButton("Copy");
        connect(copyButton, &QPushButton::clicked, this, &home::copyContent);

        ui->titleTreeWidget->setItemWidget(item, 1, copyButton);
    }
}

// void home::saveContent(const QString &title, const QString &content)
// {
//     QDir().mkpath("content");
//     QFile file("content/" + title + ".txt");
//     if (file.open(QIODevice::WriteOnly)) {
//         QTextStream out(&file);
//         out << content;
//         file.close();
//     }
// }

void home::saveContent(const QString &title, const QString &content)
{
    QDir().mkpath("content");

    // UUIDを生成
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // タイトルにUUIDを追加してファイル名を一意化
    QString uniqueTitle = title + "_" + uuid;
    QFile file("content/" + uniqueTitle + ".txt");

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << content;
        file.close();
    }
}

// QString home::loadContent(const QString &title)
// {
//     QFile file("content/" + title + ".txt");
//     if (file.open(QIODevice::ReadOnly)) {
//         QTextStream in(&file);
//         return in.readAll();
//     }
//     return "";
// }

// QString home::loadContent(const QString &title)
// {
//     QDir directory("content");
//     QStringList files = directory.entryList(QStringList() << title + "_*.txt", QDir::Files);

//     if (!files.isEmpty()) {
//         QFile file("content/" + files.first());
//         if (file.open(QIODevice::ReadOnly)) {
//             QTextStream in(&file);
//             return in.readAll();
//         }
//     }
//     return "";
// }

QString home::loadContent(const QString &filename)
{
    QFile file("content/" + filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        return in.readAll();
    }
    return "";
}

// void home::deleteContent(const QString &title)
// {
//     QFile file("content/" + title + ".txt");
//     file.remove();
// }

// void home::deleteContent(const QString &title)
// {
//     QDir directory("content");
//     QStringList files = directory.entryList(QStringList() << title + "_*.txt", QDir::Files);

//     if (!files.isEmpty()) {
//         QFile file("content/" + files.first());
//         file.remove();
//     }
// }

void home::deleteContent(const QString &filename)
{
    QFile file("content/" + filename);
    file.remove();
}

void home::on_saveButton_clicked()
{
    QString title = ui->templateTitle->text();
    QString content = ui->templateText->toPlainText();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Title cannot be empty.");
        return;
    }

    saveContent(title, content);
    loadTitles();
    ui->templateTitle->clear();
    ui->templateText->clear();
}

// void home::on_deleteButton_clicked()
// {
//     QListWidgetItem *item = ui->titleList->currentItem();
//     if (!item) {
//         QMessageBox::warning(this, "Warning", "No title selected.");
//         return;
//     }

//     QString title = item->text();
//     deleteContent(title);
//     loadTitles();
//     ui->templateTitle->clear();
//     ui->templateText->clear();
// }

// void home::on_deleteButton_clicked()
// {
//     QTreeWidgetItem *item = ui->titleTreeWidget->currentItem();
//     if (!item) {
//         QMessageBox::warning(this, "Warning", "No title selected.");
//         return;
//     }

//     QString title = item->text(0);
//     deleteContent(title);
//     loadTitles();
//     ui->templateTitle->clear();
//     ui->templateText->clear();
// }

void home::on_deleteButton_clicked()
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

// void home::on_titleList_itemClicked(QListWidgetItem *item)
// {
//     QString title = item->text();
//     QString content = loadContent(title);
//     ui->templateTitle->setText(title);
//     ui->templateText->setPlainText(content);
// }

// void home::on_titleTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
// {
//     QString title = item->text(0);
//     QString content = loadContent(title);
//     ui->templateTitle->setText(title);
//     ui->templateText->setPlainText(content);
// }

void home::on_titleTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString filename = item->data(0, Qt::UserRole).toString();
    QString content = loadContent(filename);
    QString title = item->text(0);

    ui->templateTitle->setText(title);
    ui->templateText->setPlainText(content);
}

void home::copyContent()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        QWidget *itemWidget = button->parentWidget();
        QPoint pos = button->parentWidget()->pos();
        // QTreeWidgetItem *item = ui->titleTreeWidget->indexOfTopLevelItem(itemWidget);
        QTreeWidgetItem *item = ui->titleTreeWidget->itemAt(pos);

        if (item) {
            QString title = item->text(0);
            QString content = loadContent(title);
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(content);
            QMessageBox::information(this, "Copied", "Text copied to clipboard.");
        }
    }
}

void home::on_copyButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString content = ui->templateText->toPlainText();
    clipboard->setText(content);
    QMessageBox::information(this, "Copied", "Text copied to clipboard.");
}


void home::on_toggleTreeButton_clicked()
{
    bool isVisible = ui->titleTreeWidget->isVisible();
    ui->titleTreeWidget->setVisible(!isVisible);
}

