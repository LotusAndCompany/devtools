#ifndef IMAGE_DIVISION_H
#define IMAGE_DIVISION_H

#include <QFileInfo>
#include <QPixmap>
#include "core/image/basic_image_edit_interface.h"
#include "core/image/basic_image_io.h"
#include "core/tool/tool.h"

#ifdef _TEST_ImageDivision
namespace Test {
class TestImageDivision;
}
#endif

/**
 * @brief 画像を分割するツールのインターフェイス
 */
class ImageDivisionInterface : public Tool, public BasicImageEditInterface
{
    Q_OBJECT

public:
    virtual ~ImageDivisionInterface() = default;

    /**
     * @brief 横の分割数を設定する
     * @param n 横の分割数
     * @exception InvalidArgumentException &lt;unsigned int&gt; nが不正の場合
     */
    virtual void setHorizontalDivision(unsigned int n) = 0;
    /**
     * @brief 縦の分割数を設定する
     * @param m 縦の分割数
     * @exception InvalidArgumentException &lt;unsigned int&gt; mが不正の場合
     */
    virtual void setVerticalDivision(unsigned int m) = 0;
    /**
     * @brief 分割後の横の長さを設定する
     * @param width 分割後の横の長さ
     * @exception InvalidArgumentException &lt;unsigned int&gt; widthが不正の場合
     */
    virtual void setCellWidth(unsigned int width) = 0;
    /**
     * @brief 分割後の縦の長さを設定する
     * @param height 分割後の縦の長さ
     * @exception InvalidArgumentException &lt;unsigned int&gt; heightが不正の場合
     */
    virtual void setCellHeight(unsigned int height) = 0;

    /**
     * @brief 元の画像を返す
     * @return 元の画像 
     */
    virtual const QImage &original() const = 0;

    /**
     * @brief 現在の設定に基づいて計算される分割後の画像サイズ
     * @return 分割後の画像サイズ
     */
    virtual QSizeF computedCellSize() const = 0;

    /**
     * @brief 現在の設定に基づいて計算される画像の横の分割数
     * @return 横の分割数。 `discardRemainders == false` の場合は端数を含む
     */
    virtual unsigned int numberOfHorizontalDivision() const = 0;

    /**
     * @brief 現在の設定に基づいて計算される画像の縦の分割数
     * @return 縦の分割数。 `discardRemainders == false` の場合は端数を含む
     */
    virtual unsigned int numberOfVerticalDivision() const = 0;

    /// 分割後に画像の端の部分の大きさが十分でない場合、保存対象にしない
    bool discardRemainders = true;

protected:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageDivisionInterface(QObject *parent = nullptr);
};

/**
 * @brief 画像を分割するツール
 */
class ImageDivision : public ImageDivisionInterface, private ImageIO
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ImageDivision(QObject *parent = nullptr);

    inline bool save(const QString &path,
                     const char *format = nullptr,
                     int quality = -1) const override
    {
        return saveImpl(ImageIO::save, path, format, quality);
    }
    inline bool overwriteSave(const QString &path,
                              const char *format = nullptr,
                              int quality = -1) const override
    {
        return saveImpl(ImageIO::overwriteSave, path, format, quality);
    }
    inline const QFileInfo &fileInfo(unsigned int index = 0) const override
    {
        return ImageIO::originalFileInfo();
    }

    void setHorizontalDivision(unsigned int n) noexcept(false) override;
    void setVerticalDivision(unsigned int m) noexcept(false) override;
    void setCellWidth(unsigned int width) noexcept(false) override;
    void setCellHeight(unsigned int height) noexcept(false) override;
    const QImage &original() const override { return ImageIO::original(); }
    QSizeF computedCellSize() const override;
    unsigned int numberOfHorizontalDivision() const override;
    unsigned int numberOfVerticalDivision() const override;

protected:
    bool loadImpl(const QString &path) override;
    void resetImpl() override;
    inline bool updateImpl() override { return !current().isNull(); } // 特に何もしない

private:
    /// 分割数が不正
    static const QString invalidDivisionNumber;
    /// 分割後のサイズが不正
    static const QString invalidCellSize;
    /// 元の画像サイズが不正
    static const QString invalidImageSize;

    /// 分割数指定かサイズ指定かを保存する構造体
    struct DivisionHints
    {
        /// `true`ならサイズで指定、`false`なら分割数指定
        bool isSpecifiedWithSize = false;
        /// サイズもしくは分割数
        unsigned int value = 1;
    };
    /// 横の分割指定
    DivisionHints horizontal;
    /// 縦の分割指定
    DivisionHints vertical;

    /**
     * @brief computedCellSize() の算出に使う
     * @param sourceSize 分割前のサイズ
     * @param hint 分割方法
     * @return 分割後のサイズ
     */
    static double computedCellSizeInternal(unsigned int sourceSize,
                                           const DivisionHints &hint) noexcept(false);

    /**
     * @brief 現在の設定に基づき、画像の分割数を算出する
     * @param sourceSize 分割前のサイズ
     * @param hint 分割方法
     * @return 分割数
     */
    unsigned int numberOfDivisionInternal(unsigned int sourceSize, const DivisionHints &hint) const
        noexcept(false);

    /**
     * @brief 保存処理の共通部分
     * @param saveFunc 保存処理の関数ポインタ
     * @param path ファイルパス
     * @param format フォーマット
     * @param quality 画質
     * @return 成功なら`true`
     */
    bool saveImpl(bool (*saveFunc)(const QString &, const QImage &, const char *, int),
                  const QString &path,
                  const char *format,
                  int quality) const;

#ifdef _TEST_ImageDivision
    friend class Test::TestImageDivision;
#endif
};

#endif // IMAGE_DIVISION_H
