#ifndef IMAGE_ROTATION_H
#define IMAGE_ROTATION_H

#include <QFileInfo>
#include <QPixmap>
#include <QTransform>
#include "core/image/basic_image_edit_interface.h"
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

#ifdef _TEST_ImageRotation
namespace Test {
class TestImageRotation;
}
#endif

/**
 * @brief 画像を回転・反転させるツールのインターフェイス
 */
class ImageRotationInterface : public Tool, public BasicImageEditInterface
{
    Q_OBJECT

public:
    virtual ~ImageRotationInterface() = default;

    /**
     * @brief 回転させる
     * @param rad 回転する角度(ラジアン)
     */
    inline void rotate(double rad) { rotateDegrees(rad * 180.0 / M_PI); }
    /**
     * @brief 回転させる
     * @param deg 回転する角度(度)
     */
    virtual void rotateDegrees(double deg) = 0;
    /**
     * @brief 左右反転する
     */
    virtual void flipHorizontally() = 0;
    /**
     * @brief 上下反転させる
     */
    virtual void flipVertically() = 0;

    /**
     * @brief 元の画像を返す
     * @return 元の画像 
     */
    virtual const QImage &original() const = 0;

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
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageRotationInterface(QObject *parent);

    /// Bilinear補完有効化フラグ
    bool smoothTransformationEnabled = false;
};

/**
 * @brief 画像を回転するツール
 */
class ImageRotation : public ImageRotationInterface, private ImageIO
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageRotation(QObject *parent = nullptr);

    inline bool save(const QString &path,
                     const char *format = nullptr,
                     int quality = -1) const override
    {
        return ImageIO::save(path, current(), format, quality);
    }

    inline bool overwriteSave(const QString &path,
                              const char *format = nullptr,
                              int quality = -1) const override
    {
        return ImageIO::overwriteSave(path, current(), format, quality);
    }

    inline const QFileInfo &fileInfo(unsigned int index) const override
    {
        return ImageIO::originalFileInfo();
    }

    inline const QImage &original() const override { return ImageIO::original(); }

    void rotateDegrees(double deg) override;
    void flipHorizontally() override;
    void flipVertically() override;

    void setSmoothTransformationEnabled(bool value = true) override;

protected:
    bool loadImpl(const QString &path) override;
    void resetImpl() override;
    bool updateImpl() override;

private:
    /// 変形を表す行列
    QTransform transform;

    /**
     * @brief 現在の smoothTransformation を元に変形モードを返す
     * @return 変形モード
     */
    inline Qt::TransformationMode transformationMode() const
    {
        return smoothTransformationEnabled ? Qt::SmoothTransformation : Qt::FastTransformation;
    }

#ifdef _TEST_ImageRotation
    friend class Test::TestImageRotation;
#endif
};

#endif // IMAGE_ROTATION_H
