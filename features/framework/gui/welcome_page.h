#ifndef WELCOME_PAGE_H
#define WELCOME_PAGE_H

#include <QWidget>

class QLabel;

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage() override = default;
    WelcomePage(const WelcomePage &) = delete;
    WelcomePage &operator=(const WelcomePage &) = delete;
    WelcomePage(WelcomePage &&) = delete;
    WelcomePage &operator=(WelcomePage &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private:
    QLabel *m_messageLabel{nullptr};

    void retranslateUi();
};

#endif // WELCOME_PAGE_H
