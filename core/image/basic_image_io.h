#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QString>

#ifdef _TEST_TestImageIO
namespace Test {
class TestImageIO;
}
#endif

/**
 * @brief 画像を読み書きする機能を提供するクラス。複数の画像を読み書きするなら不要、というか別の方法を用意する必要がある。
 * @details 保存する画像のデータ型として、
 *          [QImage](https://doc.qt.io/qt-6/qimage.html),
 *          [QPixmap](https://doc.qt.io/qt-6/qpixmap.html), 
 *          [QBitmap](https://doc.qt.io/qt-6/qbitmap.html)
 *          を想定している。
 */
class ImageIO
{
protected:
    /**
     * @brief コンストラクタ
     */
    ImageIO() = default;
    /**
     * @brief デストラクタ
     */
    ~ImageIO() = default;

    /**
     * @brief 画像を読み込み、_original に格納する
     * @param path 読み込む画像
     * @return 成功なら`true`
     */
    bool load(const QString &path);
    /**
     * @brief 画像を上書き保存する
     * @tparam QImageType 保存する画像データの型
     * @param path 保存先
     * @param image 保存する画像データ
     * @param format 保存形式。"PNG"など。nullptrならファイル名から推定する。
     * @param quality 画質。
     *        -1=デフォルト、
     *        0=最低(圧縮率優先)、
     *        100=最高(ほぼ圧縮されない)
     * @return 成功なら`true`
     */
    template<typename QImageType>
    static inline bool overwriteSave(const QString &path,
                                     const QImageType &image,
                                     const char *format = nullptr,
                                     int quality = -1)
    {
        if (image.isNull())
            return false;
        else
            return image.save(path, format, quality);
    }
    /**
     * @brief 画像を保存する。上書きはせず、既にファイルが存在する場合は失敗する。
     * @tparam QImageType 保存する画像データの型
     * @param path 保存先
     * @param image 保存する画像データ
     * @param format 保存形式。"PNG"など。nullptrならファイル名から推定する。
     * @param quality 画質。
     *        -1=デフォルト、
     *        0=最低(圧縮率優先)、
     *        100=最高(ほぼ圧縮されない)
     * @return 成功なら`true`
     */
    template<typename QImageType>
    static inline bool save(const QString &path,
                            const QImageType &image,
                            const char *format = nullptr,
                            int quality = -1)
    {
        if (image.isNull() || QFileInfo::exists(path))
            return false;
        else
            return image.save(path, format, quality);
    }

    /**
     * @brief 現在読み込まれている画像を返す
     * @return  現在読み込まれている画像
     */
    inline const QImage &original() const { return _original; }
    /**
     * @brief 現在読み込まれている画像のファイル情報を返す
     * @return 現在読み込まれている画像のファイル情報
     */
    inline const QFileInfo &originalFileInfo() const { return _fileInfo; }

private:
    /// 読み込んだ画像
    QImage _original;
    /// 読み込んだ画像ファイルのファイル情報
    QFileInfo _fileInfo;

#ifdef _TEST_TestImageIO
    friend class Test::TestImageIO;
#endif
};

#endif // IMAGE_IO_H
