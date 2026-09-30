#include "sqlite_file_access.h"

#import <Foundation/Foundation.h>

#include <utility>

struct SQLiteFileAccess::Impl
{
    NSURL *url = nil;
    QString filePath;
    QByteArray bookmarkData;
    bool isAccessing = false;

    ~Impl()
    {
        if (url != nil) {
            if (isAccessing) {
                [url stopAccessingSecurityScopedResource];
            }
            [url release];
        }
    }
};

namespace {
QByteArray toByteArray(NSData *data)
{
    return QByteArray(static_cast<const char *>(data.bytes), static_cast<qsizetype>(data.length));
}

QString toQString(NSURL *url)
{
    return QString::fromUtf8(url.path.UTF8String);
}

NSData *toNSData(const QByteArray &data)
{
    return [[NSData alloc] initWithBytes:data.constData()
                                  length:static_cast<NSUInteger>(data.size())];
}

NSURL *toFileUrl(const QString &filePath)
{
    const QByteArray utf8Path = filePath.toUtf8();
    NSString *path = [[NSString alloc] initWithBytes:utf8Path.constData()
                                              length:static_cast<NSUInteger>(utf8Path.size())
                                            encoding:NSUTF8StringEncoding];
    if (path == nil) {
        return nil;
    }

    NSURL *url = [NSURL fileURLWithPath:path];
    [path release];
    return url;
}

NSData *createBookmark(NSURL *url)
{
    NSError *error = nil;
    return [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
          includingResourceValuesForKeys:nil
                           relativeToURL:nil
                                   error:&error];
}
} // namespace

SQLiteFileAccess::SQLiteFileAccess(std::unique_ptr<Impl> impl) : impl(std::move(impl)) {}

SQLiteFileAccess::~SQLiteFileAccess() = default;

std::unique_ptr<SQLiteFileAccess> SQLiteFileAccess::fromFilePath(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return nullptr;
    }

    auto impl = std::make_unique<SQLiteFileAccess::Impl>();
    NSURL *url = toFileUrl(filePath);
    if (url == nil) {
        return nullptr;
    }

    NSData *bookmark = createBookmark(url);
    if (bookmark != nil) {
        BOOL isStale = NO;
        NSError *error = nil;
        NSURL *resolvedUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                                       options:NSURLBookmarkResolutionWithSecurityScope
                                                 relativeToURL:nil
                                           bookmarkDataIsStale:&isStale
                                                         error:&error];
        if (resolvedUrl != nil) {
            impl->url = [resolvedUrl retain];
            impl->bookmarkData = toByteArray(bookmark);
            impl->isAccessing = [impl->url startAccessingSecurityScopedResource];
        }
    }

    if (impl->url == nil) {
        impl->url = [url retain];
    }
    impl->filePath = toQString(impl->url);
    return std::unique_ptr<SQLiteFileAccess>(new SQLiteFileAccess(std::move(impl)));
}

std::unique_ptr<SQLiteFileAccess> SQLiteFileAccess::fromBookmark(const QByteArray &bookmarkData)
{
    if (bookmarkData.isEmpty()) {
        return nullptr;
    }

    NSData *data = toNSData(bookmarkData);
    BOOL isStale = NO;
    NSError *error = nil;
    NSURL *url = [NSURL URLByResolvingBookmarkData:data
                                           options:NSURLBookmarkResolutionWithSecurityScope
                                     relativeToURL:nil
                               bookmarkDataIsStale:&isStale
                                             error:&error];
    [data release];
    if (url == nil) {
        return nullptr;
    }

    auto impl = std::make_unique<SQLiteFileAccess::Impl>();
    impl->url = [url retain];
    impl->filePath = toQString(url);
    impl->isAccessing = [impl->url startAccessingSecurityScopedResource];
    if (!impl->isAccessing) {
        return nullptr;
    }

    if (isStale) {
        NSData *updatedBookmark = createBookmark(impl->url);
        if (updatedBookmark != nil) {
            impl->bookmarkData = toByteArray(updatedBookmark);
        }
    } else {
        impl->bookmarkData = bookmarkData;
    }

    return std::unique_ptr<SQLiteFileAccess>(new SQLiteFileAccess(std::move(impl)));
}

const QString &SQLiteFileAccess::filePath() const
{
    return impl->filePath;
}

const QByteArray &SQLiteFileAccess::bookmarkData() const
{
    return impl->bookmarkData;
}
