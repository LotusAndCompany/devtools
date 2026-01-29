#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

#include "core/image/basic_image_edit_interface.h"
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

#include <QFileInfo>
#include <QPixmap>

#ifdef _TEST_ImageResize
namespace Test {
class TestImageResize;
}
#endif

/**
 * @brief 画像をリサイズするツールのインターフェイス
 */
class ImageResizeInterface : public Tool, public BasicImageEditInterface
{
    Q_OBJECT

public:
    ImageResizeInterface(const ImageResizeInterface &) = delete;
    ImageResizeInterface(ImageResizeInterface &&) = delete;
    ImageResizeInterface &operator=(const ImageResizeInterface &) = delete;
    ImageResizeInterface &operator=(ImageResizeInterface &&) = delete;
    ~ImageResizeInterface() override = default;

    /**
     * @brief 拡大率を設定する。他の設定は上書きされる。
     * @param sx 横方向の拡大率
     * @param sy 横方向の拡大率
     * @exception InvalidArgumentException &lt;double&gt; sx<0またはsy<0の場合
     */
    virtual void setScale(double sx, double sy) noexcept(false) = 0;
    /**
     * @brief 拡大率を設定する。他の設定は上書きされる。
     * @param s 拡大率
     * @exception InvalidArgumentException &lt;double&gt; s<0の場合
     */
    void setScale(double s) noexcept(false) { setScale(s, s); }
    /**
     * @brief 横方向の拡大率を設定する。横方向の設定は上書きされる。
     * @param sx 横方向の拡大率
     * @exception InvalidArgumentException &lt;double&gt; sy<0の場合
     */
    virtual void setScaleX(double sx) noexcept(false) = 0;
    /**
     * @brief 縦方向の拡大率を設定する。縦方向の設定は上書きされる。
     * @param sy 縦方向の拡大率
     * @exception InvalidArgumentException &lt;double&gt; sy<0の場合
     */
    virtual void setScaleY(double sy) noexcept(false) = 0;

    /**
     * @brief サイズを設定する。他の設定は上書きされる。
     * @param size サイズ
     * @exception InvalidArgumentException &lt;QSize&gt; サイズが無効な場合
     */
    virtual void setSize(const QSize &size) noexcept(false) = 0;
    /**
     * @brief 横幅を設定する。横方向の設定は上書きされる。 keepAspectRatio
     * が`true`の場合は縦方向の設定も上書きされる。
     * @param w 横幅
     * @param keepAspectRatio 縦横比を保存する。
     */
    virtual void setWidth(unsigned int w, bool keepAspectRatio = false) = 0;
    /**
     * @brief 縦幅を設定する。縦方向の設定は上書きされる。 keepAspectRatio
     * が`true`の場合は横方向の設定も上書きされる。
     * @param h 縦幅
     * @param keepAspectRatio 縦横比を保存する。
     */
    virtual void setHeight(unsigned int h, bool keepAspectRatio = false) = 0;

    /**
     * @brief 現在の設定に基づいて計算される画像のサイズ
     * @return 画像のサイズ
     */
    [[nodiscard]] virtual QSize computedSize() const = 0;
    /**
     * @brief 現在の設定に基づいて計算される画像の横方向の拡大率
     * @return 画像の横方向の拡大率
     * @exception InvalidStateException 画像が空の場合
     */
    [[nodiscard]] virtual double computedScaleX() const = 0;
    /**
     * @brief 現在の設定に基づいて計算される画像の縦方向の拡大率
     * @return 画像の縦方向の拡大率
     * @exception InvalidStateException 画像が空の場合
     */
    [[nodiscard]] virtual double computedScaleY() const = 0;
    /**
     * @brief smoothTransformationEnabled を返す
     * @return smoothTransformationEnabled の値
     */
    [[nodiscard]] bool isSmoothTransformationEnabled() const { return smoothTransformationEnabled; }
    /**
     * @brief smoothTransformationEnabled を設定する
     * @param value 設定する値
     */
    virtual void setSmoothTransformationEnabled(bool value = true) = 0;
    /**
     * @brief 元の画像を返す
     * @return 元の画像
     */
    [[nodiscard]] virtual const QImage &original() const = 0;

protected:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageResizeInterface(QObject *parent = nullptr);

    /// Bilinear補完有効化フラグ
    bool smoothTransformationEnabled =
        false; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};

/**
 * @brief 画像をリサイズするツール
 */
class ImageResize : public ImageResizeInterface, private ImageIO
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageResize(QObject *parent = nullptr);

    bool save(const QString &path, const char *format = nullptr, int quality = -1) const override
    {
        return ImageIO::save(path, _current, format, quality);
    }
    bool overwriteSave(const QString &path, const char *format = nullptr,
                       int quality = -1) const override
    {
        return ImageIO::overwriteSave(path, _current, format, quality);
    }

    void setScale(double s) { ImageResizeInterface::setScale(s); }
    void setScale(double sx, double sy) override;
    void setScaleX(double sx) override;
    void setScaleY(double sy) override;

    void setSize(const QSize &size) override;
    void setWidth(unsigned int w, bool keepAspectRatio = false) override;
    void setHeight(unsigned int h, bool keepAspectRatio = false) override;

    [[nodiscard]] QSize computedSize() const override;
    [[nodiscard]] double computedScaleX() const override;
    [[nodiscard]] double computedScaleY() const override;

    void setSmoothTransformationEnabled(bool value = true) override;

    [[nodiscard]] const QFileInfo &fileInfo(unsigned int /*index*/ = 0) const override
    {
        return ImageIO::originalFileInfo();
    }

    [[nodiscard]] const QImage &original() const override { return ImageIO::original(); }

protected:
    bool loadImpl(const QString &path) override;
    bool updateImpl() override;
    void resetImpl() override;

private:
    /// サイズ指定が不正
    static const QString invalidSize;
    /// 画像サイズが不正
    static const QString invalidImageSize;
    /// 拡大率指定が不正
    static const QString invalidScale;
    /// 横幅の指定を上書きする
    static const QString widthOverwritten;
    /// 縦幅の指定を上書きする
    static const QString heightOverwritten;

    /// サイズ変更系の関数で操作した情報を保存する為の構造体
    struct ResizeHints
    {
        /// 指定なし/サイズ指定/拡大率指定を表す列挙体
        enum class Type : uint8_t {
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
        ;
        /// 値の種類
        Type type = Type::DEFAULT;

        union {
            /// サイズ指定。type == SIZE の時のみ有効
            unsigned int size{};
            /// 拡大率指定。type == SCALE の時のみ有効
            double scale;
        };
    };

    /// 横幅のデータ
    ResizeHints width;
    /// 縦幅のデータ
    ResizeHints height;

    /**
     * @brief computedSize() の算出に使う
     * @param originalSize 元画像の横もしくは縦のサイズ
     * @param hints width もしくは height
     * @exception InvalidArgumentException &lt;unsigned int&gt; originalSize == 0の場合
     * @return 計算値
     */
    static unsigned int computedSizeInternal(unsigned int originalSize, const ResizeHints &hints);
    /**
     * @brief computedScaleX() computedScaleY() の算出に使う
     * @param originalSize 元画像の横もしくは縦のサイズ
     * @param hints width もしくは height
     * @exception InvalidArgumentException &lt;unsigned int&gt; originalSize == 0の場合
     * @return 計算値
     */
    static double computedScaleInternal(unsigned int originalSize,
                                        const ResizeHints &hints) noexcept(false);
    /**
     * @brief 縦横比を返す
     * @return 縦横比
     * @exception InvalidStateException 画像サイズが無効な場合
     */
    [[nodiscard]] double aspectRatio() const noexcept(false);
    /**
     * @brief 現在の smoothTransformation を元に変形モードを返す
     * @return 変形モード
     */
    [[nodiscard]] Qt::TransformationMode transformationMode() const
    {
        return smoothTransformationEnabled ? Qt::SmoothTransformation : Qt::FastTransformation;
    }

#ifdef _TEST_ImageResize
    friend class Test::TestImageResize;
#endif
};

#endif // IMAGE_RESIZE_H
