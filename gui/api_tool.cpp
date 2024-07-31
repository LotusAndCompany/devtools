#include "api_tool.h"
#include "ui_api_tool.h"
#include "customsplitter.h"

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
    setupBodyTab();
    setupAuthenticationTab();

    setupMainSplitter();
    setupResponseView();

}

api_tool::~api_tool()
{
    qDebug() << "Destroying api_tool";
    delete ui;
}

void api_tool::setupAuthenticationTab() {
    QWidget *authTab = new QWidget();
    ui->tabWidget->addTab(authTab, tr("Authentication"));

    QFormLayout *authLayout = new QFormLayout(authTab);
    authTab->setLayout(authLayout);

    // Username field
    QLabel *usernameLabel = new QLabel(tr("Username:"));
    usernameEdit = new QLineEdit();
    authLayout->addRow(usernameLabel, usernameEdit);

    // Password field
    QLabel *passwordLabel = new QLabel(tr("Password:"));
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);  // パスワードを隠す
    authLayout->addRow(passwordLabel, passwordEdit);

    // Apply styling
    QString stylesheet = "QLabel { font: bold 14px; } QLineEdit { border: 1px solid gray; border-radius: 5px; padding: 5px; }";
    authTab->setStyleSheet(stylesheet);
}

void api_tool::setupBodyTab() {
    QWidget *bodyTab = new QWidget();  // Bodyタブ用のウィジェットを生成
    ui->tabWidget->addTab(bodyTab, "Body");  // タブウィジェットにBodyタブを追加

    QVBoxLayout *bodyLayout = new QVBoxLayout();  // QVBoxLayoutを作成
    bodyTab->setLayout(bodyLayout);  // Bodyタブにレイアウトをセット

    bodyTextEdit = new QTextEdit();  // QTextEditを作成
    bodyLayout->addWidget(bodyTextEdit);  // レイアウトにQTextEditを追加
}

void api_tool::setupParametersTable() {
    paramsModel = new QStandardItemModel(10, 3, this);
    paramsModel->setHeaderData(0, Qt::Horizontal, tr("Key"));
    paramsModel->setHeaderData(1, Qt::Horizontal, tr("Value"));
    paramsModel->setHeaderData(2, Qt::Horizontal, tr("Description"));
    ui->tableView->setModel(paramsModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void api_tool::setupMainSplitter() {
    // mainSplitterをカスタムクラスに置き換える
    CustomSplitter *customSplitter = new CustomSplitter(this);
    customSplitter->setOrientation(Qt::Vertical);

    // horizontalContainerを追加
    customSplitter->addWidget(ui->horizontalContainer);

    // タブウィジェットのスタイルシートを設定してタブを左寄せに
    ui->tabWidget->setStyleSheet("QTabBar::tab { width: 100px; } QTabBar { left: 0px; }");

    // 残りのUI要素を追加
    customSplitter->addWidget(ui->tabWidget);

    // スタイルシートの設定
    customSplitter->setStyleSheet(
        "QSplitter::handle { "
        "background-color: #d3d3d3; "
        "border: none; "
        "} "
        "QSplitter::handle:vertical { "
        "height: 2px; "
        "margin: 0px; "
        "padding: 0px; "
        "}"
        );

    // レイアウトにカスタムSplitterを追加
    ui->verticalLayout->addWidget(customSplitter);

    // MainContent（horizontalContainer）の最小高さを設定
    ui->horizontalContainer->setMinimumHeight(100);

    // QSplitterのストレッチファクターを設定
    customSplitter->setStretchFactor(0, 1);  // horizontalContainer
    customSplitter->setStretchFactor(1, 2);  // tabWidget
    customSplitter->setStretchFactor(2, 5);  // listView

    // 旧Splitterを削除
    delete ui->mainSplitter;
    ui->mainSplitter = customSplitter;
}

void api_tool::handleSendButtonClick() {
    qDebug() << "Button clicked";

    requestStartTime = QDateTime::currentMSecsSinceEpoch();

    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

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
        QString bodyText = bodyTextEdit->toPlainText();
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
