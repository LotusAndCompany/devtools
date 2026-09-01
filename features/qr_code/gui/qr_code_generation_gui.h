#ifndef QR_CODE_GENERATION_GUI_H
#define QR_CODE_GENERATION_GUI_H

#include "features/framework/gui/gui_tool.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QWidget>

/**
 * @brief QRコード生成ツールのGUIクラス
 * @details 各種タイプのQRコードを生成するための対話的なインターフェイスを提供する
 */
class QRCodeGenerationGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(QRCodeGenerationGUI)

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit QRCodeGenerationGUI(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~QRCodeGenerationGUI() override = default;
    QRCodeGenerationGUI(QRCodeGenerationGUI &&) = delete;
    QRCodeGenerationGUI &operator=(QRCodeGenerationGUI &&) = delete;

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief リサイズイベント処理
     * @param event リサイズイベント
     * @details QRコードの表示を更新する
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /**
     * @brief カテゴリが変更されたときの処理
     * @param index 選択されたカテゴリのインデックス
     */
    void onCategoryChanged(int index);
    /**
     * @brief 生成ボタンがクリックされたときの処理
     */
    void onGenerateClicked();
    /**
     * @brief クリアボタンがクリックされたときの処理
     */
    void onClearClicked();
    /**
     * @brief コピーボタンがクリックされたときの処理
     */
    void onCopyClicked();
    /**
     * @brief 保存ボタンがクリックされたときの処理
     */
    void onSaveClicked();
    /**
     * @brief 生成ボタンの有効/無効状態を更新する
     */
    void updateGenerateButtonState();

private:
    /**
     * @brief QRコードのタイプを定義する列挙体
     */
    enum class QRCodeType : uint8_t { Text, Url, Email, Phone, Sms, Wifi, Contact, Calendar, Geo };

    /**
     * @brief UIを構築する
     */
    void buildUi();
    /**
     * @brief UIテキストを再翻訳する
     */
    void retranslateUi();
    /**
     * @brief パラメータウィジェットを設定する
     */
    void setupParameterWidgets();
    /**
     * @brief すべてのパラメータをクリアする
     */
    void clearAllParameters();
    /**
     * @brief QRコードコンテンツを生成する
     * @return 生成されたコンテンツ文字列
     */
    QString generateQRCodeContent();

    /**
     * @brief テキストタイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createTextWidget();
    /**
     * @brief URLタイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createUrlWidget();
    /**
     * @brief メールタイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createEmailWidget();
    /**
     * @brief 電話番号タイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createPhoneWidget();
    /**
     * @brief SMSタイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createSmsWidget();
    /**
     * @brief Wi-Fiタイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createWifiWidget();
    /**
     * @brief 連絡先タイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createContactWidget();
    /**
     * @brief カレンダータイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createCalendarWidget();
    /**
     * @brief 地理座標タイプ用のウィジェットを作成する
     * @return 作成されたウィジェット
     */
    QWidget *createGeoWidget();

    /// カテゴリ選択用のグループボックス
    QGroupBox *categoryGroupBox{nullptr};
    /// パラメータ入力用のグループボックス
    QGroupBox *parametersGroupBox{nullptr};
    /// 出力表示用のグループボックス
    QGroupBox *outputGroupBox{nullptr};
    /// カテゴリ選択用のコンボボックス
    QComboBox *categoryComboBox{nullptr};
    /// パラメータウィジェットを切り替えるスタックウィジェット
    QStackedWidget *parameterStackedWidget{nullptr};
    /// 生成ボタン
    QPushButton *generateButton{nullptr};
    /// クリアボタン
    QPushButton *clearButton{nullptr};
    /// コピーボタン
    QPushButton *copyButton{nullptr};
    /// 保存ボタン
    QPushButton *saveButton{nullptr};
    /// QRコード画像表示用のラベル
    QLabel *qrCodeLabel{nullptr};
    /// コンテンツプレビュー用のテキストエディット
    QTextEdit *contentPreviewEdit{nullptr};

    /// 現在のQRコードタイプ
    QRCodeType currentType{QRCodeType::Text};

    /// パラメータウィジェットへの参照マップ
    QMap<QString, QWidget *> parameterWidgets;

    /// エラーラベル用のマップ
    QMap<QString, QLabel *> errorLabels;

    /**
     * @brief 現在のタイプのバリデーションを実行する
     * @return バリデーション成功時は `true`
     */
    bool validateCurrentType();
    /**
     * @brief バリデーションエラーを表示する
     * @param fieldKey フィールドキー
     * @param message エラーメッセージ
     */
    void showValidationError(const QString &fieldKey, const QString &message);
    /**
     * @brief すべてのバリデーションエラーをクリアする
     */
    void clearValidationErrors();
    /**
     * @brief 電話番号が有効かどうかを検証する
     * @param phone 検証する電話番号
     * @return 有効な場合は `true`
     */
    static bool isValidPhoneNumber(const QString &phone);

    /// 生成されたQRコード画像を保持
    QImage currentQRImage;

    /**
     * @brief QRコード表示を更新する
     */
    void refreshQRCodeDisplay();
};

#endif // QR_CODE_GENERATION_GUI_H
