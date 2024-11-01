#include "test_util.h"

#include <QDebug>
#include "test_config.cpp"

namespace Test {
const QString TEST_SRC_DIR = QString(_TEST_SRC_DIR);
const QString TEST_BIN_DIR = QString(_TEST_BIN_DIR);

bool isEqaulApprox(double a, double b, double errorRatio)
{
    if (errorRatio < 0) {
        qCritical() << "errorRatio must be grater or equal to 0";
        return false;
    }

    if (a == b)
        return true;

    if (a == 0.0 || b == 0.0) {
        qWarning() << "a or b appears to be 0. use errorRatio as absolute tolerance value";
        return abs(a - b) < errorRatio;
    }

    const double r = abs(a / b) - 1.0;
    const bool result = (-errorRatio <= r && r <= errorRatio);

    if (!result)
        qWarning() << "a=" << a << ", b=" << b << ", r=" << r;

    return result;
}
}
