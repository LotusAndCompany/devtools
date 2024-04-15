#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

#include <QFileInfo>
#include <QPixmap>
#include "core/image/basic_image_edit_interface.h"
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

/**
 * @brief 画像をリサイズするツールのインターフェイス
 */
class ImageResizeInterface : public Tool, public BasicImageEditInterface
{
    Q_OBJECT

public:
    explicit ImageResizeInterface(QObject *parent = nullptr);
    virtual ~ImageResizeInterface() = default;

    /**
     * @brief 拡大率を設定する。他の設定は上書きされる。
     * @param sx 横方向の拡大率
     * @param sy 横方向の拡大率
     * @exception InvalidArgumentException &lt;double&gt; sx<0またはsy<0の場合
     */
    virtual void setScale(double sx, double sy) noexcept(false) = 0;
    /**
     * @brief 拡大率を設定する。他の設定は上書きされる。
     * @exception InvalidArgumentException &lt;double&gt; s<0の場合
     * @param s 拡大率
     */
    inline void setScale(double s) noexcept(false) { setScale(s, s); }
    /**
     * @brief 横方向の拡大率を設定する。横方向の設定は上書きされる。
     * @exception InvalidArgumentException &lt;double&gt; sy<0の場合
     * @param sx 横方向の拡大率
     */
    virtual void setScaleX(double sx) noexcept(false) = 0;
    /**
     * @brief 縦方向の拡大率を設定する。縦方向の設定は上書きされる。
     * @exception InvalidArgumentException &lt;double&gt; sy<0の場合
     * @param sy 縦方向の拡大率
     */
    virtual void setScaleY(double sy) noexcept(false) = 0;

    /**
     * @brief サイズを設定する。他の設定は上書きされる。
     * @param サイズ
     */
    virtual void setSize(const QSize &size) = 0;
    /**
     * @brief 横幅を設定する。横方向の設定は上書きされる。 keepAspectRatio が`true`の場合は縦方向の設定も上書きされる。
     * @param w 横幅
     * @param keepAspectRatio 縦横比を保存する。
     */
    virtual void setWidth(unsigned int w, bool keepAspectRatio = false) = 0;
    /**
     * @brief 縦幅を設定する。縦方向の設定は上書きされる。 keepAspectRatio が`true`の場合は横方向の設定も上書きされる。
     * @param h 縦幅
     * @param keepAspectRatio 縦横比を保存する。
     */
    virtual void setHeight(unsigned int h, bool keepAspectRatio = false) = 0;

    /**
     * @brief 現在の設定に基づいて計算される画像のサイズ
     * @return 画像のサイズ
     */
    virtual QSize computedSize() const = 0;
    /**
     * @brief 現在の設定に基づいて計算される画像の横方向の拡大率
     * @return 画像の横方向の拡大率
     */
    virtual double computedScaleX() const = 0;
    /**
     * @brief 現在の設定に基づいて計算される画像の縦方向の拡大率
     * @return 画像の縦方向の拡大率
     */
    virtual double computedScaleY() const = 0;
    /**
     * @brief smoothTransformationEnabled を返す
     * @return smoothTransformationEnabled の値
     */
    bool isSmoothTransformationEnabled() const { return smoothTransformationEnabled; }
    /**
     * @brief smoothTransformationEnabled を設定する
     * @param value 設定する値
     */
    virtual void setSmoothTransformationEnabled(bool value = true) = 0;

protected:
    /// Bilinear補完を使うフラグ
    bool smoothTransformationEnabled = false;
};

/**
 * @brief 画像をリサイズするツール
 */
class ImageResize : public ImageResizeInterface, private ImageIO
{
    Q_OBJECT

public:
    explicit ImageResize(QObject *parent = nullptr);

    inline bool save(const QString &path,
                     const char *format = nullptr,
                     int quality = -1) const override
    {
        return ImageIO::save(path, _current, format, quality);
    }
    inline bool overwriteSave(const QString &path, const char *format, int quality) const override
    {
        return ImageIO::overwriteSave(path, _current, format, quality);
    }

    void setScale(double sx, double sy) override;
    void setScaleX(double sx) override;
    void setScaleY(double sy) override;

    void setSize(const QSize &size) override;
    void setWidth(unsigned int w, bool keepAspectRatio = false) override;
    void setHeight(unsigned int h, bool keepAspectRatio = false) override;

    QSize computedSize() const override;
    double computedScaleX() const override;
    double computedScaleY() const override;

    void setSmoothTransformationEnabled(bool value = true) override;

    inline const QFileInfo &fileInfo(unsigned int index = 0) const override
    {
        return ImageIO::originalFileInfo();
    }

protected:
    bool loadImpl(const QString &path) override;
    bool updateImpl() override;
    void resetImpl() override;

private:
    static const QString invalidImageSize;
    static const QString invalidScale;
    static const QString widthOverwritten;
    static const QString heightOverwritten;

    /// サイズ変更系の関数で操作した情報を保存する為の構造体
    struct ResizeHints
    {
        enum class Type {
            /// 最小値
            MIN,
            /// 変更しない
            DEFAULT,
            /// サイズ指定による変更
            SIZE,
            /// 拡大率指定による変更
            SCALE,
            /// 最大値
            MAX,
        };
        /// 値の種類
        Type type = Type::DEFAULT;

        union {
            /// type == SIZE の時のみ有効
            unsigned int size;
            /// type == SCALE の時のみ有効
            double scale;
        };
    };

    /// 横幅のデータと縦幅のデータ
    ResizeHints width, height;

    /**
     * @brief computedSize() の算出に使う
     * @param originalSize 元画像の横もしくは縦のサイズ
     * @param hints width もしくは height
     * @exception InvalidArgumentException &lt;unsigned int&gt; originalSize == 0の場合
     * @return 計算値
     */
    unsigned int computedSizeInternal(unsigned int originalSize, const ResizeHints &hints) const
        noexcept(false);
    /**
     * @brief computedScaleX() computedScaleY() の算出に使う
     * @param originalSize 元画像の横もしくは縦のサイズ
     * @param hints width もしくは height
     * @exception InvalidArgumentException &lt;unsigned int&gt; originalSize == 0の場合
     * @return 計算値
     */
    double computedScaleInternal(unsigned int originalSize, const ResizeHints &hints) const
        noexcept(false);
    /**
     * @brief 縦横比を返す
     * @return 縦横比
     * @exception InvalidStateException 画像サイズが無効な場合
     */
    double aspectRatio() const noexcept(false);
    /**
     * @brief 現在の smoothTransformation を元に変形モードを返す
     * @return 変形モード
     */
    inline Qt::TransformationMode transformationMode() const
    {
        return smoothTransformationEnabled ? Qt::SmoothTransformation : Qt::FastTransformation;
    }
};

#endif // IMAGE_RESIZE_H
