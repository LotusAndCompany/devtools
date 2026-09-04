#ifndef JWT_DECODER_GUI_H
#define JWT_DECODER_GUI_H

#include "features/framework/gui/gui_tool.h"
#include "features/jwt_decoder/core/jwt_decoder.h"
#include "features/jwt_decoder/gui/jwt_palette.h"

#include <QString>

class JsonHighlighter;
class JwtSegmentHighlighter;

class QEvent;
class QLabel;
class QTextBrowser;
class QTextEdit;
class QTimer;
class QWidget;

/**
 * @brief JWT デコーダーの GUI
 * @details 上部に入力欄、下部に Header / Payload / Signature の 3 ペインを縦に並べる。@n
 *          `.ui` は使わずコンストラクタでウィジェットを構築する。@n
 *          署名を検証しない旨の注意バナーを常時表示する。
 */
class JwtDecoderGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(JwtDecoderGUI)

public:
    explicit JwtDecoderGUI(JwtDecoder *decoder, QWidget *parent = nullptr);
    ~JwtDecoderGUI() override = default;
    JwtDecoderGUI(JwtDecoderGUI &&) = delete;
    JwtDecoderGUI &operator=(JwtDecoderGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onInputTextChanged();
    void onDecodeTimeout();

private:
    /// 1 つの出力ペインを構成するウィジェット群
    struct Pane
    {
        QWidget *widget = nullptr;
        QLabel *dot = nullptr;
        QLabel *label = nullptr;
        QLabel *note = nullptr;
        QTextBrowser *view = nullptr;
    };

    void buildUi();
    QWidget *buildNotice();
    QWidget *buildInputArea();
    QWidget *buildStructureError();
    Pane buildPane();

    void retranslateUi();

    /// 現在のテーマから色を取り直し、変化していれば適用する
    void updatePalette();
    /// 保持しているパレットをスタイルシート・ハイライタ・ペイン本文に反映する
    void applyPalette();

    void updateViews();
    void showPart(const Pane &pane, const JwtDecoder::DecodedPart &part) const;
    void showSignature();
    static void showEmpty(const Pane &pane);

    JwtDecoder *const decoder;

    /// テーマ由来の意味色 (`QWidget::palette()` と紛れないよう接頭辞を付ける)
    JwtPalette jwtPalette;

    QWidget *notice = nullptr;             ///< 署名非検証の注意バナー
    QLabel *noticeLabel = nullptr;         ///< 署名非検証の注意文
    QLabel *inputLabel = nullptr;          ///< "Encoded JWT"
    QLabel *inputHint = nullptr;           ///< デバウンスの注記
    QTextEdit *inputEdit = nullptr;        ///< JWT 入力欄
    QWidget *structureError = nullptr;     ///< 形式不正バナー (通常は非表示)
    QLabel *structureErrorLabel = nullptr; ///< 形式不正メッセージ

    Pane headerPane;
    Pane payloadPane;
    Pane signaturePane;

    JwtSegmentHighlighter *segmentHighlighter = nullptr; ///< 入力欄のセグメント色分け
    JsonHighlighter *headerHighlighter = nullptr;        ///< ヘッダー JSON の色分け
    JsonHighlighter *payloadHighlighter = nullptr;       ///< ペイロード JSON の色分け

    QTimer *decodeTimer = nullptr; ///< single-shot デバウンス

    /// 直近にデコードした入力 (再デコードの抑止に使う)
    QString lastDecodedText;
};

#endif // JWT_DECODER_GUI_H
