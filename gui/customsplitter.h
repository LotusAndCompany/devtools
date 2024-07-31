#ifndef CUSTOMSPLITTER_H
#define CUSTOMSPLITTER_H

#include <QSplitter>

class CustomSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit CustomSplitter(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int minimumHeightMainContent;
};

#endif // CUSTOMSPLITTER_H
