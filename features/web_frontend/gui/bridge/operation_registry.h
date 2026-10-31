#ifndef OPERATION_REGISTRY_H
#define OPERATION_REGISTRY_H

#include <QString>

#include <optional>

enum class OperationHandler {
    JsonFormat,
    Tool,
    Image,
    Database,
    QrImageSave,
    RegexSettings,
    SettingsRead,
    AboutInfo,
    SaveSettings,
    SaveLastUsedTool,
    Clipboard,
    FileDialog,
};

[[nodiscard]] std::optional<OperationHandler> operationHandler(const QString &operation);

#endif // OPERATION_REGISTRY_H
