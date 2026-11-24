#include "api_tool.h"

#include "features/framework/gui/design_system.h"
#include "features/http_request/core/http_service.h"

#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>

#include <exception>

api_tool::api_tool(QWidget *parent)
    : QFrame(parent)
    , http_service(new HttpService(this))
    , params_model(new QStandardItemModel(this))
{
    buildUi();
    setupParametersTable();
    setupResponseView();

    connect(send_button, &QPushButton::clicked, this, &api_tool::handleSendButtonClick);
    connect(http_service, &HttpService::finished, this, &api_tool::handleNetworkReplyFinished);
    connect(params_model, &QStandardItemModel::itemChanged, this, &api_tool::updateUrlFromParams);

    qDebug() << "api_tool UI setup complete";
}

api_tool::~api_tool()
{
    qDebug() << "Destroying api_tool";
}

// NOLINTNEXTLINE(readability-function-size)
void api_tool::buildUi()
{
    setWindowTitle(tr("HTTP Request"));
    auto *root_layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(root_layout);

    request_container = DevTools::Ui::createPane(tr("Request"), this);
    DevTools::Ui::configureCompactPane(request_container);
    auto *request_layout = new QVBoxLayout(request_container);
    DevTools::Ui::applyPanelLayout(request_layout);

    auto *request_row = new QHBoxLayout;
    DevTools::Ui::applyInlineLayout(request_row);

    method_combo = new QComboBox(request_container);
    DevTools::Ui::configureComboBox(method_combo);
    method_combo->addItems({tr("GET"), tr("POST"), tr("PUT"), tr("DELETE")});
    request_row->addWidget(method_combo);

    url_edit = new QLineEdit(request_container);
    DevTools::Ui::configureLineEdit(url_edit);
    request_row->addWidget(url_edit, 1);

    send_button = new QPushButton(tr("Send"), request_container);
    DevTools::Ui::configureCompactButton(send_button);
    request_row->addWidget(send_button);
    request_layout->addLayout(request_row);

    root_layout->addWidget(request_container);

    content_layout = new QHBoxLayout;
    DevTools::Ui::applyInlineLayout(content_layout);
    root_layout->addLayout(content_layout, 1);

    tabs_container = DevTools::Ui::createPane(tr("Options"), this);
    auto *tabs_layout = new QVBoxLayout(tabs_container);
    DevTools::Ui::applyPanelLayout(tabs_layout);

    tab_widget = new QTabWidget(tabs_container);

    auto *params_tab = new QWidget(tab_widget);
    auto *params_layout = new QVBoxLayout(params_tab);
    DevTools::Ui::applyPanelLayout(params_layout);
    params_table = new QTableView(params_tab);
    DevTools::Ui::configureTableView(params_table);
    params_layout->addWidget(params_table);
    params_layout->addStretch();
    tab_widget->addTab(params_tab, tr("Parameters"));

    auto *auth_tab = new QWidget(tab_widget);
    auto *auth_layout = new QFormLayout(auth_tab);
    DevTools::Ui::configureFormLayout(auth_layout);
    username_edit = new QLineEdit(auth_tab);
    DevTools::Ui::configureLineEdit(username_edit);
    password_edit = new QLineEdit(auth_tab);
    DevTools::Ui::configureLineEdit(password_edit);
    password_edit->setEchoMode(QLineEdit::Password);
    username_label = new QLabel(tr("Username:"), auth_tab);
    password_label = new QLabel(tr("Password:"), auth_tab);
    auth_layout->addRow(username_label, username_edit);
    auth_layout->addRow(password_label, password_edit);
    tab_widget->addTab(auth_tab, tr("Authentication"));

    auto *body_tab = new QWidget(tab_widget);
    auto *body_layout = new QVBoxLayout(body_tab);
    DevTools::Ui::applyPanelLayout(body_layout);
    body_edit = new QPlainTextEdit(body_tab);
    DevTools::Ui::configureCodeEditor(body_edit);
    body_layout->addWidget(body_edit);
    tab_widget->addTab(body_tab, tr("Body"));

    tabs_layout->addWidget(tab_widget);
    content_layout->addWidget(tabs_container);
}

void api_tool::setupParametersTable()
{
    params_model = new QStandardItemModel(10, 3, this);
    params_model->setHeaderData(0, Qt::Horizontal, tr("Key"));
    params_model->setHeaderData(1, Qt::Horizontal, tr("Value"));
    params_model->setHeaderData(2, Qt::Horizontal, tr("Description"));
    params_table->setModel(params_model);
    params_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    DevTools::Ui::fitTableViewToContents(params_table);
}

void api_tool::handleSendButtonClick()
{
    const QString method = QStringList{QStringLiteral("GET"), QStringLiteral("POST"),
                                       QStringLiteral("PUT"), QStringLiteral("DELETE")}
                               .at(method_combo->currentIndex());
    http_service->send({method, url_edit->text(), username_edit->text(), password_edit->text(),
                        body_edit->toPlainText()});
}

void api_tool::setupResponseView()
{
    response_container = DevTools::Ui::createPane(tr("Response"), this);
    auto *responseLayout = new QVBoxLayout(response_container);
    DevTools::Ui::applyPanelLayout(responseLayout);

    status_label = new QLabel();
    responseLayout->addWidget(status_label);
    response_edit = new QPlainTextEdit(response_container);
    response_edit->setObjectName(QStringLiteral("responseTextEdit"));
    DevTools::Ui::configureDisplayTextControl(response_edit);
    response_edit->setReadOnly(true);
    response_edit->setTextInteractionFlags(Qt::TextSelectableByKeyboard |
                                           Qt::TextSelectableByMouse);
    responseLayout->addWidget(response_edit, 1);

    content_layout->addWidget(response_container);
    DevTools::Ui::configureEqualLayout(content_layout);
}

void api_tool::retranslateUi()
{
    setWindowTitle(tr("HTTP Request"));
    request_container->setTitle(tr("Request"));
    tabs_container->setTitle(tr("Options"));
    response_container->setTitle(tr("Response"));
    send_button->setText(tr("Send"));

    method_combo->setItemText(0, tr("GET"));
    method_combo->setItemText(1, tr("POST"));
    method_combo->setItemText(2, tr("PUT"));
    method_combo->setItemText(3, tr("DELETE"));

    tab_widget->setTabText(0, tr("Parameters"));
    tab_widget->setTabText(1, tr("Authentication"));
    tab_widget->setTabText(2, tr("Body"));
    username_label->setText(tr("Username:"));
    password_label->setText(tr("Password:"));

    params_model->setHeaderData(0, Qt::Horizontal, tr("Key"));
    params_model->setHeaderData(1, Qt::Horizontal, tr("Value"));
    params_model->setHeaderData(2, Qt::Horizontal, tr("Description"));
}

void api_tool::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    } else {
        QFrame::changeEvent(event);
    }
}

QString formatDataSize(qint64 bytes)
{
    const double KB = 1024.0;
    const double MB = 1024.0 * KB;
    const double GB = 1024.0 * MB;
    const auto bytesDouble = static_cast<double>(bytes);

    if (bytesDouble < KB) {
        return QString::number(bytes) + " B";
    } else if (bytesDouble < MB) {
        return QString::number(bytesDouble / KB, 'f', 2) + " KB";
    } else if (bytesDouble < GB) {
        return QString::number(bytesDouble / MB, 'f', 2) + " MB";
    } else {
        return QString::number(bytesDouble / GB, 'f', 2) + " GB";
    }
}

void api_tool::handleNetworkReplyFinished(const HttpResponse &response)
{
    try {
        QString response_text;
        if (response.isSuccess()) {
            response_text = QString::fromUtf8(response.body);
        } else {
            response_text = tr("Error: ") + response.error_message;
        }
        QString const status_info =
            tr("Status: %1 %2 Time: %3ms Size: %4")
                .arg(response.status_code)
                .arg(response.reason.isEmpty() ? tr("Error") : response.reason)
                .arg(response.elapsed_ms)
                .arg(formatDataSize(response.body.size()));
        status_label->setText(status_info);
        response_edit->setPlainText(response_text);
    } catch (const std::exception &e) {
        qCritical() << "Exception caught during response handling:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught during response handling.";
    }
}

void api_tool::updateUrlFromParams()
{
    QList<QPair<QString, QString>> parameters;
    for (int row = 0; row < params_model->rowCount(); ++row) {
        QString const key =
            (params_model->item(row, 0) != nullptr) ? params_model->item(row, 0)->text() : "";
        QString const value =
            (params_model->item(row, 1) != nullptr) ? params_model->item(row, 1)->text() : "";
        parameters.append({key, value});
    }
    url_edit->setText(HttpService::urlWithParameters(url_edit->text(), parameters));
}
