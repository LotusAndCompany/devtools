#ifndef FILE_DIALOGS_H
#define FILE_DIALOGS_H

#include <QFileDialog>

/**
 * @brief 画像読み込み用のファイルダイアログ
 */
class ImageOpenDialog : public QFileDialog
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ImageOpenDialog(QWidget *parent = nullptr);

private:
    /**
     * @brief ファイル名フィルタを作成して返す
     * @return Images (*.aaa *.bbb ...) の形式の文字列
     */
    static QString fileNameFilter();
};

/**
 * @brief 画像保存用のファイルダイアログ
 */
class ImageSaveDialog : public QFileDialog
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ImageSaveDialog(QWidget *parent = nullptr);

private:
    /**
     * @brief ファイル名フィルタを作成して返す
     * @return Images (*.aaa *.bbb ...) の形式の文字列
     */
    static QString fileNameFilter();
};

#endif // FILE_DIALOGS_H
