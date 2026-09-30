#ifndef SQLITE_FILE_PICKER_H
#define SQLITE_FILE_PICKER_H

#include "../core/sqlite_file_access.h"

class QWidget;

std::unique_ptr<SQLiteFileAccess> selectSQLiteDatabaseFile(QWidget *parent,
                                                           const QString &initialPath);

#endif // SQLITE_FILE_PICKER_H
