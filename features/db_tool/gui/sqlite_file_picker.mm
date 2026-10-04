#include "sqlite_file_picker.h"

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <QFileInfo>
#include <QMessageBox>
#include <QWidget>

namespace {
NSData *createBookmark(NSURL *url)
{
    NSError *error = nil;
    return [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
          includingResourceValuesForKeys:nil
                           relativeToURL:nil
                                   error:&error];
}
} // namespace

std::unique_ptr<SQLiteFileAccess> selectSQLiteDatabaseFile(QWidget *parent,
                                                           const QString &initialPath)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseFiles = YES;
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = NO;
    panel.allowedContentTypes = @[[UTType typeWithFilenameExtension:@"db"],
                                  [UTType typeWithFilenameExtension:@"sqlite"],
                                  [UTType typeWithFilenameExtension:@"sqlite3"]];

    if (!initialPath.isEmpty()) {
        QFileInfo const initialInfo(initialPath);
        const QString directory = initialInfo.isDir() ? initialInfo.absoluteFilePath()
                                                      : initialInfo.absolutePath();
        const QByteArray directoryUtf8 = directory.toUtf8();
        NSString *directoryString = [[NSString alloc] initWithBytes:directoryUtf8.constData()
                                                              length:static_cast<NSUInteger>(
                                                                          directoryUtf8.size())
                                                            encoding:NSUTF8StringEncoding];
        if (directoryString != nil) {
            panel.directoryURL = [NSURL fileURLWithPath:directoryString isDirectory:YES];
            [directoryString release];
        }
    }

    if ([panel runModal] != NSModalResponseOK) {
        return nullptr;
    }

    NSURL *selectedUrl = [[panel URL] retain];
    BOOL startedAccessing = [selectedUrl startAccessingSecurityScopedResource];
    NSData *bookmark = createBookmark(selectedUrl);
    std::unique_ptr<SQLiteFileAccess> fileAccess;
    if (bookmark != nil) {
        const QByteArray bookmarkData(static_cast<const char *>(bookmark.bytes),
                                      static_cast<qsizetype>(bookmark.length));
        fileAccess = SQLiteFileAccess::fromBookmark(bookmarkData);
    }

    if (!fileAccess) {
        fileAccess = SQLiteFileAccess::fromSecurityScopedUrl(selectedUrl, startedAccessing);
        if (fileAccess) {
            startedAccessing = NO; // SQLiteFileAccess now owns the access lifetime.
        }
    }

    if (startedAccessing) {
        [selectedUrl stopAccessingSecurityScopedResource];
    }
    [selectedUrl release];
    if (!fileAccess) {
        QMessageBox::critical(parent, QObject::tr("File Opening Error"),
                              QObject::tr("Could not access the selected database file."));
    }
    return fileAccess;
}
