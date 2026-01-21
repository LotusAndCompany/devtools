#ifndef BASIC_IMAGE_EDIT_INTERFACE_H
#define BASIC_IMAGE_EDIT_INTERFACE_H

#include <QImage>

#ifdef _TEST_BasicImageEditInterface
namespace Test {
class TestBasicImageEditInterface;
}
#endif

class QFileInfo;

/**
 * @brief 画像編集系ツールの基本的なインターフェイス
 */
class BasicImageEditInterface
{
public:
    virtual ~BasicImageEditInterface() = default;
    /**
     * @brief 画像を読み込む。 reset() や update() は呼び出さない
     * @param path 読み込む画像
     * @return 成功なら`true`
     */
    bool load(const QString &path);
    /**
     * @brief _current を保存する(上書き)
     * @param path 保存先
     * @param format フォーマット。"PNG"など。nullptrならファイル名から推定する。
     * @param quality 画質。-1ならデフォルト値を使用する。
     * @return 成功なら`true`
     */
    virtual bool save(const QString &path, const char *format = nullptr,
                      int quality = -1) const = 0;
    /**
     * @brief _current を保存する。既にファイルが存在する場合は失敗する。
     * @param path 保存先
     * @param format フォーマット。"PNG"など。nullptrならファイル名から推定する。
     * @param quality 画質。-1ならデフォルト値を使用する。
     * @return 成功なら`true`
     */
    virtual bool overwriteSave(const QString &path, const char *format = nullptr,
                               int quality = -1) const = 0;
    /**
     * @brief 現在読み込んでいるファイルの情報を返す
     * @param index ファイルのインデックス(複数のファイルを読み込んでいる場合)
     * @return 現在読み込んでいるファイルの情報
     */
    virtual const QFileInfo &fileInfo(unsigned int index = 0) const = 0;
    /**
     * @brief _current を更新する。 _outdated を`false`に設定する。
     */
    bool update();
    /**
     * @brief これまでの編集を全て取り消し、 _current を load() 直後の状態に戻し、 updated()
     * を呼び出す。GUI向けを想定。
     */
    void reset();
    /**
     * @brief 現在編集中の画像を取得する。GUI向けを想定。
     * @note 正しい状態を返すのは load(), reset(), update() 直後のみ
     */
    inline const QImage &current() const { return _current; }
    /**
     * @brief outdated を`true`に設定する
     */
    inline void setOutdated() { outdated = true; }
    /**
     * @brief update() を呼ぶ必要があるか
     * @return 呼ぶ必要がある時(= updated() で _currentが更新される可能性がある時 )は`true`
     */
    inline bool isOutdated() const { return outdated; }

protected:
    /// 編集中の画像
    QImage _current;

    /**
     * @brief load() の実装
     * @param path 読み込む画像
     * @return 成功なら`true`
     */
    virtual bool loadImpl(const QString &path) = 0;
    /**
     * @brief reset() の実装
     */
    virtual void resetImpl() = 0;
    /**
     * @brief update() の実装
     * @return 成功なら`true`
     */
    virtual bool updateImpl() = 0;

private:
    /// update() を呼ぶ必要があるか
    bool outdated = false;

    /// update() の最後の結果
    bool updateResult = false;
#ifdef _TEST_BasicImageEditInterface
    friend class Test::TestBasicImageEditInterface;
#endif
};

#endif // BASIC_IMAGE_EDIT_INTERFACE_H
