#include "frontend_operation_service.h"

#include "features/command/core/command_service.h"
#include "features/data_conversion/core/data_conversion.h"
#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/query_service.h"
#include "features/diff_tool/core/diff_tool.h"
#include "features/http_request/core/http_service.h"
#include "features/markdown_preview/core/markdown_service.h"
#include "features/phrase_generation/core/template_reader.h"
#include "features/phrase_generation/core/template_repository.h"
#include "features/qr_code/core/content_generator.h"
#include "features/qr_code/core/qr_image.h"
#include "features/qr_code/core/qr_validation.h"
#include "features/regex_tool/core/regex_tool.h"
#include "features/timestamp_conversion/core/timestamp_conversion.h"

#include <QBuffer>
#include <QDateTime>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include <algorithm>
#include <utility>

namespace {
FrontendOperationService::Result invalidInput(const QString &message)
{
    return {{}, QStringLiteral("INVALID_INPUT"), message, false};
}

bool readText(const QVariantMap &payload, const QString &key, QString *value)
{
    const QVariant field = payload.value(key);
    if (field.metaType().id() != QMetaType::QString) {
        return false;
    }
    *value = field.toString();
    return value->toUtf8().size() <= 1024 * 1024;
}

QString qrImageData(const QImage &image)
{
    QImage preview = image;
    while (true) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        if (!buffer.open(QIODevice::WriteOnly) || !preview.save(&buffer, "PNG")) {
            return {};
        }
        if (bytes.size() <= 1024 * 1024) {
            return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
        }
        if (preview.width() <= 128 || preview.height() <= 128) {
            return {};
        }
        preview = preview.scaled(preview.size() / 2, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
}

FrontendOperationService::Result convertData(const QVariantMap &payload)
{
    QString input;
    if (!readText(payload, QStringLiteral("input"), &input)) {
        return invalidInput(QStringLiteral("Input must be a UTF-8 string no larger than 1 MiB."));
    }

    DataConversion::Format format = DataConversion::Format::JSON;
    const QString formatName = payload.value(QStringLiteral("format")).toString();
    if (formatName == QStringLiteral("yaml-block")) {
        format = DataConversion::Format::YAML_BLOCK;
    } else if (formatName == QStringLiteral("yaml-flow")) {
        format = DataConversion::Format::YAML_FLOW;
    } else if (formatName == QStringLiteral("toml")) {
        format = DataConversion::Format::TOML;
    } else if (formatName != QStringLiteral("json")) {
        return invalidInput(QStringLiteral("Output format must be JSON, YAML or TOML."));
    }

    DataConversion::Indentation indentation = DataConversion::Indentation::SPACES_2;
    const QString indentationName = payload.value(QStringLiteral("indentation")).toString();
    if (indentationName == QStringLiteral("4")) {
        indentation = DataConversion::Indentation::SPACES_4;
    } else if (indentationName == QStringLiteral("tab")) {
        indentation = DataConversion::Indentation::TABS;
    } else if (indentationName == QStringLiteral("minified")) {
        indentation = DataConversion::Indentation::MINIFIED;
    } else if (indentationName != QStringLiteral("2")) {
        return invalidInput(QStringLiteral("Indentation must be 2, 4, tab or minified."));
    }

    DataConversion conversion;
    conversion.setInputText(input);
    if (conversion.messages() == DataConversion::tr("Cannot parse input text")) {
        return invalidInput(conversion.messages());
    }
    conversion.setOutputFormat(format);
    conversion.setIndentation(indentation);
    conversion.updateOutputText();
    if (conversion.messages().contains(QStringLiteral("error"), Qt::CaseInsensitive) &&
        conversion.outputText().isEmpty()) {
        return invalidInput(conversion.messages());
    }
    if (conversion.outputText().toUtf8().size() > 4 * 1024 * 1024) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("Converted output is larger than 4 MiB."),
                false};
    }
    return {{{QStringLiteral("output"), conversion.outputText()},
             {QStringLiteral("message"), conversion.messages()}},
            {},
            {},
            false};
}

FrontendOperationService::Result convertTimestamp(const QVariantMap &payload)
{
    QString value;
    if (!readText(payload, QStringLiteral("value"), &value)) {
        return invalidInput(QStringLiteral("Timestamp input must be a UTF-8 string."));
    }
    const QString source = payload.value(QStringLiteral("source")).toString();
    QDateTime instant;
    if (source == QStringLiteral("seconds")) {
        instant = TimestampConversion::fromUnixTimestamp(value, TimestampConversion::Unit::Seconds);
    } else if (source == QStringLiteral("milliseconds")) {
        instant =
            TimestampConversion::fromUnixTimestamp(value, TimestampConversion::Unit::Milliseconds);
    } else if (source == QStringLiteral("iso")) {
        instant = TimestampConversion::fromIso8601(value);
    } else {
        return invalidInput(QStringLiteral("Timestamp source is not supported."));
    }
    if (!instant.isValid()) {
        return invalidInput(QStringLiteral("Enter a valid timestamp or ISO-8601 date."));
    }
    return {
        {{QStringLiteral("seconds"),
          TimestampConversion::toUnixTimestamp(instant, TimestampConversion::Unit::Seconds)},
         {QStringLiteral("milliseconds"),
          TimestampConversion::toUnixTimestamp(instant, TimestampConversion::Unit::Milliseconds)},
         {QStringLiteral("utc"), TimestampConversion::toIso8601(instant)},
         {QStringLiteral("local"), TimestampConversion::toIso8601Local(instant)}},
        {},
        {},
        false};
}

FrontendOperationService::Result compareText(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString left;
    QString right;
    if (!readText(payload, QStringLiteral("left"), &left) ||
        !readText(payload, QStringLiteral("right"), &right)) {
        return invalidInput(
            QStringLiteral("Both texts must be UTF-8 strings no larger than 1 MiB."));
    }
    if (DiffTool::splitLines(left).size() > 2000 || DiffTool::splitLines(right).size() > 2000) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("Diff is limited to 2,000 lines per side."),
                false};
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    progress(20, QStringLiteral("Comparing text"));
    const QVector<SideBySideRow> rows = DiffTool::groupSideBySide(DiffTool::diff(left, right));
    QVariantList output;
    output.reserve(rows.size());
    for (const SideBySideRow &row : rows) {
        if (cancellation->load(std::memory_order_relaxed)) {
            return {{}, {}, {}, true};
        }
        output.append(QVariantMap{{QStringLiteral("left"), row.left_text},
                                  {QStringLiteral("right"), row.right_text},
                                  {QStringLiteral("leftChanged"), row.left_changed},
                                  {QStringLiteral("rightChanged"), row.right_changed}});
    }
    return {{{QStringLiteral("rows"), output}}, {}, {}, false};
}

FrontendOperationService::Result generateQr(const QVariantMap &payload)
{
    const QString typeName = payload.value(QStringLiteral("type")).toString();
    static const QStringList names = {
        QStringLiteral("text"),    QStringLiteral("url"),      QStringLiteral("email"),
        QStringLiteral("phone"),   QStringLiteral("sms"),      QStringLiteral("wifi"),
        QStringLiteral("contact"), QStringLiteral("calendar"), QStringLiteral("geo")};
    const int typeIndex = names.indexOf(typeName);
    if (typeIndex < 0) {
        return invalidInput(QStringLiteral("QR code type is not supported."));
    }
    const QVariantMap params = payload.value(QStringLiteral("parameters")).toMap();
    if (!QRValidation::canGenerate(static_cast<QRCodeType>(typeIndex), params)) {
        return invalidInput(QStringLiteral("Complete the required QR code fields."));
    }

    QVariantMap validatedParams = params;
    if (static_cast<QRCodeType>(typeIndex) == QRCodeType::Calendar) {
        for (const QString &key : {QStringLiteral("cal_start"), QStringLiteral("cal_end")}) {
            const QDateTime dateTime =
                QDateTime::fromString(params.value(key).toString(), Qt::ISODate);
            if (!dateTime.isValid()) {
                return invalidInput(QStringLiteral("Enter valid local calendar dates and times."));
            }
            validatedParams.insert(key, dateTime);
        }
    }
    const QList<QRValidationError> validationErrors =
        QRValidation::validate(static_cast<QRCodeType>(typeIndex), validatedParams);
    if (!validationErrors.isEmpty()) {
        return invalidInput(
            QStringLiteral("Invalid QR code field: %1.").arg(validationErrors.constFirst().field));
    }

    // QRValidation uses the current UI field names, while ContentGenerator
    // retains the legacy service keys. Keep that boundary conversion here.
    QVariantMap generatorParams;
    const auto copyField = [&params, &generatorParams](const char *source, const char *target) {
        const QString sourceKey = QString::fromLatin1(source);
        if (params.contains(sourceKey)) {
            generatorParams.insert(QString::fromLatin1(target), params.value(sourceKey));
        }
    };
    switch (static_cast<QRCodeType>(typeIndex)) {
    case QRCodeType::Text:
        copyField("text_content", "text");
        break;
    case QRCodeType::Url:
        copyField("url_content", "url");
        break;
    case QRCodeType::Email:
        copyField("email_address", "email");
        break;
    case QRCodeType::Phone:
        copyField("phone_number", "number");
        break;
    case QRCodeType::Sms:
        copyField("sms_phone", "number");
        copyField("sms_message", "text");
        break;
    case QRCodeType::Wifi:
        copyField("wifi_ssid", "ssid");
        copyField("wifi_password", "password");
        copyField("wifi_security", "type");
        copyField("wifi_hidden", "hidden");
        break;
    case QRCodeType::Contact:
        copyField("contact_name", "name");
        copyField("contact_phone", "phone");
        copyField("contact_email", "email");
        break;
    case QRCodeType::Calendar: {
        copyField("cal_summary", "summary");
        for (const auto &[source, target] :
             {std::pair{"cal_start", "start"}, std::pair{"cal_end", "end"}}) {
            const QDateTime dateTime = validatedParams.value(QLatin1String(source)).toDateTime();
            generatorParams.insert(QLatin1String(target), dateTime.toString("yyyyMMdd'T'hhmmss"));
        }
        break;
    }
    case QRCodeType::Geo:
        copyField("geo_lat", "lat");
        copyField("geo_lng", "lng");
        break;
    }

    const QString content = ContentGenerator::generate(typeName, generatorParams);
    if (content.isEmpty() || content.toUtf8().size() > 2953) {
        return invalidInput(QStringLiteral("QR code content is empty or exceeds its size limit."));
    }
    const QString image = qrImageData(QRImage::generate(content));
    if (image.isEmpty()) {
        return {{},
                QStringLiteral("INTERNAL_ERROR"),
                QStringLiteral("Could not create a QR preview."),
                false};
    }
    return {{{QStringLiteral("content"), content}, {QStringLiteral("imageDataUrl"), image}},
            {},
            {},
            false};
}

FrontendOperationService::Result commandData(const QVariantMap &payload)
{
    const QString categoryName = payload.value(QStringLiteral("category")).toString();
    CommandCategory category = CommandCategory::Unknown;
    if (categoryName == QStringLiteral("git")) {
        category = CommandCategory::Git;
    } else if (categoryName == QStringLiteral("docker")) {
        category = CommandCategory::Docker;
    } else if (categoryName == QStringLiteral("compose")) {
        category = CommandCategory::DockerCompose;
    } else {
        return invalidInput(QStringLiteral("Command category is not supported."));
    }
    const QList<CommandFunction> functions = CommandService::functions(category);
    if (payload.value(QStringLiteral("action")).toString() == QStringLiteral("list")) {
        QVariantList items;
        for (const CommandFunction &function : functions) {
            QVariantList options;
            for (const CommandOption &option : function.getOptions()) {
                options.append(QVariantMap{{QStringLiteral("name"), option.getName()},
                                           {QStringLiteral("required"), option.isRequired()},
                                           {QStringLiteral("title"), option.getTitle()},
                                           {QStringLiteral("quotes"), option.isRequiredQuotes()}});
            }
            items.append(QVariantMap{{QStringLiteral("name"), function.getName()},
                                     {QStringLiteral("description"), function.getDescription()},
                                     {QStringLiteral("options"), options}});
        }
        return {{{QStringLiteral("functions"), items}}, {}, {}, false};
    }
    bool functionIndexValid = false;
    bool optionIndexValid = false;
    const int functionIndex =
        payload.value(QStringLiteral("functionIndex")).toInt(&functionIndexValid);
    const int optionIndex = payload.value(QStringLiteral("optionIndex")).toInt(&optionIndexValid);
    if (!functionIndexValid || !optionIndexValid) {
        return invalidInput(QStringLiteral("Select a valid command and option."));
    }
    QString value;
    if (!readText(payload, QStringLiteral("value"), &value)) {
        return invalidInput(QStringLiteral("Command value must be a UTF-8 string."));
    }
    const CommandGenerationResult result =
        CommandService::generate(category, functionIndex, optionIndex, value);
    if (result.error != CommandGenerationError::None) {
        return invalidInput(result.error == CommandGenerationError::InvalidValue
                                ? QStringLiteral("Quotes are not allowed in command values.")
                                : QStringLiteral("Select a command and option."));
    }
    return {{{QStringLiteral("command"), result.command}}, {}, {}, false};
}

bool validTemplateFilename(const QString &filename)
{
    static const QRegularExpression pattern(QStringLiteral(
        "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.txt$"));
    return pattern.match(filename).hasMatch();
}

FrontendOperationService::Result phraseData(const QVariantMap &payload)
{
    const QString action = payload.value(QStringLiteral("action")).toString();
    if (action == QStringLiteral("list")) {
        QVariantList templates;
        const QStringList filenames = TemplateReader::filenames();
        templates.reserve(filenames.size());
        for (const QString &filename : filenames) {
            QString title;
            const QString content = TemplateReader::load(filename, &title);
            templates.append(QVariantMap{{QStringLiteral("filename"), filename},
                                         {QStringLiteral("title"), title},
                                         {QStringLiteral("content"), content}});
        }
        return {{{QStringLiteral("templates"), templates}}, {}, {}, false};
    }
    if (action == QStringLiteral("save")) {
        QString title;
        QString content;
        if (!readText(payload, QStringLiteral("title"), &title) ||
            !readText(payload, QStringLiteral("content"), &content)) {
            return invalidInput(
                QStringLiteral("Template title and content must be strings no larger than 1 MiB."));
        }
        const QString filename = payload.value(QStringLiteral("filename")).toString();
        if (!filename.isEmpty() && !validTemplateFilename(filename)) {
            return invalidInput(QStringLiteral("Template identifier is invalid."));
        }
        const TemplateSaveResult saved = TemplateRepository::save(title, content, filename);
        if (saved.error != TemplateValidationError::None || !saved.written) {
            return invalidInput(saved.error == TemplateValidationError::EmptyTitle
                                    ? QStringLiteral("Template title is required.")
                                : saved.error == TemplateValidationError::EmptyContent
                                    ? QStringLiteral("Template content is required.")
                                    : QStringLiteral("Could not save the template."));
        }
        return {{{QStringLiteral("filename"), saved.filename}}, {}, {}, false};
    }
    if (action == QStringLiteral("delete")) {
        const QString filename = payload.value(QStringLiteral("filename")).toString();
        if (!validTemplateFilename(filename) || !TemplateRepository::remove(filename)) {
            return invalidInput(QStringLiteral("Could not delete the selected template."));
        }
        return {{{QStringLiteral("deleted"), true}}, {}, {}, false};
    }
    return invalidInput(QStringLiteral("Template action is not supported."));
}

FrontendOperationService::Result evaluateRegex(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString pattern;
    QString input;
    QString replacement;
    if (!readText(payload, QStringLiteral("pattern"), &pattern) ||
        !readText(payload, QStringLiteral("text"), &input) ||
        !readText(payload, QStringLiteral("replacement"), &replacement)) {
        return invalidInput(QStringLiteral(
            "Pattern, text and replacement must be UTF-8 strings no larger than 1 MiB."));
    }
    if (pattern.isEmpty()) {
        return {
            {{QStringLiteral("matches"), QVariantList{}}, {QStringLiteral("replacedText"), input}},
            {},
            {},
            false};
    }
    const auto options = devtools::RegexTool::patternOptions(
        payload.value(QStringLiteral("caseInsensitive")).toBool(),
        payload.value(QStringLiteral("multiline")).toBool(),
        payload.value(QStringLiteral("dotMatchesEverything")).toBool(),
        payload.value(QStringLiteral("useUnicodeProperties")).toBool(),
        payload.value(QStringLiteral("extendedPatternSyntax")).toBool());
    const bool global = !payload.contains(QStringLiteral("global")) ||
                        payload.value(QStringLiteral("global")).toBool();
    const QString limitedPattern = QStringLiteral("(*LIMIT_MATCH=1000000)") + pattern;
    const QRegularExpression regex(limitedPattern, options);
    if (!regex.isValid()) {
        return invalidInput(regex.errorString());
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }

    progress(15, QStringLiteral("Matching regular expression"));
    QVariantList matches;
    const QStringList captureNames = regex.namedCaptureGroups();
    auto iterator = global ? regex.globalMatch(input) : QRegularExpressionMatchIterator{};
    int index = 0;
    if (global) {
        while (iterator.hasNext()) {
            if (cancellation->load(std::memory_order_relaxed)) {
                return {{}, {}, {}, true};
            }
            const QRegularExpressionMatch match = iterator.next();
            QVariantList groups;
            for (int groupIndex = 0; groupIndex <= match.lastCapturedIndex(); ++groupIndex) {
                groups.append(
                    QVariantMap{{QStringLiteral("index"), groupIndex},
                                {QStringLiteral("name"), captureNames.value(groupIndex)},
                                {QStringLiteral("value"), match.captured(groupIndex)},
                                {QStringLiteral("offset"), match.capturedStart(groupIndex)},
                                {QStringLiteral("length"), match.capturedLength(groupIndex)}});
            }
            matches.append(QVariantMap{{QStringLiteral("index"), index++},
                                       {QStringLiteral("offset"), match.capturedStart()},
                                       {QStringLiteral("length"), match.capturedLength()},
                                       {QStringLiteral("value"), match.captured(0)},
                                       {QStringLiteral("groups"), groups}});
            if (matches.size() >= 10000) {
                return {{},
                        QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                        QStringLiteral("Regex results are limited to 10,000 matches."),
                        false};
            }
        }
        if (!iterator.isValid()) {
            return invalidInput(QStringLiteral("Regular expression match limit exceeded."));
        }
    } else {
        const QRegularExpressionMatch match = regex.match(input);
        if (match.hasMatch()) {
            QVariantList groups;
            for (int groupIndex = 0; groupIndex <= match.lastCapturedIndex(); ++groupIndex) {
                groups.append(
                    QVariantMap{{QStringLiteral("index"), groupIndex},
                                {QStringLiteral("name"), captureNames.value(groupIndex)},
                                {QStringLiteral("value"), match.captured(groupIndex)},
                                {QStringLiteral("offset"), match.capturedStart(groupIndex)},
                                {QStringLiteral("length"), match.capturedLength(groupIndex)}});
            }
            matches.append(QVariantMap{{QStringLiteral("index"), 0},
                                       {QStringLiteral("offset"), match.capturedStart()},
                                       {QStringLiteral("length"), match.capturedLength()},
                                       {QStringLiteral("value"), match.captured(0)},
                                       {QStringLiteral("groups"), groups}});
        }
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    const QString replaced =
        devtools::RegexTool::replace(limitedPattern, input, replacement, options, global);
    progress(90, QStringLiteral("Building results"));
    return {{{QStringLiteral("matches"), matches}, {QStringLiteral("replacedText"), replaced}},
            {},
            {},
            false};
}

FrontendOperationService::Result sendHttpRequest(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString urlText;
    QString method;
    QString body;
    if (!readText(payload, QStringLiteral("url"), &urlText) ||
        !readText(payload, QStringLiteral("method"), &method) ||
        !readText(payload, QStringLiteral("body"), &body)) {
        return invalidInput(
            QStringLiteral("URL, method and body must be strings no larger than 1 MiB."));
    }
    static const QStringList methods = {QStringLiteral("GET"), QStringLiteral("POST"),
                                        QStringLiteral("PUT"), QStringLiteral("DELETE")};
    const QVariantList parameterValues = payload.value(QStringLiteral("parameters")).toList();
    if (parameterValues.size() > 10) {
        return invalidInput(QStringLiteral("At most 10 query parameters are supported."));
    }
    QList<QPair<QString, QString>> parameters;
    for (const QVariant &value : parameterValues) {
        const QVariantMap parameter = value.toMap();
        if (parameter.value(QStringLiteral("key")).metaType().id() != QMetaType::QString ||
            parameter.value(QStringLiteral("value")).metaType().id() != QMetaType::QString ||
            parameter.value(QStringLiteral("description")).metaType().id() != QMetaType::QString) {
            return invalidInput(QStringLiteral("Query parameters must contain string fields."));
        }
        parameters.append({parameter.value(QStringLiteral("key")).toString(),
                           parameter.value(QStringLiteral("value")).toString()});
    }
    if (!parameters.isEmpty()) {
        urlText = HttpService::urlWithParameters(urlText, parameters);
    }
    const QUrl url(urlText);
    if (!methods.contains(method) || !url.isValid() ||
        (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https")) ||
        url.host().isEmpty()) {
        return invalidInput(QStringLiteral("Enter a valid HTTP or HTTPS URL and method."));
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setTransferTimeout(30'000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QVariantMap headers = payload.value(QStringLiteral("headers")).toMap();
    for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
        const QString key = it.key().trimmed();
        const QString value = it.value().toString();
        static const QRegularExpression headerName(QStringLiteral("^[!#$%&'*+.^_`|~0-9A-Za-z-]+$"));
        if (!headerName.match(key).hasMatch() || value.contains(QLatin1Char('\r')) ||
            value.contains(QLatin1Char('\n')) ||
            key.compare(QStringLiteral("Host"), Qt::CaseInsensitive) == 0 ||
            key.compare(QStringLiteral("Content-Length"), Qt::CaseInsensitive) == 0) {
            return invalidInput(QStringLiteral("A request header is invalid or restricted."));
        }
        request.setRawHeader(key.toUtf8(), value.toUtf8());
    }
    const QString username = payload.value(QStringLiteral("username")).toString();
    const QString password = payload.value(QStringLiteral("password")).toString();
    if (!username.isEmpty() && !password.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("Authorization"),
                             QByteArrayLiteral("Basic ") +
                                 (username + QLatin1Char(':') + password).toUtf8().toBase64());
    }
    QElapsedTimer elapsed;
    elapsed.start();
    progress(5, QStringLiteral("Sending HTTP request"));
    QNetworkReply *reply = nullptr;
    if (method == QStringLiteral("GET")) {
        reply = manager.get(request);
    } else if (method == QStringLiteral("POST")) {
        reply = manager.post(request, body.toUtf8());
    } else if (method == QStringLiteral("PUT")) {
        reply = manager.put(request, body.toUtf8());
    } else {
        reply = manager.deleteResource(request);
    }
    QByteArray responseBody;
    bool tooLarge = false;
    QObject::connect(reply, &QIODevice::readyRead, reply, [&]() {
        responseBody.append(reply->readAll());
        if (responseBody.size() > 4 * 1024 * 1024) {
            tooLarge = true;
            reply->abort();
        }
    });
    QEventLoop loop;
    QTimer cancellationTimer;
    cancellationTimer.setInterval(50);
    QObject::connect(&cancellationTimer, &QTimer::timeout, &loop, [&]() {
        if (cancellation->load(std::memory_order_relaxed)) {
            reply->abort();
        }
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    cancellationTimer.start();
    loop.exec();
    responseBody.append(reply->readAll());
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    const QString error = reply->errorString();
    const int errorCode = static_cast<int>(reply->error());
    const qint64 elapsedMs = elapsed.elapsed();
    reply->deleteLater();
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    if (tooLarge || responseBody.size() > 4 * 1024 * 1024) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("HTTP response body is limited to 4 MiB."),
                false};
    }
    progress(95, QStringLiteral("Preparing HTTP response"));
    return {{{QStringLiteral("status"), status},
             {QStringLiteral("reason"), reason},
             {QStringLiteral("elapsedMs"), elapsedMs},
             {QStringLiteral("errorCode"), errorCode},
             {QStringLiteral("error"), error},
             {QStringLiteral("body"), QString::fromUtf8(responseBody)},
             {QStringLiteral("bytes"), responseBody.size()}},
            {},
            {},
            false};
}

FrontendOperationService::Result withDatabase(
    const QVariantMap &payload,
    const std::function<FrontendOperationService::Result(const QSqlDatabase &)> &callback)
{
    const QVariantMap parameters = payload.value(QStringLiteral("connection")).toMap();
    const QString type = parameters.value(QStringLiteral("type")).toString();
    if (type != QStringLiteral("QSQLITE") && type != QStringLiteral("QMYSQL") &&
        type != QStringLiteral("QPSQL")) {
        return invalidInput(QStringLiteral("Choose SQLite, MySQL or PostgreSQL."));
    }
    const DatabaseParameters dbParameters{type, parameters.value(QStringLiteral("host")).toString(),
                                          parameters.value(QStringLiteral("database")).toString(),
                                          parameters.value(QStringLiteral("username")).toString(),
                                          parameters.value(QStringLiteral("password")).toString()};
    if (dbParameters.database.isEmpty() ||
        (type != QStringLiteral("QSQLITE") &&
         (dbParameters.host.isEmpty() || dbParameters.username.isEmpty()))) {
        return invalidInput(QStringLiteral("Complete the database connection fields."));
    }
    const QString connectionName =
        QStringLiteral("react-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QSqlDatabase database = DatabaseConnection::open(dbParameters, connectionName);
    if (!database.isOpen()) {
        const QString message = database.lastError().text();
        database = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return {{}, QStringLiteral("IO_ERROR"), message, false};
    }
    FrontendOperationService::Result result = callback(database);
    database.close();
    database = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
    return result;
}

FrontendOperationService::Result databaseConnect(const QVariantMap &payload)
{
    QVariantMap connection = payload;
    connection.remove(QStringLiteral("action"));
    connection.remove(QStringLiteral("contractVersion"));
    connection.remove(QStringLiteral("requestId"));
    const QVariantMap wrapper{{QStringLiteral("connection"), connection}};
    return withDatabase(
        wrapper, [&connection](const QSqlDatabase &database) -> FrontendOperationService::Result {
            const QString displayName = connection.value(QStringLiteral("displayName")).toString();
            const QStringList tables = database.tables();
            return {{{QStringLiteral("connected"), true},
                     {QStringLiteral("displayName"), displayName},
                     {QStringLiteral("tables"), QVariant::fromValue(tables)}},
                    {},
                    {},
                    false};
        });
}

FrontendOperationService::Result databaseTables(const QVariantMap &payload)
{
    return withDatabase(payload,
                        [](const QSqlDatabase &database) -> FrontendOperationService::Result {
                            QVariantList tables;
                            for (const QString &table : database.tables()) {
                                tables.append(table);
                            }
                            return {{{QStringLiteral("tables"), tables}}, {}, {}, false};
                        });
}

FrontendOperationService::Result databaseQuery(const QVariantMap &payload)
{
    QString sql;
    if (!readText(payload, QStringLiteral("sql"), &sql) || sql.trimmed().isEmpty()) {
        return invalidInput(QStringLiteral("Enter SQL no larger than 1 MiB."));
    }
    return withDatabase(payload, [&sql](const QSqlDatabase &database) {
        QueryExecution execution = QueryService::execute(database, sql);
        if (!execution.success) {
            return FrontendOperationService::Result{
                {}, QStringLiteral("INVALID_INPUT"), execution.error.text(), false};
        }
        QVariantList columns;
        const QSqlRecord record = execution.query.record();
        for (int column = 0; column < record.count(); ++column) {
            columns.append(record.fieldName(column));
        }
        QVariantList rows;
        qsizetype totalBytes = 0;
        bool truncated = false;
        constexpr int MAX_ROWS = 1000;
        constexpr qsizetype MAX_RESULT_BYTES = 4 * 1024 * 1024;
        while (execution.query.next()) {
            if (rows.size() >= MAX_ROWS) {
                truncated = true;
                break;
            }
            QVariantList row;
            row.reserve(record.count());
            for (int column = 0; column < record.count(); ++column) {
                const QVariant value = execution.query.value(column);
                if (!value.isValid() || value.isNull()) {
                    row.append(QVariant());
                    continue;
                }
                const QString text = value.toString();
                totalBytes += text.toUtf8().size();
                if (text.size() > 64 * 1024 || totalBytes > MAX_RESULT_BYTES) {
                    return FrontendOperationService::Result{
                        {},
                        QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                        QStringLiteral(
                            "Database results are limited to 4 MiB; reduce the query result."),
                        false};
                }
                row.append(text);
            }
            rows.append(row);
        }
        return FrontendOperationService::Result{
            {{QStringLiteral("columns"), columns},
             {QStringLiteral("rows"), rows},
             {QStringLiteral("truncated"), truncated},
             {QStringLiteral("affectedRows"), execution.query.numRowsAffected()}},
            {},
            {},
            false};
    });
}
} // namespace

FrontendOperationService::Result FrontendOperationService::execute(
    const QString &operation, const QVariantMap &payload, const CancellationToken &cancellation,
    const ProgressCallback &progress)
{
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    if (operation == QStringLiteral("convertData")) {
        return convertData(payload);
    }
    if (operation == QStringLiteral("convertTimestamp")) {
        return convertTimestamp(payload);
    }
    if (operation == QStringLiteral("compareText")) {
        return compareText(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("renderMarkdown")) {
        QString markdown;
        if (!readText(payload, QStringLiteral("markdown"), &markdown)) {
            return invalidInput(
                QStringLiteral("Markdown must be a UTF-8 string no larger than 1 MiB."));
        }
        return {{{QStringLiteral("html"), MarkdownService::renderHtml(markdown)}}, {}, {}, false};
    }
    if (operation == QStringLiteral("generateQr")) {
        return generateQr(payload);
    }
    if (operation == QStringLiteral("commandData")) {
        return commandData(payload);
    }
    if (operation == QStringLiteral("phraseData")) {
        return phraseData(payload);
    }
    if (operation == QStringLiteral("evaluateRegex")) {
        return evaluateRegex(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("sendHttpRequest")) {
        return sendHttpRequest(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("databaseConnect")) {
        return databaseConnect(payload);
    }
    if (operation == QStringLiteral("databaseTables")) {
        return databaseTables(payload);
    }
    if (operation == QStringLiteral("databaseQuery")) {
        return databaseQuery(payload);
    }
    return invalidInput(QStringLiteral("The requested tool operation is not available."));
}
