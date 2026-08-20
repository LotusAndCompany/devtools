#include "api_tool.h"

#include "features/framework/gui/design_system.h"

#include <QAuthenticator>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>

api_tool::api_tool(QWidget *parent)
    : QFrame(parent)
    , network_manager(new QNetworkAccessManager(this))
    , params_model(new QStandardItemModel(this))
{
    buildUi();
    setupParametersTable();
    setupResponseView();

    connect(send_button, &QPushButton::clicked, this, &api_tool::handleSendButtonClick);
    connect(network_manager, &QNetworkAccessManager::finished, this,
            &api_tool::handleNetworkReplyFinished);
    connect(params_model, &QStandardItemModel::itemChanged, this, &api_tool::updateUrlFromParams);

    qDebug() << "api_tool UI setup complete";
}

api_tool::~api_tool()
{
    qDebug() << "Destroying api_tool";
}

void api_tool::buildUi()
{
    setWindowTitle(tr("API Tool"));
    auto *root_layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(root_layout);

    auto *request_container = new QGroupBox(this);
    request_container->setTitle(tr("Request"));
    auto *request_layout = new QVBoxLayout(request_container);
    DevTools::Ui::applyPanelLayout(request_layout);

    auto *request_row = new QHBoxLayout;
    DevTools::Ui::applyInlineLayout(request_row);

    method_combo = new QComboBox(request_container);
    method_combo->addItem(tr("GET"), QStringLiteral("GET"));
    method_combo->addItem(tr("POST"), QStringLiteral("POST"));
    method_combo->addItem(tr("PUT"), QStringLiteral("PUT"));
    method_combo->addItem(tr("DELETE"), QStringLiteral("DELETE"));
    request_row->addWidget(method_combo);

    url_edit = new QLineEdit(request_container);
    request_row->addWidget(url_edit);

    send_button = new QPushButton(tr("Send"), request_container);
    DevTools::Ui::configureCompactButton(send_button);
    request_row->addWidget(send_button);
    request_layout->addLayout(request_row);

    root_layout->addWidget(request_container);

    main_splitter = new QSplitter(Qt::Horizontal, this);
    root_layout->addWidget(main_splitter, 1);

    auto *tabs_container = new QGroupBox(main_splitter);
    tabs_container->setTitle(tr("Options"));
    auto *tabs_layout = new QVBoxLayout(tabs_container);
    DevTools::Ui::applyPanelLayout(tabs_layout);

    auto *tab_widget = new QTabWidget(tabs_container);

    auto *params_tab = new QWidget(tab_widget);
    auto *params_layout = new QVBoxLayout(params_tab);
    DevTools::Ui::applyPanelLayout(params_layout);
    params_table = new QTableView(params_tab);
    params_layout->addWidget(params_table);
    tab_widget->addTab(params_tab, tr("Parameters"));

    auto *auth_tab = new QWidget(tab_widget);
    auto *auth_layout = new QFormLayout(auth_tab);
    DevTools::Ui::configureFormLayout(auth_layout);
    username_edit = new QLineEdit(auth_tab);
    password_edit = new QLineEdit(auth_tab);
    password_edit->setEchoMode(QLineEdit::Password);
    auth_layout->addRow(tr("Username:"), username_edit);
    auth_layout->addRow(tr("Password:"), password_edit);
    tab_widget->addTab(auth_tab, tr("Authentication"));

    auto *body_tab = new QWidget(tab_widget);
    auto *body_layout = new QVBoxLayout(body_tab);
    DevTools::Ui::applyPanelLayout(body_layout);
    body_edit = new QPlainTextEdit(body_tab);
    DevTools::Ui::configureTextControl(body_edit);
    body_layout->addWidget(body_edit);
    tab_widget->addTab(body_tab, tr("Body"));

    tabs_layout->addWidget(tab_widget);
    main_splitter->addWidget(tabs_container);
}

void api_tool::setupParametersTable()
{
    params_model = new QStandardItemModel(10, 3, this);
    params_model->setHeaderData(0, Qt::Horizontal, tr("Key"));
    params_model->setHeaderData(1, Qt::Horizontal, tr("Value"));
    params_model->setHeaderData(2, Qt::Horizontal, tr("Description"));
    params_table->setModel(params_model);
    params_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void api_tool::handleSendButtonClick()
{
    qDebug() << "Button clicked";

    request_start_time = QDateTime::currentMSecsSinceEpoch();

    QString const username = username_edit->text();
    QString const password = password_edit->text();

    QString const selectedMethod = method_combo->currentData().toString();
    QString const url = url_edit->text();
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!username.isEmpty() && !password.isEmpty()) {
        QString const credentials = username + ":" + password;
        QByteArray const data = credentials.toLocal8Bit().toBase64();
        request.setRawHeader("Authorization", "Basic " + data);
    }

    QByteArray payload;
    if (selectedMethod == "POST" || selectedMethod == "PUT") {
        QString const bodyText = body_edit->toPlainText();
        payload = bodyText.toUtf8();
    }

    if (selectedMethod == "GET") {
        network_manager->get(request);
    } else if (selectedMethod == "POST") {
        network_manager->post(request, payload);
    } else if (selectedMethod == "PUT") {
        network_manager->put(request, payload);
    } else if (selectedMethod == "DELETE") {
        network_manager->deleteResource(request);
    }
}

void api_tool::setupResponseView()
{
    auto *responseWidget = new QGroupBox(main_splitter);
    responseWidget->setTitle(tr("Response"));
    auto *responseLayout = new QVBoxLayout(responseWidget);
    DevTools::Ui::applyPanelLayout(responseLayout);

    status_label = new QLabel();
    responseLayout->addWidget(status_label);
    auto *responseListView = new QListView();
    response_model = new QStringListModel(this);
    responseListView->setModel(response_model);
    responseLayout->addWidget(responseListView);

    main_splitter->addWidget(responseWidget);
    main_splitter->setHandleWidth(DevTools::Ui::Metrics::SPLITTER_HANDLE_WIDTH);
    main_splitter->setStretchFactor(0, DevTools::Ui::Metrics::SIDE_PANEL_STRETCH);
    main_splitter->setStretchFactor(1, DevTools::Ui::Metrics::MAIN_PANEL_STRETCH);
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

void api_tool::handleNetworkReplyFinished(QNetworkReply *reply)
{
    try {
        if (reply == nullptr) {
            qCritical() << "Received a null reply object.";
            return;
        }

        qint64 const responseTime = QDateTime::currentMSecsSinceEpoch() - request_start_time;
        qDebug() << "Handling network reply, elapsed time:" << responseTime << "ms";

        QByteArray const responseData = reply->readAll();
        qint64 const dataSize = responseData.size();
        qDebug() << "Response size:" << dataSize << "bytes";

        int const statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString const statusText =
            reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << "Status code:" << statusCode << (statusText.isEmpty() ? "" : statusText);

        QString responseText;
        if (reply->error() == QNetworkReply::NoError) {
            responseText = QString::fromUtf8(responseData);
        } else {
            responseText = tr("Error: ") + reply->errorString();
        }
        QString const formattedSize = formatDataSize(dataSize);

        QString const statusInfo = tr("Status: %1 %2 Time: %3ms Size: %4")
                                       .arg(statusCode)
                                       .arg(statusText.isEmpty() ? "Error" : statusText)
                                       .arg(responseTime)
                                       .arg(formattedSize);
        status_label->setText(statusInfo);

        QStringList responseList;
        responseList << responseText;
        response_model->setStringList(responseList);

    } catch (const std::exception &e) {
        qCritical() << "Exception caught during response handling:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught during response handling.";
    }

    reply->deleteLater();
}

void api_tool::updateUrlFromParams()
{
    QString baseUrl = url_edit->text().split('?').at(0);
    QString queryString;

    for (int row = 0; row < params_model->rowCount(); ++row) {
        QString const key =
            (params_model->item(row, 0) != nullptr) ? params_model->item(row, 0)->text() : "";
        QString const value =
            (params_model->item(row, 1) != nullptr) ? params_model->item(row, 1)->text() : "";

        if (!key.isEmpty()) {
            if (!queryString.isEmpty()) {
                queryString.append('&');
            }
            queryString.append(QUrl::toPercentEncoding(key) + '=' + QUrl::toPercentEncoding(value));
        }
    }

    if (!queryString.isEmpty()) {
        baseUrl.append('?').append(queryString);
    }

    url_edit->setText(baseUrl);
}
