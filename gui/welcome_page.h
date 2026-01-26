#ifndef WELCOME_PAGE_H
#define WELCOME_PAGE_H

#include <QWidget>

namespace Ui {
class WelcomePage;
}

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage() override;
    WelcomePage(const WelcomePage &) = delete;
    WelcomePage &operator=(const WelcomePage &) = delete;
    WelcomePage(WelcomePage &&) = delete;
    WelcomePage &operator=(WelcomePage &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::WelcomePage *const ui;
};

#endif // WELCOME_PAGE_H
