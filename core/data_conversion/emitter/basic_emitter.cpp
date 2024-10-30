#include "basic_emitter.h"

#include <QRegularExpression>

QString BasicEmitter::replace4spaceIndentation(const QString &src, const QString &newIndent) const
{
    if (src.contains('\r') || src.contains('\n')) {
        static QRegularExpression lineEndRegEx("[\r\n]");
        QStringList lines = src.split(lineEndRegEx, Qt::SkipEmptyParts);

        QString result;
        QTextStream stream(&result);
        for (const QString &line : lines) {
            int count = 0;
            while (line[count] == ' ')
                count++;

            if (count % 4)
                qWarning() << "indentation might be incorrect";

            stream << newIndent.repeated(count / 4) + line.mid(count) << Qt::endl;
        }

        // NOTE: 最後の改行は取り除かない
        return stream.readAll();
    }

    return "";
}
