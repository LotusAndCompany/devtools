#include "tests/test_util.h"

#include <QDateTime>
#include <QtTest>

namespace Test {
class TestLogging : public QObject
{
    Q_OBJECT

private:
    static QByteArray s_captured;

    static void captureHandler(QtMsgType type, const QMessageLogContext &, const QString &msg);
    static void resetHandler();

private slots:
    void test_outputContainsTypeLabel();
    void test_handlerReceivesCorrectType();
    void test_outputContainsMessage();
};

QByteArray TestLogging::s_captured;

void TestLogging::captureHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        s_captured += "DEBUG:";
        break;
    case QtWarningMsg:
        s_captured += "WARN:";
        break;
    case QtCriticalMsg:
        s_captured += "CRIT:";
        break;
    case QtInfoMsg:
        s_captured += "INFO:";
        break;
    case QtFatalMsg:
        s_captured += "FATAL:";
        break;
    }
    s_captured += msg.toUtf8();
}

void TestLogging::resetHandler()
{
    qInstallMessageHandler(nullptr);
}

void TestLogging::test_outputContainsTypeLabel()
{
    qInstallMessageHandler(captureHandler);

    s_captured.clear();
    qDebug() << "debug_test";
    QVERIFY(s_captured.startsWith("DEBUG:"));
    QVERIFY(s_captured.contains("debug_test"));

    s_captured.clear();
    qWarning() << "warn_test";
    QVERIFY(s_captured.startsWith("WARN:"));
    QVERIFY(s_captured.contains("warn_test"));

    s_captured.clear();
    qCritical() << "crit_test";
    QVERIFY(s_captured.startsWith("CRIT:"));
    QVERIFY(s_captured.contains("crit_test"));

    resetHandler();
}

void TestLogging::test_handlerReceivesCorrectType()
{
    qInstallMessageHandler(captureHandler);

    s_captured.clear();
    qInfo() << "info_message";
    QVERIFY(s_captured.startsWith("INFO:"));

    s_captured.clear();
    qWarning() << "warning_message";
    QVERIFY(s_captured.startsWith("WARN:"));

    s_captured.clear();
    qCritical() << "critical_message";
    QVERIFY(s_captured.startsWith("CRIT:"));

    resetHandler();
}

void TestLogging::test_outputContainsMessage()
{
    qInstallMessageHandler(captureHandler);

    s_captured.clear();
    qDebug() << "hello world";
    QVERIFY(s_captured.contains("hello world"));

    resetHandler();
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestLogging)

#include "test_logging.moc"
