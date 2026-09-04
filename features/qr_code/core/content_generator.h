#ifndef CONTENT_GENERATOR_H
#define CONTENT_GENERATOR_H

#include <QString>
#include <QVariantMap>

/**
 * @brief QRコードのコンテンツを生成するクラス
 * @details 各種タイプのQRコードに対応したコンテンツ文字列を生成する
 */
class ContentGenerator
{
public:
    /**
     * @brief 指定されたタイプのQRコードコンテンツを生成する
     * @param type QRコードのタイプ (例: "text", "url", "email"等)
     * @param params コンテンツ生成に必要なパラメータのマップ
     * @return 生成されたQRコードコンテンツ文字列
     */
    static QString generate(const QString &type, const QVariantMap &params);

private:
    /**
     * @brief カレンダーイベント用のコンテンツを生成する
     * @param params イベント情報のパラメータ
     * @return カレンダーイベントコンテンツ
     */
    static QString generateCalendarEvent(const QVariantMap &params);
    /**
     * @brief 連絡先情報用のコンテンツを生成する
     * @param params 連絡先情報のパラメータ
     * @return 連絡先情報コンテンツ
     */
    static QString generateContactInfo(const QVariantMap &params);
    /**
     * @brief メールアドレス用のコンテンツを生成する
     * @param params メールアドレス情報のパラメータ
     * @return メールアドレスコンテンツ
     */
    static QString generateEmail(const QVariantMap &params);
    /**
     * @brief 地理座標用のコンテンツを生成する
     * @param params 地理座標情報のパラメータ
     * @return 地理座標コンテンツ
     */
    static QString generateGeo(const QVariantMap &params);
    /**
     * @brief 電話番号用のコンテンツを生成する
     * @param params 電話番号情報のパラメータ
     * @return 電話番号コンテンツ
     */
    static QString generatePhone(const QVariantMap &params);
    /**
     * @brief SMS用のコンテンツを生成する
     * @param params SMS情報のパラメータ
     * @return SMSコンテンツ
     */
    static QString generateSms(const QVariantMap &params);
    /**
     * @brief プレーンテキスト用のコンテンツを生成する
     * @param params テキスト情報のパラメータ
     * @return プレーンテキストコンテンツ
     */
    static QString generateText(const QVariantMap &params);
    /**
     * @brief URL用のコンテンツを生成する
     * @param params URL情報のパラメータ
     * @return URLコンテンツ
     */
    static QString generateUrl(const QVariantMap &params);
    /**
     * @brief Wi-Fi設定用のコンテンツを生成する
     * @param params Wi-Fi設定情報のパラメータ
     * @return Wi-Fi設定コンテンツ
     */
    static QString generateWifi(const QVariantMap &params);
};

#endif
