#include "features/web_frontend/gui/bridge/operations/regex/regex_operations.h"

#include <QTest>

#include <atomic>
#include <memory>

class TestRegexOperations : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void rejectsAggregateCaptureOutput();
    void rejectsExpandedReplacementOutput();
    void cancelsDuringResultGeneration();
};

void TestRegexOperations::rejectsAggregateCaptureOutput()
{
    const QString text(300000, QLatin1Char('a'));
    const QString pattern = QString(15, QLatin1Char('(')) + QStringLiteral("(?:.{60000}){5}") +
                            QString(15, QLatin1Char(')'));
    const QVariantMap payload{{QStringLiteral("pattern"), pattern},
                              {QStringLiteral("text"), text},
                              {QStringLiteral("replacement"), QString()},
                              {QStringLiteral("global"), false}};
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const FrontendOperationService::Result result = FrontendOperationHandlers::evaluateRegex(
        payload, cancellation, [](int, const QString &) {});

    QVERIFY2(result.errorCode == QStringLiteral("SIZE_LIMIT_EXCEEDED"),
             qPrintable(result.errorMessage));
}

void TestRegexOperations::rejectsExpandedReplacementOutput()
{
    const QVariantMap payload{{QStringLiteral("pattern"), QStringLiteral(".")},
                              {QStringLiteral("text"), QStringLiteral("abcdef")},
                              {QStringLiteral("replacement"), QString(1000000, QLatin1Char('r'))}};
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const FrontendOperationService::Result result = FrontendOperationHandlers::evaluateRegex(
        payload, cancellation, [](int, const QString &) {});

    QCOMPARE(result.errorCode, QStringLiteral("SIZE_LIMIT_EXCEEDED"));
}

void TestRegexOperations::cancelsDuringResultGeneration()
{
    const QVariantMap payload{{QStringLiteral("pattern"), QStringLiteral(".")},
                              {QStringLiteral("text"), QString(10000, QLatin1Char('a'))},
                              {QStringLiteral("replacement"), QStringLiteral("x")}};
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const FrontendOperationService::Result result = FrontendOperationHandlers::evaluateRegex(
        payload, cancellation, [cancellation](int percent, const QString &) {
            if (percent == 15) {
                cancellation->store(true, std::memory_order_relaxed);
            }
        });

    QVERIFY(result.cancelled);
}

QTEST_APPLESS_MAIN(TestRegexOperations)
#include "test_regex_operations.moc"
