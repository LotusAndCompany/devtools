#include "features/image/core/image_processing_service.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

class TestImageProcessingService : public QObject
{
    Q_OBJECT

private slots:
    void loadAndSave();
    void resizeAndTransform();
    void transparency();
    void saveDividedImages();
};

void TestImageProcessingService::loadAndSave()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QImage source(2, 1, QImage::Format_RGB32);
    source.setPixelColor(0, 0, Qt::red);
    source.setPixelColor(1, 0, Qt::blue);
    const QString sourcePath = directory.filePath("source.png");
    QVERIFY(source.save(sourcePath));

    const QImage loaded = ImageProcessingService::load(sourcePath);
    QCOMPARE(loaded.size(), QSize(2, 1));
    QCOMPARE(loaded.format(), QImage::Format_RGBA8888);
    QCOMPARE(loaded.pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(loaded.pixelColor(1, 0), QColor(Qt::blue));

    const QString savedPath = directory.filePath("saved.png");
    QVERIFY(ImageProcessingService::save(loaded, savedPath));
    const QImage saved(savedPath);
    QCOMPARE(saved.size(), loaded.size());
    QCOMPARE(saved.pixelColor(0, 0), loaded.pixelColor(0, 0));
    QCOMPARE(saved.pixelColor(1, 0), loaded.pixelColor(1, 0));
    QVERIFY(ImageProcessingService::load(directory.filePath("missing.png")).isNull());
    QVERIFY(!ImageProcessingService::save(QImage(), savedPath));
    QVERIFY(!ImageProcessingService::save(loaded, directory.filePath("missing/out.png")));
}

void TestImageProcessingService::resizeAndTransform()
{
    QImage source(2, 2, QImage::Format_RGBA8888);
    source.setPixelColor(0, 0, Qt::red);
    source.setPixelColor(1, 0, Qt::blue);
    source.setPixelColor(0, 1, Qt::green);
    source.setPixelColor(1, 1, Qt::yellow);
    QImage wide(4, 2, QImage::Format_RGBA8888);
    wide.fill(Qt::red);
    QImage thin(4, 1, QImage::Format_RGBA8888);
    thin.fill(Qt::red);

    QCOMPARE(ImageProcessingService::resizeBySize(source, 8, 8, true, false).size(), QSize(8, 8));
    QCOMPARE(ImageProcessingService::resizeBySize(wide, 8, 8, true, false).size(), QSize(8, 4));
    QCOMPARE(ImageProcessingService::resizeBySize(thin, 1, 1, true, false), thin);
    QCOMPARE(ImageProcessingService::resizeBySize(source, 0, 8, false, false), source);
    QCOMPARE(ImageProcessingService::resizeByScale(source, 50.0, 100.0, false, false).size(),
             QSize(1, 2));
    QCOMPARE(ImageProcessingService::resizeByScale(source, 100.0, 25.0, true, true).size(),
             QSize(2, 2));
    QCOMPARE(ImageProcessingService::resizeByScale(wide, 50.0, 100.0, true, false).size(),
             QSize(2, 1));

    const QImage flippedHorizontal =
        ImageProcessingService::flip(source, ImageFlipDirection::Horizontal);
    QCOMPARE(flippedHorizontal.pixelColor(0, 0), QColor(Qt::blue));
    QCOMPARE(flippedHorizontal.pixelColor(1, 1), QColor(Qt::green));
    const QImage flippedVertical =
        ImageProcessingService::flip(source, ImageFlipDirection::Vertical);
    QCOMPARE(flippedVertical.pixelColor(0, 0), QColor(Qt::green));
    QCOMPARE(flippedVertical.pixelColor(1, 1), QColor(Qt::blue));

    QCOMPARE(ImageProcessingService::rotateLeft(ImageProcessingService::rotateRight(source)),
             source);
}

void TestImageProcessingService::transparency()
{
    QCOMPARE(ImageProcessingService::alphaForTransparency(0.0), uint8_t(255));
    QCOMPARE(ImageProcessingService::alphaForTransparency(0.5), uint8_t(127));
    QCOMPARE(ImageProcessingService::alphaForTransparency(1.0), uint8_t(0));
    QCOMPARE(ImageProcessingService::alphaForTransparency(-1.0), uint8_t(255));
    QCOMPARE(ImageProcessingService::alphaForTransparency(2.0), uint8_t(0));

    QImage source(4, 1, QImage::Format_RGBA8888);
    source.setPixelColor(0, 0, Qt::red);
    source.setPixelColor(1, 0, QColor(250, 0, 0));
    source.setPixelColor(2, 0, Qt::blue);
    source.setPixelColor(3, 0, Qt::red);
    const QColor::Spec specs[] = {QColor::Spec::Rgb, QColor::Spec::Hsv, QColor::Spec::Hsl};
    for (const QColor::Spec spec : specs) {
        const QImage result = ImageProcessingService::makeTransparent(source, QPoint(0, 0), Qt::red,
                                                                      spec, 0.05, 0, false);
        QCOMPARE(result.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(result.pixelColor(2, 0).alpha(), 255);
    }

    const QImage contiguous = ImageProcessingService::makeTransparent(
        source, QPoint(0, 0), Qt::red, QColor::Spec::Rgb, 0.05, 64, true);
    QCOMPARE(contiguous.pixelColor(0, 0).alpha(), 64);
    QCOMPARE(contiguous.pixelColor(1, 0).alpha(), 64);
    QCOMPARE(contiguous.pixelColor(2, 0).alpha(), 255);
    QCOMPARE(contiguous.pixelColor(3, 0).alpha(), 255);
    QCOMPARE(source.pixelColor(0, 0).alpha(), 255);

    const QImage invalidStart = ImageProcessingService::makeTransparent(
        source, QPoint(-1, 0), Qt::red, QColor::Spec::Rgb, 0.0, 0, true);
    QCOMPARE(invalidStart, source);
}

void TestImageProcessingService::saveDividedImages()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QImage source(5, 3, QImage::Format_RGB32);
    source.fill(Qt::red);
    const QString originalPath = directory.filePath("sample.png");
    QVERIFY(source.save(originalPath));

    const QString dividedPath = directory.filePath("division");
    QVERIFY(QDir().mkpath(dividedPath));
    ImageSplitOptions division;
    division.mode = ImageSplitMode::DivisionCount;
    division.horizontalDivisions = 2;
    division.verticalDivisions = 2;
    QVERIFY(ImageProcessingService::saveDividedImages(source, originalPath, dividedPath, division));
    QCOMPARE(QImage(QDir(dividedPath).filePath("sample_0_0.png")).size(), QSize(2, 1));
    QCOMPARE(QImage(QDir(dividedPath).filePath("sample_1_0.png")).size(), QSize(3, 1));
    QCOMPARE(QImage(QDir(dividedPath).filePath("sample_0_1.png")).size(), QSize(2, 2));
    QCOMPARE(QImage(QDir(dividedPath).filePath("sample_1_1.png")).size(), QSize(3, 2));

    const QString cellsPath = directory.filePath("cells");
    QVERIFY(QDir().mkpath(cellsPath));
    ImageSplitOptions cells;
    cells.mode = ImageSplitMode::CellSize;
    cells.cellWidth = 2;
    cells.cellHeight = 2;
    QVERIFY(ImageProcessingService::saveDividedImages(source, originalPath, cellsPath, cells));
    QCOMPARE(QImage(QDir(cellsPath).filePath("sample_2_0.png")).size(), QSize(1, 2));
    QCOMPARE(QImage(QDir(cellsPath).filePath("sample_0_1.png")).size(), QSize(2, 1));
    QCOMPARE(QImage(QDir(cellsPath).filePath("sample_2_1.png")).size(), QSize(1, 1));

    const QString ignoredPath = directory.filePath("ignored");
    QVERIFY(QDir().mkpath(ignoredPath));
    cells.ignoreRemainders = true;
    QVERIFY(ImageProcessingService::saveDividedImages(source, originalPath, ignoredPath, cells));
    QVERIFY(QFileInfo::exists(QDir(ignoredPath).filePath("sample_1_0.png")));
    QVERIFY(!QFileInfo::exists(QDir(ignoredPath).filePath("sample_2_0.png")));

    QVERIFY(!ImageProcessingService::saveDividedImages(source, originalPath,
                                                       directory.filePath("missing"), division));
    QVERIFY(
        !ImageProcessingService::saveDividedImages(QImage(), originalPath, dividedPath, division));
}

QTEST_GUILESS_MAIN(TestImageProcessingService)
#include "test_image_processing_service.moc"
