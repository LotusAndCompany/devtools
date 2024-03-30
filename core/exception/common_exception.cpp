#include "common_exception.h"
#include <QTextStream>

CommonException::CommonException(const QString &message)
    : message(message)
{}

QException *CommonException::clone() const
{
    return new CommonException(*this);
}

QTextStream &operator<<(QTextStream &stream, const CommonException &exception)
{
    return stream << exception.message;
}
