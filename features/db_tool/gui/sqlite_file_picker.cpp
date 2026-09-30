#include "sqlite_file_picker.h"

#include <QFileDialog>

std::unique_ptr<SQLiteFileAccess> selectSQLiteDatabaseFile(QWidget *parent,
                                                           const QString &initialPath)
{
    const QString selectedPath = QFileDialog::getOpenFileName(
        parent, QObject::tr("Select Database File"), initialPath,
        QObject::tr("SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"));
    return selectedPath.isEmpty() ? nullptr : SQLiteFileAccess::fromFilePath(selectedPath);
}
