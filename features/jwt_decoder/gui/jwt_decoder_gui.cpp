#include "jwt_decoder_gui.h"

#include "features/jwt_decoder/gui/jwt_highlighters.h"
#include "features/jwt_decoder/gui/jwt_palette.h"

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
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
/// .pen の Input Edit の高さ (下限として使い、レイアウトに合わせて伸びられるようにする)
constexpr int INPUT_HEIGHT = 132;
/// .pen の Header / Signature ペインの高さ (QSplitter の初期値)
constexpr int HEADER_PANE_HEIGHT = 148;
constexpr int PAYLOAD_PANE_HEIGHT = 300;
constexpr int SIGNATURE_PANE_HEIGHT = 100;
/// ペインタイトルのセグメント色ドット
constexpr int DOT_SIZE = 8;
/// .pen の gap に合わせた QSplitter のハンドル幅
constexpr int SPLITTER_HANDLE_WIDTH = 10;

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

QString coloredHtml(const QString &text, const QColor &color)
{
    return QStringLiteral("<span style=\"color: %1;\">%2</span>")
        .arg(color.name(QColor::HexRgb), text.toHtmlEscaped());
}

/// バナーの背景色を設定する
void setBannerStyle(QWidget *banner, const QColor &background)
{
    banner->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 6px;")
                              .arg(background.name(QColor::HexRgb)));
}

/// バナー内ラベルの文字色を設定する (背景はバナー側に描かせる)
void setBannerTextStyle(QLabel *label, const QColor &foreground)
{
    label->setStyleSheet(
        QStringLiteral("color: %1; background: transparent;").arg(foreground.name(QColor::HexRgb)));
}

/// ペインタイトルのセグメント色ドットを設定する
void setDotStyle(QLabel *dot, const QColor &color)
{
    dot->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;")
                           .arg(color.name(QColor::HexRgb))
                           .arg(DOT_SIZE / 2));
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
    // ContentsArea が既に 6px の余白を持つため、markdown_preview と同じ 16 / 12 に揃える
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(buildNotice());
    mainLayout->addWidget(buildInputArea());
    mainLayout->addWidget(buildStructureError());

    headerPane = buildPane();
    payloadPane = buildPane();
    signaturePane = buildPane();

    headerHighlighter = new JsonHighlighter(headerPane.view->document());
    payloadHighlighter = new JsonHighlighter(payloadPane.view->document());

    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(headerPane.widget);
    splitter->addWidget(payloadPane.widget);
    splitter->addWidget(signaturePane.widget);
    // .pen のペイン高さを初期値にする (ペイロードのみ伸縮させる)
    splitter->setSizes({HEADER_PANE_HEIGHT, PAYLOAD_PANE_HEIGHT, SIGNATURE_PANE_HEIGHT});
    splitter->setStretchFactor(1, 1);
    splitter->setHandleWidth(SPLITTER_HANDLE_WIDTH);
    mainLayout->addWidget(splitter);

    decodeTimer = new QTimer(this);
    decodeTimer->setSingleShot(true);
    decodeTimer->setInterval(DECODE_DEBOUNCE_MS);

    jwtPalette = JwtPalette::current();
    applyPalette();

    retranslateUi();
}

QWidget *JwtDecoderGUI::buildNotice()
{
    notice = new QFrame(this);

    auto *layout = new QHBoxLayout(notice);
    layout->setContentsMargins(12, 10, 12, 10);

    noticeLabel = new QLabel(notice);
    noticeLabel->setWordWrap(true);
    layout->addWidget(noticeLabel);

    // 配色は applyPalette() でまとめて設定する

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
    // 高さは固定せず、長いトークンでレイアウトに余裕があれば広がるようにする
    inputEdit->setMinimumHeight(INPUT_HEIGHT);
    inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // インストール環境に依存しないようシステム標準の等幅フォントを使う
    inputEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(inputEdit);

    segmentHighlighter = new JwtSegmentHighlighter(inputEdit->document());

    return area;
}

QWidget *JwtDecoderGUI::buildStructureError()
{
    structureError = new QFrame(this);
    structureError->hide();

    auto *layout = new QHBoxLayout(structureError);
    layout->setContentsMargins(12, 8, 12, 8);

    structureErrorLabel = new QLabel(structureError);
    structureErrorLabel->setWordWrap(true);
    layout->addWidget(structureErrorLabel);

    // 配色は applyPalette() でまとめて設定する

    return structureError;
}

JwtDecoderGUI::Pane JwtDecoderGUI::buildPane()
{
    Pane pane;
    pane.widget = new QWidget(this);

    auto *layout = new QVBoxLayout(pane.widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);

    // ドットの色は applyPalette() でセグメントごとに設定する
    pane.dot = new QLabel(pane.widget);
    pane.dot->setFixedSize(DOT_SIZE, DOT_SIZE);

    pane.label = captionLabel(pane.widget);
    pane.note = captionLabel(pane.widget);

    titleRow->addWidget(pane.dot);
    titleRow->addWidget(pane.label);
    titleRow->addStretch();
    titleRow->addWidget(pane.note);
    layout->addLayout(titleRow);

    pane.view = new QTextBrowser(pane.widget);
    pane.view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // 出力ビューは選択・コピーのみ (data_conversion と同じ扱い)
    pane.view->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
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

    // 値が無いことはプレースホルダで示す (本文に書くと全選択コピーに混ざるため)
    headerPane.view->setPlaceholderText(tr("No data"));
    payloadPane.view->setPlaceholderText(tr("No data"));
    signaturePane.view->setPlaceholderText(tr("No data"));

    // ペインの注記と本文は状態に依存するため、まとめて再生成する
    updateViews();
}

void JwtDecoderGUI::updatePalette()
{
    const JwtPalette current = JwtPalette::current();
    if (current == jwtPalette) {
        return;
    }

    jwtPalette = current;
    applyPalette();
}

void JwtDecoderGUI::applyPalette()
{
    setBannerStyle(notice, jwtPalette.noticeBackground);
    setBannerTextStyle(noticeLabel, jwtPalette.noticeText);
    setBannerStyle(structureError, jwtPalette.errorBackground);
    setBannerTextStyle(structureErrorLabel, jwtPalette.errorText);

    setDotStyle(headerPane.dot, jwtPalette.segmentHeader);
    setDotStyle(payloadPane.dot, jwtPalette.segmentPayload);
    setDotStyle(signaturePane.dot, jwtPalette.segmentSignature);

    segmentHighlighter->setPalette(jwtPalette);
    headerHighlighter->setPalette(jwtPalette);
    payloadHighlighter->setPalette(jwtPalette);

    // ペイン本文は HTML に色を埋め込んでいるため作り直す
    updateViews();
}

void JwtDecoderGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    case QEvent::PaletteChange:
    case QEvent::ApplicationPaletteChange:
        // GuiApplication::applyColorScheme() がテーマを切り替えるとパレットが届く。
        // スタイルシートはリセットされないため、ここで色を取り直す。
        updatePalette();
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

void JwtDecoderGUI::showPart(const Pane &pane, const JwtDecoder::DecodedPart &part) const
{
    if (part.ok) {
        pane.note->setText(tr("Base64URL decoded · JSON"));
        // 色分けは JsonHighlighter が行うため、本文はプレーンテキストで設定する
        setPlainTextIfChanged(pane.view, part.formattedJson);
        return;
    }

    pane.note->setText(tr("Decode failed"));
    pane.view->setHtml(coloredHtml(part.error, jwtPalette.errorText));
}

void JwtDecoderGUI::showSignature()
{
    const QString signature = decoder->signature();

    signaturePane.note->setText(tr("Not decoded · not verified"));

    // 2 パートの JWS では署名が無い。プレースホルダに任せる
    if (signature.isEmpty()) {
        signaturePane.view->clear();
        return;
    }

    signaturePane.view->setHtml(
        coloredHtml(signature, jwtPalette.segmentSignature) +
        QStringLiteral("<p>%1</p>")
            .arg(coloredHtml(tr("Shown as the raw Base64URL string."), jwtPalette.secondaryText)));
}

void JwtDecoderGUI::showEmpty(const Pane &pane)
{
    pane.note->setText(tr("No data"));
    pane.view->clear();
}
