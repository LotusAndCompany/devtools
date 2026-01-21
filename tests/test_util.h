#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "mock_helper.h"
#include "random_data.h"

#include <QString>

namespace Test {
extern const QString TEST_SRC_DIR;
extern const QString TEST_BIN_DIR;

bool isEqaulApprox(double a, double b, double errorRatio = 1.0 / 256.0);
} // namespace Test

#endif // TEST_UTIL_H
