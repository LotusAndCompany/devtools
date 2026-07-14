#include "file_dialogs.h"

#include <QDir>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>

/**
 * @brief fileNameFilter() の実装部分
 * @param formats supportedImageFormats() の返り値
 * @return Images (*.aaa *.bbb ...) の形式の文字列
 */
QString fileNameFilterImpl(const QList<QByteArray> &formats)
{
    QString filter = QCoreApplication::tr("Images") + " (";
    for (const auto &item : formats) {
        filter += "*." + item + " ";
    }
    filter.replace(filter.length() - 1, 1, ')');

    return filter;
}

ImageOpenDialog::ImageOpenDialog(QWidget *parent)
    : QFileDialog(parent, tr("Open Image"),
                  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                  fileNameFilter())
{
    setAcceptMode(QFileDialog::AcceptOpen);
    setFileMode(QFileDialog::ExistingFile);
}

QString ImageOpenDialog::fileNameFilter()
{
    return fileNameFilterImpl(QImageReader::supportedImageFormats());
}

ImageSaveDialog::ImageSaveDialog(QWidget *parent)
    : QFileDialog(parent, tr("Save Image"),
                  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                  fileNameFilter())
{
    setAcceptMode(QFileDialog::AcceptSave);
}

QString ImageSaveDialog::fileNameFilter()
{
    return fileNameFilterImpl(QImageWriter::supportedImageFormats());
}
