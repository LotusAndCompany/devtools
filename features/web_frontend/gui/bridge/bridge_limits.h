#ifndef BRIDGE_LIMITS_H
#define BRIDGE_LIMITS_H

#include <QtGlobal>

namespace BridgeLimits {
inline constexpr qsizetype MAX_INPUT_BYTES = 1024 * 1024;
inline constexpr qsizetype MAX_OUTPUT_BYTES = 4 * 1024 * 1024;
inline constexpr qsizetype MAX_MESSAGE_BYTES = 2 * 1024 * 1024;
} // namespace BridgeLimits

#endif // BRIDGE_LIMITS_H
