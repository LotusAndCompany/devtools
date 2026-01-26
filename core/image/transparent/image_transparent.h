#ifndef IMAGE_TRANSPARENT_H
#define IMAGE_TRANSPARENT_H

#include "core/exception/invalid_argument_exception.h"
#include "core/image/basic_image_edit_interface.h"
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

#include <cstdint>

#ifdef _TEST_ImageTransparent
namespace Test {
class TestImageTransparent;
}
#endif

/**
 * @brief 画像を透明化するツールのインターフィス
 */
class ImageTransparentInterface : public Tool, public BasicImageEditInterface
{
    Q_OBJECT

public:
    ~ImageTransparentInterface() override = default;

    /**
     * @brief 画像全体の対象の色を透明化する
     * @param targetColor 対象の色
     */
    virtual void addTransparentColor(const QColor &targetColor) = 0;
    /**
     * @brief 選択したピクセルと繋がっている領域を透明化する
     * @param start 開始地点
     * @exception InvalidArgumentException &lt;QPoint&gt; 開始地点が画像の中に無い場合
     */
    virtual void addTransparentPixel(const QPoint &start) = 0;
    /**
     * @brief 元の画像を返す
     * @return 元の画像
     */
    [[nodiscard]] virtual const QImage &original() const = 0;

    /**
     * @brief 計算に使う色空間(RGB, HSV, HSL)
     */
    QColor::Spec colorSpec = QColor::Spec::Rgb;
    /**
     * @brief 色の許容誤差(相対)
     */
    double tolerance = 0.1;
    /**
     * @brief 不透明度(0=透明, 255=不透明)
     */
    uint8_t opacity = 0;

protected:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageTransparentInterface(QObject *parent = nullptr);
};

/**
 * @brief 画像を透明化するツール
 */
class ImageTransparent : public ImageTransparentInterface, private ImageIO
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    ImageTransparent(QObject *parent = nullptr);

    bool save(const QString &path, const char *format = nullptr, int quality = -1) const override
    {
        return ImageIO::save(path, current(), format, quality);
    }

    bool overwriteSave(const QString &path, const char *format = nullptr,
                       int quality = -1) const override
    {
        return ImageIO::overwriteSave(path, current(), format, quality);
    }

    [[nodiscard]] const QFileInfo &fileInfo(unsigned int /*index*/) const override
    {
        return ImageIO::originalFileInfo();
    }

    [[nodiscard]] const QImage &original() const override { return ImageIO::original(); }

    void addTransparentColor(const QColor &targetColor) override;
    void addTransparentPixel(const QPoint &start) override;

protected:
    bool loadImpl(const QString &path) override;
    void resetImpl() override;
    bool updateImpl() override { return !current().isNull(); } // 特に何もしない

private:
    /// 想定される画像フォーマット
    static const QString expectedImageFormat;
    /// 想定されるcolorSpec
    static const QString expectedColorSpec;
    /// 指定された点が無効
    static const QString pixelOutOfImage;

    /// @brief original()のフォーマットを変換して_currentに設定する
    void fetchOriginal();

    /**
     * @brief 画像のフォーマットが不正なら例外を投げる
     * @exception InvalidStateException 画像のフォーマットが `QImage::Format_RGBA8888` 以外の場合
     */
    static void validateImageFormat(QImage::Format format)
    {
        if (format != QImage::Format_RGBA8888) {
            throw InvalidArgumentException<int>(format, expectedImageFormat);
        }
    }

    /// colorDiffSquared*の関数ポインタ型
    using color_comp_function_type = double (*)(const QColor &, const QColor &);

    /**
     * @brief RGB色空間での色の差の2乗を計算する。透明度は考慮しない。
     * @details 立方体上の点同士の距離として計算する
     * @param a 色1
     * @param b 色2
     * @return 色の差の2乗 [0.0-3.0]
     */
    static double colorDiffSquaredRgb(const QColor &a, const QColor &b);

    /**
     * @brief HSV色空間での色の差の2乗を計算する。透明度は考慮しない。
     * @details 円錐上の点同士の距離として計算する
     * @param a 色1
     * @param b 色2
     * @return 色の差の2乗 [0.0-4.0]
     */
    static double colorDiffSquaredHsv(const QColor &a, const QColor &b);

    /**
     * @brief HSL色空間での色の差の2乗を計算する。透明度は考慮しない。
     * @details 双円錐上の点同士の距離として計算する
     * @param a 色1
     * @param b 色2
     * @return 色の差の2乗 [0.0-4.0]
     */
    static double colorDiffSquaredHsl(const QColor &a, const QColor &b);

    /**
     * @brief colorDiffSquared*を返す
     * @param colorSpec 色空間
     * @return 色空間に対応した関数ポインタ
     */
    static color_comp_function_type colorComparisonFunction(QColor::Spec colorSpec);

    /**
     * @brief colorDiffSquaredの最大値を返す
     * @param colorSpec 色空間
     * @return RGB→3.0, HSL, HSV→4.0
     */
    static double maxColorDiffSquared(QColor::Spec colorSpec);

    /**
     * @brief 色空間が正しくない場合に例外を投げる
     * @param colorSpec 色空間
     * @exception InvalidArgumentException `colorSpec` が `QColor::Spec::Rgb`, `QColor::Spec::Hsv`,
     * `QColor::Spec::Hsl` 以外の場合
     */
    static void validateColorSpec(QColor::Spec colorSpec) noexcept(false);

#ifdef _TEST_ImageTransparent
    friend class Test::TestImageTransparent;
#endif
};

#endif // IMAGE_TRANSPARENT_H
