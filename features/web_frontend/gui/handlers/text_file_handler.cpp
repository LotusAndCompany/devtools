#include "features/web_frontend/gui/bridge/bridge_io.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMetaType>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringDecoder>

#include <algorithm>

void NativeBridge::startFileDialog(const QString &requestId, const QString &operation,
                                   const QVariantMap &payload)
{
    const bool saving = operation == QStringLiteral("saveTextFile");
    if (saving) {
        QString text;
        if (!textPayload(requestId, payload, QStringLiteral("Text to save is larger than 1 MiB."),
                         &text)) {
            operations_.remove(requestId);
            return;
        }
    }
    const QString fileType = payload.value(QStringLiteral("fileType")).toString();
    QString filter = tr("Text files (*.txt);;All files (*)");
    QString suffix = QStringLiteral("txt");
    if (fileType == QStringLiteral("data")) {
        filter = tr("Data files (*.json *.yaml *.yml *.toml);;All files (*)");
        suffix = QStringLiteral("json");
    } else if (fileType == QStringLiteral("markdown")) {
        filter = tr("Markdown files (*.md *.markdown);;All files (*)");
        suffix = QStringLiteral("md");
    } else if (fileType == QStringLiteral("html")) {
        filter = tr("HTML files (*.html *.htm);;All files (*)");
        suffix = QStringLiteral("html");
    }
    auto *dialog = new QFileDialog(
        dialog_parent_, saving ? tr("Save text file") : tr("Open text file"), QString(), filter);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFileMode(saving ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
    if (saving) {
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        const QString suggested =
            safeSuggestedName(payload.value(QStringLiteral("suggestedName")).toString());
        dialog->setDefaultSuffix(suffix);
        dialog->selectFile(suggested.isEmpty() ? QStringLiteral("output.%1").arg(suffix)
                                               : suggested);
    }

    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        dialog->deleteLater();
        return;
    }
    it->dialog = dialog;
    reportProgress(requestId, 5, QStringLiteral("Waiting for file selection"));

    connect(dialog, &QFileDialog::accepted, this, [this, dialog, requestId, operation, payload]() {
        auto current = operations_.find(requestId);
        if (current == operations_.end()) {
            return;
        }
        const QString path = dialog->selectedFiles().value(0);
        current->dialog = nullptr;
        if (path.isEmpty() || current->cancellation->load(std::memory_order_relaxed)) {
            operations_.erase(current);
            emit operationCancelled(requestId);
            return;
        }

        if (operation == QStringLiteral("openTextFile")) {
            startWorker(requestId, operation,
                        [path](const CancellationToken &token, const ProgressCallback &progress) {
                            return NativeBridge::readTextFile(path, token, progress);
                        });
            return;
        }

        const QString text = payload.value(QStringLiteral("text")).toString();
        startWorker(requestId, operation,
                    [path, text](const CancellationToken &token, const ProgressCallback &progress) {
                        return NativeBridge::saveTextFile(path, text, token, progress);
                    });
    });
    connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
        auto current = operations_.find(requestId);
        if (current != operations_.end()) {
            operations_.erase(current);
            emit operationCancelled(requestId);
        }
    });
    dialog->open();
}

NativeBridge::OperationResult NativeBridge::readTextFile(const QString &path,
                                                         const CancellationToken &cancellation,
                                                         const ProgressCallback &progress)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return OperationResult{{}, frontendIoErrorCode(file.error()), file.errorString(), false};
    }
    if (file.size() > MAX_INPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Selected file is larger than 1 MiB."),
                               false};
    }

    QByteArray bytes;
    const qint64 totalBytes = file.size();
    while (!file.atEnd()) {
        if (cancellation->load(std::memory_order_relaxed)) {
            return OperationResult{{}, {}, {}, true};
        }
        const QByteArray chunk = file.read(FRONTEND_IO_CHUNK_BYTES);
        if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
            return OperationResult{
                {}, frontendIoErrorCode(file.error()), file.errorString(), false};
        }
        bytes.append(chunk);
        if (bytes.size() > MAX_INPUT_BYTES) {
            return OperationResult{{},
                                   QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                                   QStringLiteral("Selected file is larger than 1 MiB."),
                                   false};
        }
        const int percent =
            totalBytes > 0
                ? 10 + static_cast<int>(static_cast<qint64>(bytes.size()) * 80 / totalBytes)
                : 90;
        progress(std::min(percent, 90), QStringLiteral("Reading file"));
    }

    QStringDecoder decoder(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
    const QString text = decoder(bytes);
    if (decoder.hasError()) {
        return OperationResult{{},
                               QStringLiteral("INVALID_ENCODING"),
                               QStringLiteral("The selected file is not valid UTF-8."),
                               false};
    }
    return OperationResult{
        {{QStringLiteral("text"), text}, {QStringLiteral("fileName"), QFileInfo(path).fileName()}},
        {},
        {},
        false};
}

NativeBridge::OperationResult NativeBridge::saveTextFile(const QString &path, const QString &text,
                                                         const CancellationToken &cancellation,
                                                         const ProgressCallback &progress)
{
    const QByteArray bytes = text.toUtf8();
    if (bytes.size() > MAX_INPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Text to save is larger than 1 MiB."),
                               false};
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return OperationResult{{}, frontendIoErrorCode(file.error()), file.errorString(), false};
    }
    qint64 writtenBytes = 0;
    while (writtenBytes < bytes.size()) {
        if (cancellation->load(std::memory_order_relaxed)) {
            file.cancelWriting();
            return OperationResult{{}, {}, {}, true};
        }
        const qint64 remainingBytes = bytes.size() - writtenBytes;
        const qint64 chunkSize = std::min<qint64>(FRONTEND_IO_CHUNK_BYTES, remainingBytes);
        const qint64 written = file.write(bytes.constData() + writtenBytes, chunkSize);
        if (written <= 0) {
            file.cancelWriting();
            return OperationResult{
                {}, frontendIoErrorCode(file.error()), file.errorString(), false};
        }
        writtenBytes += written;
        const int percent =
            bytes.isEmpty() ? 90 : 10 + static_cast<int>(writtenBytes * 80 / bytes.size());
        progress(std::min(percent, 90), QStringLiteral("Writing file"));
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        file.cancelWriting();
        return OperationResult{{}, {}, {}, true};
    }
    if (!file.commit()) {
        return OperationResult{{}, frontendIoErrorCode(file.error()), file.errorString(), false};
    }

    return OperationResult{{{QStringLiteral("fileName"), QFileInfo(path).fileName()},
                            {QStringLiteral("bytesWritten"), writtenBytes}},
                           {},
                           {},
                           false};
}

QString NativeBridge::safeSuggestedName(const QString &name)
{
    QString safeName = name;
    safeName.replace(QRegularExpression(QStringLiteral("[/\\\\]")), QStringLiteral("_"));
    safeName.remove(QStringLiteral(".."));
    safeName = safeName.trimmed();
    if (safeName.isEmpty()) {
        return QStringLiteral("output.txt");
    }
    return safeName.left(128);
}
