#ifndef BRIDGE_IO_H
#define BRIDGE_IO_H

#include <QFileDevice>
#include <QString>

inline constexpr qsizetype FRONTEND_IO_CHUNK_BYTES = qsizetype{64} * 1024;

inline QString frontendIoErrorCode(QFileDevice::FileError error)
{
    return error == QFileDevice::PermissionsError ? QStringLiteral("PERMISSION_DENIED")
                                                  : QStringLiteral("IO_ERROR");
}

#endif // BRIDGE_IO_H
