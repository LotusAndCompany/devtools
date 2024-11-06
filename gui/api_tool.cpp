#include "api_tool.h"
#include "ui_api_tool.h"
#include <QVBoxLayout>
#include <QAuthenticator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QNetworkRequest>
#include <QFormLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <QStringListModel>

api_tool::api_tool(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::api_tool)
    , networkManager(new QNetworkAccessManager(this))
    , paramsModel(new QStandardItemModel(this)) // Model initialization
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);

    connect(ui->sendButton, &QPushButton::clicked, this, &api_tool::handleSendButtonClick);
    connect(networkManager, &QNetworkAccessManager::finished, this, &api_tool::handleNetworkReplyFinished);
    connect(paramsModel, &QStandardItemModel::itemChanged, this, &api_tool::updateUrlFromParams);

    qDebug() << "api_tool UI setup complete";

    setupParametersTable();
    setupResponseView();

}

api_tool::~api_tool()
{
    qDebug() << "Destroying api_tool";
    delete ui;
}

void api_tool::setupParametersTable() {
    paramsModel = new QStandardItemModel(10, 3, this);
    paramsModel->setHeaderData(0, Qt::Horizontal, tr("Key"));
    paramsModel->setHeaderData(1, Qt::Horizontal, tr("Value"));
    paramsModel->setHeaderData(2, Qt::Horizontal, tr("Description"));
    ui->tableView->setModel(paramsModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void api_tool::handleSendButtonClick() {
    qDebug() << "Button clicked";

    requestStartTime = QDateTime::currentMSecsSinceEpoch();

    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    QString selectedMethod = ui->comboBox->currentText();
    QString url = ui->textEdit->toPlainText();
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!username.isEmpty() && !password.isEmpty()) {
        QString credentials = username + ":" + password;
        QByteArray data = credentials.toLocal8Bit().toBase64();
        request.setRawHeader("Authorization", "Basic " + data);
    }

    QByteArray payload;
    // Bodyテキストエリアからデータを取得してpayloadに設定
    if (selectedMethod == "POST" || selectedMethod == "PUT") {
        QString bodyText = ui->bodyTextEdit->toPlainText();
        payload = bodyText.toUtf8();
    }

    if (selectedMethod == "GET") {
        networkManager->get(request);
    } else if (selectedMethod == "POST") {
        networkManager->post(request, payload);
    } else if (selectedMethod == "PUT") {
        networkManager->put(request, payload);
    } else if (selectedMethod == "DELETE") {
        networkManager->deleteResource(request);
    }
}

void api_tool::setupResponseView() {
    QWidget *responseWidget = new QWidget();
    QVBoxLayout *responseLayout = new QVBoxLayout(responseWidget);

    statusLabel = new QLabel();  // ステータスラベルの作成
    responseLayout->addWidget(statusLabel);  // レイアウトにステータスラベルを追加

    QListView *responseListView = new QListView();  // レスポンス表示用のリストビュー作成
    responseModel = new QStringListModel(this);  // モデルのインスタンス作成
    responseListView->setModel(responseModel);  // リストビューにモデルをセット
    responseLayout->addWidget(responseListView);  // レイアウトにリストビューを追加

    responseWidget->setLayout(responseLayout);  // ウィジェットにレイアウトをセット
    ui->mainSplitter->addWidget(responseWidget);  // mainSplitterにレスポンスウィジェットを追加
}

QString formatDataSize(qint64 bytes) {
    const double KB = 1024.0;
    const double MB = 1024.0 * KB;
    const double GB = 1024.0 * MB;

    if (bytes < KB) {
        return QString::number(bytes) + " bytes";
    } else if (bytes < MB) {
        return QString::number(bytes / KB, 'f', 2) + " KB";
    } else if (bytes < GB) {
        return QString::number(bytes / MB, 'f', 2) + " MB";
    } else {
        return QString::number(bytes / GB, 'f', 2) + " GB";
    }
}


void api_tool::handleNetworkReplyFinished(QNetworkReply *reply) {
    try {
        if (!reply) {
            qCritical() << "Received a null reply object.";
            return;
        }

        qint64 responseTime = QDateTime::currentMSecsSinceEpoch() - requestStartTime;
        qDebug() << "Handling network reply, elapsed time:" << responseTime << "ms";

        QByteArray responseData = reply->readAll();
        qint64 dataSize = responseData.size();
        qDebug() << "Response size:" << dataSize << "bytes";

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString statusText = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << "Status code:" << statusCode << (statusText.isEmpty() ? "" : statusText);

        QString responseText;
        if (reply->error() == QNetworkReply::NoError) {
            responseText = QString::fromUtf8(responseData);
        } else {
            responseText = tr("Error: ") + reply->errorString();
        }
        QString formattedSize = formatDataSize(dataSize);

        QString statusInfo = tr("Status: %1 %2 Time: %3ms Size: %4")
                                 .arg(statusCode)
                                 .arg(statusText.isEmpty() ? "Error" : statusText)
                                 .arg(responseTime)
                                 .arg(formattedSize);
        statusLabel->setText(statusInfo);

        // Updating the view
        QStringList responseList;
        responseList << responseText; // Include error message if error occurred
        responseModel->setStringList(responseList);

    } catch (const std::exception& e) {
        qCritical() << "Exception caught during response handling:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught during response handling.";
    }

    // Safe deletion
    reply->deleteLater();
}



void api_tool::updateUrlFromParams()
{
    QString baseUrl = ui->textEdit->toPlainText().split('?').at(0);
    QString queryString;

    for (int row = 0; row < paramsModel->rowCount(); ++row) {
        QString key = paramsModel->item(row, 0) ? paramsModel->item(row, 0)->text() : "";
        QString value = paramsModel->item(row, 1) ? paramsModel->item(row, 1)->text() : "";

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

    ui->textEdit->setPlainText(baseUrl);
}
