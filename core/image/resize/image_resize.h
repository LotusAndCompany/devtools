#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

#include <QFileInfo>
#include <QPixmap>
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

/*
 * 全体的に見直しが必要。
 * 1. BasicImageEditInterface→save(), load(), commit()もしくはupdated(), preview()
 * 2. BasicImageIO復活→save(), load(), original
 * 3. keepAspectRatioはsetWidth, setHeightに引数として渡す。
 * 4. 
 * 5. リサイズ系関数は以下のようにし、別の関数を呼ぶと以前の内容が消えるようにする。
 *    但し、setWidth→setScaleYなどは許す
 *    setWidth(w, kar)
 *    setHeight(h, kar)
 *    setSize(w, h)
 *    setScale(s)=setScale(s, s)
 *    setScale(sx, sy)
 *    setScaleX(sx)=setScale(sx, 1.0)
 *    setScaleY(sy)=setScale(1.0, sy)
 * 6. ↑の関数ではpixmapは更新せず、update()で反映させる
 * 7. smoothTransformationはupdate()の引数にする
 */

class ImageResizeInterface : public Tool
{
    Q_OBJECT

public:
    explicit ImageResizeInterface(QObject *parent = nullptr);
    virtual ~ImageResizeInterface() = default;

    virtual bool load(const QString &path) = 0;
    virtual bool save(const QString &path, const char *format = nullptr, int quality = -1) const = 0;

    inline void setScaleX(double sx) { setScale(sx, 1.0); }
    inline void setScaleY(double sy) { setScale(1.0, sy); }
    inline void setScale(double s) { setScale(s, s); }
    virtual void setScale(double sx, double sy) = 0;
    virtual void setSize(const QSize &size) = 0;
    virtual void setWidth(unsigned int w, bool keepAspectRatio = false) = 0;
    virtual void setHeight(unsigned int h, bool keepAspectRatio = false) = 0;
};

class ImageResize : public ImageResizeInterface, private ImageIO
{
    Q_OBJECT

public:
    explicit ImageResize(QObject *parent = nullptr);

    /**
     * @brief 画像を読み込む
     * @param path 読み込む画像
     * @return 成功なら`true`
     */
    bool load(const QString &path) override { return ImageIO::load(path); }
    /**
     * @brief 画像を保存する(上書き)
     * @param path 保存先
     * @param format フォーマット。"PNG"など。nullptrならファイル名から推定する。
     * @return 
     */
    bool save(const QString &path, const char *format = nullptr, int quality = -1);

private:
    struct ResizeOperation
    {
        enum class Type {
            MIN,
            SIZE,
            SCALE,
            MAX,
        } type;
        union {
            unsigned int size;
            double scale;
        };
    } width, height;
};

#if 0
class ImageResizeInterface : public Tool
{
    Q_OBJECT

public:
    explicit ImageResizeInterface(QObject *parent = nullptr);
    virtual ~ImageResizeInterface() = default;

    virtual const QPixmap &currentPixmap() const = 0;
    inline QSize currentSize() const { return currentPixmap().size(); }
    virtual QSize originalSize() const = 0;

    virtual bool load(const QString &path) = 0;
    virtual bool save(const QString &path, const char *format = nullptr) const = 0;

    virtual void setScaleX(double sx) = 0;
    virtual void setScaleY(double sy) = 0;
    virtual void setWidth(unsigned int w) = 0;
    virtual void setHeight(unsigned int h) = 0;

    /// アスペクト比を固定するフラグ
    bool keepAspectRatio = false;
    /// bilinear補完を有効にするフラグ
    bool smoothTransformation = false;
};

class ImageResize : public ImageResizeInterface
{
    Q_OBJECT

public:
    explicit ImageResize(QObject *parent = nullptr);

    /**
     * @brief 現在の画像を返す
     * @return 現在の画像
     */
    const QPixmap &currentPixmap() const override { return pixmap; }
    /**
     * @brief 編集前の画像サイズを返す
     * @return 編集前の画像サイズ
     */
    QSize originalSize() const override { return original.size(); }
    /**
     * @brief 画像を読み込む
     * @details original, pixmap は上書きされる
     * @param path 読み込む画像のパス
     * @return 成功したら`true`
     */
    bool load(const QString &path) override;
    /**
     * @brief 現在の画像を指定のパスに保存する
     * @param path 保存先
     * @param format 保存形式。PNGなど。
     * @return 成功したら`true`
     */
    bool save(const QString &path, const char *format = nullptr) const override;

    // NOTE: scale<0の時は入力の時点で弾くようにする
    /**
     * @brief 横方向の拡大率を設定する
     * @details keepAspectRatio が`true`の場合は縦方向にも同じ拡大率が適用される
     * @param sx 拡大率
     * @exception InvalidArgumentException &lt;double&gt; sx<0の場合
     */
    void setScaleX(double sx) noexcept(false) override;
    /**
     * @brief 縦方向の拡大率を設定する
     * @details keepAspectRatio が`true`の場合は横方向にも同じ拡大率が適用される
     * @param sy 拡大率
     * @exception InvalidArgumentException &lt;double&gt; sy<0の場合
     */
    void setScaleY(double sy) noexcept(false) override;
    /**
     * @brief 横幅を設定する
     * @details keepAspectRatio が`true`の場合は縦幅も合わせて変更される
     * @param w 横幅
     */
    void setWidth(unsigned int w) noexcept(false) override;
    /**
     * @brief 縦幅を設定する
     * @details keepAspectRatio が`true`の場合は横幅も合わせて変更される
     * @param h 縦幅
     */
    void setHeight(unsigned int h) noexcept(false) override;

private:
    /// 拡大率の値が無効
    static const QString invalidScaleReason;
    /// サイズが無効
    static const QString invalidSizeReason;

    /// 編集前の画像。 load() でのみ設定される。
    QPixmap original;

    // NOTE: これはロジック側で持たない方が良いかも知れない。GUI向けの側面が強い。
    /// 編集対象の画像。 load() の他、 setScaleX(), setScaleY(), setWidth(), setHeight() で変更される。
    QPixmap pixmap;

    /// 読み込んだファイルの情報。 load() でのみ設定される。
    QFileInfo currentFileInfo;

    /**
     * @brief 現在の smoothTransformation を元に変形モードを返す
     * @return 変形モード
     */
    inline Qt::TransformationMode transformationMode() const
    {
        return smoothTransformation ? Qt::SmoothTransformation : Qt::FastTransformation;
    }
};
#endif

#endif // IMAGE_RESIZE_H
