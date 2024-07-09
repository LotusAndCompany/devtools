#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>

namespace Ui {
class ItemWidget;
}

class ItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ItemWidget(const QString &title, QWidget *parent = nullptr);
    ~ItemWidget();

    QString getTitle() const;

signals:
    void listCopyButtonClicked(const QString &title);

private slots:
    // void on_copyButton_clicked();

    void on_listCopyButton_clicked();

private:
    Ui::ItemWidget *ui;
    QString title;
};

#endif // ITEMWIDGET_H
