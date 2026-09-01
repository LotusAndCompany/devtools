#include "jwt_decoder_gui.h"

#include "features/jwt_decoder/gui/jwt_highlighters.h"
#include "features/jwt_decoder/gui/jwt_palette.h"

#include <QEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <oclero/qlementine/widgets/Label.hpp>

namespace {
constexpr int DECODE_DEBOUNCE_MS = 180;
constexpr int DEFAULT_WIDTH = 1000;
constexpr int DEFAULT_HEIGHT = 700;
/// .pen の Input Edit の高さ
constexpr int INPUT_HEIGHT = 132;
/// .pen の Header / Signature ペインの高さ (QSplitter の初期値)
constexpr int HEADER_PANE_HEIGHT = 148;
constexpr int PAYLOAD_PANE_HEIGHT = 300;
constexpr int SIGNATURE_PANE_HEIGHT = 100;
/// ペインタイトルのセグメント色ドット
constexpr int DOT_SIZE = 8;
/// 値が無いことを示すプレースホルダ
constexpr const char *EMPTY_PLACEHOLDER = "—";

QLabel *captionLabel(QWidget *parent)
{
    // 見出し・注記は qlementine の Caption ロールでテーマに追従させる
    return new oclero::qlementine::Label(QString(), oclero::qlementine::TextRole::Caption, parent);
}

void setPlainTextIfChanged(QTextBrowser *view, const QString &text)
{
    // 同じ内容でも setPlainText するとスクロール位置が先頭に戻るため、変化時だけ更新する
    if (view->toPlainText() == text) {
        return;
    }

    view->setPlainText(text);
}

QString coloredHtml(const QString &text, const char *color)
{
    return QStringLiteral("<span style=\"color: %1;\">%2</span>")
        .arg(QString::fromLatin1(color), text.toHtmlEscaped());
}
} // namespace

JwtDecoderGUI::JwtDecoderGUI(JwtDecoder *decoder, QWidget *parent)
    : GuiTool(parent), decoder(decoder)
{
    buildUi();

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (decoder != nullptr && decoder->parent() == nullptr) {
        decoder->setParent(this);
    }

    connect(inputEdit, &QTextEdit::textChanged, this, &JwtDecoderGUI::onInputTextChanged);
    connect(decodeTimer, &QTimer::timeout, this, &JwtDecoderGUI::onDecodeTimeout);
}

void JwtDecoderGUI::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    mainLayout->addWidget(buildNotice());
    mainLayout->addWidget(buildInputArea());
    mainLayout->addWidget(buildStructureError());

    headerPane = buildPane(JwtPalette::SEGMENT_HEADER);
    payloadPane = buildPane(JwtPalette::SEGMENT_PAYLOAD);
    signaturePane = buildPane(JwtPalette::SEGMENT_SIGNATURE);

    headerHighlighter = new JsonHighlighter(headerPane.view->document());
    payloadHighlighter = new JsonHighlighter(payloadPane.view->document());

    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(headerPane.widget);
    splitter->addWidget(payloadPane.widget);
    splitter->addWidget(signaturePane.widget);
    // .pen のペイン高さを初期値にする (ペイロードのみ伸縮させる)
    splitter->setSizes({HEADER_PANE_HEIGHT, PAYLOAD_PANE_HEIGHT, SIGNATURE_PANE_HEIGHT});
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    decodeTimer = new QTimer(this);
    decodeTimer->setSingleShot(true);
    decodeTimer->setInterval(DECODE_DEBOUNCE_MS);

    retranslateUi();
}

QWidget *JwtDecoderGUI::buildNotice()
{
    auto *notice = new QFrame(this);
    notice->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 6px;")
                              .arg(QString::fromLatin1(JwtPalette::NOTICE_BACKGROUND)));

    auto *layout = new QHBoxLayout(notice);
    layout->setContentsMargins(12, 10, 12, 10);

    noticeLabel = new QLabel(notice);
    noticeLabel->setWordWrap(true);
    noticeLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                                   .arg(QString::fromLatin1(JwtPalette::NOTICE_TEXT)));
    layout->addWidget(noticeLabel);

    return notice;
}

QWidget *JwtDecoderGUI::buildInputArea()
{
    auto *area = new QWidget(this);

    auto *layout = new QVBoxLayout(area);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto *labelRow = new QHBoxLayout;
    inputLabel = captionLabel(area);
    inputHint = captionLabel(area);
    labelRow->addWidget(inputLabel);
    labelRow->addStretch();
    labelRow->addWidget(inputHint);
    layout->addLayout(labelRow);

    inputEdit = new QTextEdit(area);
    inputEdit->setAcceptRichText(false);
    inputEdit->setFixedHeight(INPUT_HEIGHT);
    // インストール環境に依存しないようシステム標準の等幅フォントを使う
    inputEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(inputEdit);

    segmentHighlighter = new JwtSegmentHighlighter(inputEdit->document());

    return area;
}

QWidget *JwtDecoderGUI::buildStructureError()
{
    structureError = new QFrame(this);
    structureError->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 6px;")
                                      .arg(QString::fromLatin1(JwtPalette::ERROR_BACKGROUND)));
    structureError->hide();

    auto *layout = new QHBoxLayout(structureError);
    layout->setContentsMargins(12, 8, 12, 8);

    structureErrorLabel = new QLabel(structureError);
    structureErrorLabel->setWordWrap(true);
    structureErrorLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                                           .arg(QString::fromLatin1(JwtPalette::ERROR_TEXT)));
    layout->addWidget(structureErrorLabel);

    return structureError;
}

JwtDecoderGUI::Pane JwtDecoderGUI::buildPane(const char *dotColor)
{
    Pane pane;
    pane.widget = new QWidget(this);

    auto *layout = new QVBoxLayout(pane.widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);

    auto *dot = new QLabel(pane.widget);
    dot->setFixedSize(DOT_SIZE, DOT_SIZE);
    dot->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;")
                           .arg(QString::fromLatin1(dotColor))
                           .arg(DOT_SIZE / 2));

    pane.label = captionLabel(pane.widget);
    pane.note = captionLabel(pane.widget);

    titleRow->addWidget(dot);
    titleRow->addWidget(pane.label);
    titleRow->addStretch();
    titleRow->addWidget(pane.note);
    layout->addLayout(titleRow);

    pane.view = new QTextBrowser(pane.widget);
    pane.view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(pane.view);

    return pane;
}

void JwtDecoderGUI::retranslateUi()
{
    setWindowTitle(tr("JWT Decoder"));
    noticeLabel->setText(
        tr("This tool does not verify signatures. Decoding only, fully local — "
           "nothing is sent over the network."));
    inputLabel->setText(tr("Encoded JWT"));
    inputHint->setText(tr("Auto-decodes as you type (180 ms debounce)"));
    inputEdit->setPlaceholderText(tr("Paste a JWT here..."));
    structureErrorLabel->setText(tr("Invalid JWT structure — expected header.payload.signature"));
    headerPane.label->setText(tr("Header"));
    payloadPane.label->setText(tr("Payload"));
    signaturePane.label->setText(tr("Signature"));

    // ペインの注記と本文は状態に依存するため、まとめて再生成する
    updateViews();
}

void JwtDecoderGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void JwtDecoderGUI::onInputTextChanged()
{
    // 入力のたびにタイマーを再スタートしてデバウンスする
    decodeTimer->start();
}

void JwtDecoderGUI::onDecodeTimeout()
{
    if (decoder == nullptr) {
        return;
    }

    // NOTE: シンタックスハイライトの書式変更でも textChanged が飛ぶため、入力が
    //       変わっていなければ何もしない。再デコードするとビューが作り直され、
    //       ユーザーのスクロール位置が失われる。
    const QString token = inputEdit->toPlainText();
    if (token == lastDecodedText) {
        return;
    }
    lastDecodedText = token;

    decoder->setInputText(token);
    decoder->decode();
    updateViews();
}

void JwtDecoderGUI::updateViews()
{
    if (decoder == nullptr) {
        return;
    }

    const bool valid = decoder->isValidStructure();
    const bool empty = inputEdit->toPlainText().trimmed().isEmpty();

    // 未入力のうちは形式エラーを出さない
    structureError->setVisible(!valid && !empty);

    segmentHighlighter->updateState(valid, !decoder->header().ok, !decoder->payload().ok);

    if (!valid) {
        showEmpty(headerPane);
        showEmpty(payloadPane);
        showEmpty(signaturePane);
        return;
    }

    showPart(headerPane, decoder->header());
    showPart(payloadPane, decoder->payload());
    showSignature();
}

void JwtDecoderGUI::showPart(const Pane &pane, const JwtDecoder::DecodedPart &part)
{
    if (part.ok) {
        pane.note->setText(tr("Base64URL decoded · JSON"));
        // 色分けは JsonHighlighter が行うため、本文はプレーンテキストで設定する
        setPlainTextIfChanged(pane.view, part.formattedJson);
        return;
    }

    pane.note->setText(tr("Decode failed"));
    pane.view->setHtml(coloredHtml(part.error, JwtPalette::ERROR_TEXT));
}

void JwtDecoderGUI::showSignature()
{
    const QString signature = decoder->signature();

    signaturePane.note->setText(tr("Not decoded · not verified"));
    signaturePane.view->setHtml(
        coloredHtml(signature.isEmpty() ? QString::fromUtf8(EMPTY_PLACEHOLDER) : signature,
                    JwtPalette::SEGMENT_SIGNATURE) +
        QStringLiteral("<p>%1</p>")
            .arg(
                coloredHtml(tr("Shown as the raw Base64URL string."), JwtPalette::SECONDARY_TEXT)));
}

void JwtDecoderGUI::showEmpty(const Pane &pane)
{
    pane.note->setText(tr("No data"));
    pane.view->setHtml(
        coloredHtml(QString::fromUtf8(EMPTY_PLACEHOLDER), JwtPalette::SECONDARY_TEXT));
}
