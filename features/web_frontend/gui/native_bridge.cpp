#include "native_bridge.h"

#include "features/data_conversion/core/emitter/json_emitter.h"
#include "features/data_conversion/core/parser/json_parser.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringDecoder>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>
#include <array>
#include <cstddef>

namespace {
constexpr qsizetype IO_CHUNK_BYTES = qsizetype{64} * 1024;

QString ioErrorCode(QFileDevice::FileError error)
{
    return error == QFileDevice::PermissionsError ? QStringLiteral("PERMISSION_DENIED")
                                                  : QStringLiteral("IO_ERROR");
}
} // namespace

NativeBridge::NativeBridge(QWidget *dialogParent, QObject *parent)
    : QObject(parent), dialog_parent_(dialogParent)
{}

NativeBridge::~NativeBridge()
{
    for (auto &operation : operations_) {
        operation.cancellation->store(true, std::memory_order_relaxed);
        if (operation.dialog != nullptr) {
            operation.dialog->disconnect(this);
            operation.dialog->reject();
        }
        if (operation.watcher != nullptr) {
            operation.watcher->future().waitForFinished();
        }
    }
}

bool NativeBridge::validateRequest(const QString &requestId, const QVariantMap &request)
{
    if (!isValidRequestId(requestId)) {
        fail(QString(), QStringLiteral("INVALID_REQUEST"),
             QStringLiteral("Request ID must be 1 to 80 ASCII letters, digits, '_' or '-'."));
        return false;
    }
    if (request.value(QStringLiteral("contractVersion")).toString() != contract_version_) {
        fail(requestId, QStringLiteral("UNSUPPORTED_VERSION"),
             QStringLiteral("The bridge contract version is not supported."));
        return false;
    }
    if (operations_.contains(requestId)) {
        fail(requestId, QStringLiteral("DUPLICATE_REQUEST"),
             QStringLiteral("A request with this ID is already running."));
        return false;
    }
    if (operations_.size() >= MAX_CONCURRENT_OPERATIONS) {
        fail(requestId, QStringLiteral("TOO_MANY_REQUESTS"),
             QStringLiteral("The maximum number of concurrent operations was reached."));
        return false;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    if (operation != QStringLiteral("formatJson") && operation != QStringLiteral("openTextFile") &&
        operation != QStringLiteral("saveTextFile")) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The requested operation is not available."));
        return false;
    }
    return true;
}

bool NativeBridge::textPayload(const QString &requestId, const QVariantMap &payload,
                               const QString &sizeError, QString *text)
{
    const QVariant textValue = payload.value(QStringLiteral("text"));
    if (textValue.metaType().id() != QMetaType::QString) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The text field must be a string."));
        return false;
    }
    *text = textValue.toString();
    if (text->toUtf8().size() > MAX_INPUT_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"), sizeError);
        return false;
    }
    return true;
}

bool NativeBridge::parseIndentation(const QString &requestId, const QVariantMap &payload,
                                    int *indentation)
{
    const QString value = payload.value(QStringLiteral("indentation")).toString();
    if (value == QStringLiteral("2")) {
        *indentation = 0;
    } else if (value == QStringLiteral("4")) {
        *indentation = 1;
    } else if (value == QStringLiteral("tab")) {
        *indentation = 2;
    } else if (value == QStringLiteral("minified")) {
        *indentation = 3;
    } else {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Indentation must be 2, 4, tab or minified."));
        return false;
    }
    return true;
}

void NativeBridge::startJsonFormat(const QString &requestId, const QVariantMap &payload)
{
    QString text;
    int indentation = 0;
    if (!textPayload(requestId, payload, QStringLiteral("Input is larger than 1 MiB."), &text) ||
        !parseIndentation(requestId, payload, &indentation)) {
        operations_.remove(requestId);
        return;
    }

    reportProgress(requestId, 5, QStringLiteral("Preparing JSON formatter"));
    startWorker(
        requestId, QStringLiteral("formatJson"),
        [text, indentation](const CancellationToken &token, const ProgressCallback &progress) {
            return formatJson(text, indentation, token, progress);
        });
}

void NativeBridge::startOperation(const QVariantMap &request)
{
    const QString requestId = request.value(QStringLiteral("requestId")).toString();
    if (!validateRequest(requestId, request)) {
        return;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    const QVariantMap payload = request.value(QStringLiteral("payload")).toMap();
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    operations_.insert(requestId, RunningOperation{operation, cancellation, nullptr, nullptr});
    emit operationStarted(requestId, operation);

    if (operation == QStringLiteral("formatJson")) {
        startJsonFormat(requestId, payload);
        return;
    }
    if (operation == QStringLiteral("saveTextFile")) {
        QString text;
        if (!textPayload(requestId, payload, QStringLiteral("Text to save is larger than 1 MiB."),
                         &text)) {
            operations_.remove(requestId);
            return;
        }
    }

    startFileDialog(requestId, operation, payload);
}

void NativeBridge::cancelOperation(const QString &requestId)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }

    it->cancellation->store(true, std::memory_order_relaxed);
    if (it->dialog != nullptr) {
        it->dialog->reject();
    } else {
        reportProgress(requestId, 0, QStringLiteral("Cancellation requested"));
    }
}

void NativeBridge::fail(const QString &requestId, const QString &code, const QString &message)
{
    emit operationFailed(requestId,
                         {{QStringLiteral("code"), code}, {QStringLiteral("message"), message}});
}

void NativeBridge::startWorker(const QString &requestId, const QString &operation, Worker worker)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }
    if (it->cancellation->load(std::memory_order_relaxed)) {
        operations_.erase(it);
        emit operationCancelled(requestId);
        return;
    }

    const CancellationToken cancellation = it->cancellation;
    const ProgressCallback progress = [this, requestId](int percent, const QString &message) {
        emit operationProgress(requestId, percent, message);
    };
    auto *watcher = new QFutureWatcher<OperationResult>(this);
    it->operation = operation;
    it->watcher = watcher;

    connect(watcher, &QFutureWatcher<OperationResult>::finished, this,
            [this, requestId, watcher]() {
                const auto it = operations_.find(requestId);
                if (it == operations_.end() || it->watcher != watcher) {
                    watcher->deleteLater();
                    return;
                }

                const OperationResult result = watcher->result();
                operations_.erase(it);
                watcher->deleteLater();

                if (result.cancelled) {
                    emit operationCancelled(requestId);
                } else if (!result.errorCode.isEmpty()) {
                    fail(requestId, result.errorCode, result.errorMessage);
                } else {
                    emit operationProgress(requestId, 100, QStringLiteral("Complete"));
                    emit operationCompleted(requestId, result.value);
                }
            });

    watcher->setFuture(QtConcurrent::run([worker = std::move(worker), cancellation, progress]() {
        return worker(cancellation, progress);
    }));
}

void NativeBridge::reportProgress(const QString &requestId, int percent, const QString &message)
{
    emit operationProgress(requestId, percent, message);
}

void NativeBridge::startFileDialog(const QString &requestId, const QString &operation,
                                   const QVariantMap &payload)
{
    const bool saving = operation == QStringLiteral("saveTextFile");
    auto *dialog =
        new QFileDialog(dialog_parent_, saving ? tr("Save text file") : tr("Open text file"),
                        QString(), tr("Text files (*.txt *.json);;All files (*)"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFileMode(saving ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
    if (saving) {
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setDefaultSuffix(QStringLiteral("txt"));
        dialog->selectFile(
            safeSuggestedName(payload.value(QStringLiteral("suggestedName")).toString()));
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

NativeBridge::OperationResult NativeBridge::formatJson(const QString &text, int indentation,
                                                       const CancellationToken &cancellation,
                                                       const ProgressCallback &progress)
{
    if (cancellation->load(std::memory_order_relaxed)) {
        return OperationResult{{}, {}, {}, true};
    }
    progress(25, QStringLiteral("Parsing JSON"));
    const JsonParser::ParseResult parsed = JsonParser().tryParse(text);
    if (!parsed.success) {
        return OperationResult{
            {}, QStringLiteral("INVALID_INPUT"), parsed.errors.join(QStringLiteral("\n")), false};
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return OperationResult{{}, {}, {}, true};
    }

    static constexpr std::array<DataConversion::Indentation, 4> INDENTATIONS = {
        DataConversion::Indentation::SPACES_2,
        DataConversion::Indentation::SPACES_4,
        DataConversion::Indentation::TABS,
        DataConversion::Indentation::MINIFIED,
    };
    progress(80, QStringLiteral("Formatting JSON"));
    JsonEmitter emitter;
    const BasicEmitter::EmitResult emitted =
        emitter.emitQString(parsed.data, INDENTATIONS.at(static_cast<std::size_t>(indentation)));
    if (!emitted.error.isEmpty()) {
        return OperationResult{{}, QStringLiteral("INVALID_INPUT"), emitted.error, false};
    }
    if (emitted.text.toUtf8().size() > MAX_OUTPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Formatted output is larger than 4 MiB."),
                               false};
    }
    return OperationResult{{{QStringLiteral("text"), emitted.text}}, {}, {}, false};
}

NativeBridge::OperationResult NativeBridge::readTextFile(const QString &path,
                                                         const CancellationToken &cancellation,
                                                         const ProgressCallback &progress)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
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
        const QByteArray chunk = file.read(IO_CHUNK_BYTES);
        if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
            return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
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
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
    }
    qint64 writtenBytes = 0;
    while (writtenBytes < bytes.size()) {
        if (cancellation->load(std::memory_order_relaxed)) {
            file.cancelWriting();
            return OperationResult{{}, {}, {}, true};
        }
        const qint64 remainingBytes = bytes.size() - writtenBytes;
        const qint64 chunkSize = std::min<qint64>(IO_CHUNK_BYTES, remainingBytes);
        const qint64 written = file.write(bytes.constData() + writtenBytes, chunkSize);
        if (written <= 0) {
            file.cancelWriting();
            return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
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
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
    }

    return OperationResult{{{QStringLiteral("fileName"), QFileInfo(path).fileName()},
                            {QStringLiteral("bytesWritten"), writtenBytes}},
                           {},
                           {},
                           false};
}

bool NativeBridge::isValidRequestId(const QString &requestId)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9_-]{1,80}$"));
    return pattern.match(requestId).hasMatch();
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
