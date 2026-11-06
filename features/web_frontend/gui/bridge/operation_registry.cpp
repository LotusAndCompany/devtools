#include "operation_registry.h"

#include <array>

namespace {
struct OperationEntry
{
    const char *name;
    OperationHandler handler;
};

constexpr std::array OPERATIONS = {
    OperationEntry{"formatJson", OperationHandler::JsonFormat},
    OperationEntry{"convertData", OperationHandler::Tool},
    OperationEntry{"convertTimestamp", OperationHandler::Tool},
    OperationEntry{"compareText", OperationHandler::Tool},
    OperationEntry{"renderMarkdown", OperationHandler::Tool},
    OperationEntry{"generateQr", OperationHandler::Tool},
    OperationEntry{"commandData", OperationHandler::Tool},
    OperationEntry{"phraseData", OperationHandler::Tool},
    OperationEntry{"evaluateRegex", OperationHandler::Tool},
    OperationEntry{"sendHttpRequest", OperationHandler::Tool},
    OperationEntry{"imageOpen", OperationHandler::Image},
    OperationEntry{"imageEdit", OperationHandler::Image},
    OperationEntry{"imageReset", OperationHandler::Image},
    OperationEntry{"imageSave", OperationHandler::Image},
    OperationEntry{"imageSplit", OperationHandler::Image},
    OperationEntry{"imageTakeDropped", OperationHandler::Image},
    OperationEntry{"databaseHistory", OperationHandler::Database},
    OperationEntry{"databaseSelectFile", OperationHandler::Database},
    OperationEntry{"databaseConnect", OperationHandler::Database},
    OperationEntry{"databaseDisconnect", OperationHandler::Database},
    OperationEntry{"databaseQuery", OperationHandler::Database},
    OperationEntry{"databaseTables", OperationHandler::Database},
    OperationEntry{"saveQrImage", OperationHandler::QrImageSave},
    OperationEntry{"regexSettings", OperationHandler::RegexSettings},
    OperationEntry{"getSettings", OperationHandler::SettingsRead},
    OperationEntry{"getAboutInfo", OperationHandler::AboutInfo},
    OperationEntry{"saveSettings", OperationHandler::SaveSettings},
    OperationEntry{"saveLastUsedTool", OperationHandler::SaveLastUsedTool},
    OperationEntry{"copyToClipboard", OperationHandler::Clipboard},
    OperationEntry{"openTextFile", OperationHandler::FileDialog},
    OperationEntry{"saveTextFile", OperationHandler::FileDialog},
};
} // namespace

std::optional<OperationHandler> operationHandler(const QString &operation)
{
    for (const OperationEntry &entry : OPERATIONS) {
        if (operation == QLatin1String(entry.name)) {
            return entry.handler;
        }
    }
    return std::nullopt;
}
