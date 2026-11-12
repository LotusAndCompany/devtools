import type { OperationName, OperationOutput } from "@/shared/bridge/types";

type Validator = (value: unknown) => boolean;

function isRecord(value: unknown): value is Record<string, unknown> {
    return typeof value === "object" && value !== null && !Array.isArray(value);
}

const string: Validator = (value) => typeof value === "string";
const number: Validator = (value) => typeof value === "number" && Number.isFinite(value);
const boolean: Validator = (value) => typeof value === "boolean";
const nullValue: Validator = (value) => value === null;
const literal =
    (expected: string | number | boolean): Validator =>
    (value) =>
        value === expected;
const arrayOf =
    (item: Validator): Validator =>
    (value) =>
        Array.isArray(value) && Array.from(value).every(item);
const union =
    (...validators: Validator[]): Validator =>
    (value) =>
        validators.some((validate) => validate(value));
const object =
    (required: Record<string, Validator>, optional: Record<string, Validator> = {}): Validator =>
    (value) =>
        isRecord(value) &&
        Object.entries(required).every(
            ([key, validate]) => Object.hasOwn(value, key) && validate(value[key]),
        ) &&
        Object.entries(optional).every(
            ([key, validate]) => !Object.hasOwn(value, key) || validate(value[key]),
        );

const databaseDriver = union(literal("sqlite"), literal("mysql"), literal("postgres"));
const preview = object({ imageDataUrl: string, width: number, height: number });
const regexOptions = object({
    global: boolean,
    caseInsensitive: boolean,
    multiline: boolean,
    dotMatchesEverything: boolean,
    useUnicodeProperties: boolean,
    extendedPatternSyntax: boolean,
});
const regexSettings = object({
    pattern: string,
    testText: string,
    replacePattern: string,
    options: regexOptions,
});
const commandOption = object({ name: string, required: boolean, title: string, quotes: boolean });
const commandFunction = object({
    name: string,
    description: string,
    options: arrayOf(commandOption),
});
const regexCapture = object({
    index: number,
    name: string,
    value: string,
    offset: number,
    length: number,
});
const regexMatch = object({
    index: number,
    offset: number,
    length: number,
    value: string,
    groups: arrayOf(regexCapture),
});

const outputValidators = {
    formatJson: object({ text: string }),
    openTextFile: object({ text: string, fileName: string }),
    saveTextFile: object({ fileName: string, bytesWritten: number }),
    convertData: object({ output: string, message: string }),
    convertTimestamp: object({ seconds: string, milliseconds: string, utc: string, local: string }),
    compareText: object({
        rows: arrayOf(
            object({ left: string, right: string, leftChanged: boolean, rightChanged: boolean }),
        ),
    }),
    renderMarkdown: object({ html: string }),
    generateQr: object({ content: string, imageDataUrl: string }),
    saveQrImage: object({ fileName: string }),
    commandData: union(
        object({ functions: arrayOf(commandFunction) }),
        object({ command: string }),
    ),
    getSettings: object(
        {
            language: union(literal("en"), literal("ja_JP")),
            theme: union(literal("system"), literal("light"), literal("dark")),
            showSidebarOnStartup: boolean,
            showLastToolOnStartup: boolean,
            alwaysOnTop: boolean,
            rememberWindowSize: boolean,
            rememberWindowPosition: boolean,
        },
        {
            lastUsedTool: number,
            windowWidth: number,
            windowHeight: number,
            windowX: number,
            windowY: number,
        },
    ),
    saveSettings: object({ saved: literal(true) }),
    copyToClipboard: object({ copied: literal(true) }),
    getAboutInfo: object({
        appVersion: string,
        revision: string,
        buildEnvironment: string,
        runtime: string,
        copyright: string,
        licenseText: string,
    }),
    phraseData: union(
        object({
            templates: arrayOf(object({ filename: string, title: string, content: string })),
        }),
        object({ filename: string }),
        object({ deleted: literal(true) }),
    ),
    evaluateRegex: object({ matches: arrayOf(regexMatch), replacedText: string }),
    regexSettings: union(regexSettings, object({ saved: literal(true) })),
    sendHttpRequest: object({
        status: number,
        reason: string,
        elapsedMs: number,
        errorCode: number,
        error: string,
        body: string,
        bytes: number,
    }),
    imageOpen: object({ imageDataUrl: string, width: number, height: number, fileName: string }),
    imageEdit: preview,
    imageReset: preview,
    imageSave: object({ fileName: string }),
    imageSplit: object({ saved: literal(true) }),
    imageTakeDropped: union(
        object({ found: literal(false) }),
        object({
            found: literal(true),
            imageDataUrl: string,
            width: number,
            height: number,
            fileName: string,
        }),
    ),
    databaseHistory: object({
        history: arrayOf(
            object({
                index: number,
                type: databaseDriver,
                host: string,
                database: string,
                username: string,
                displayName: string,
            }),
        ),
        selectedFileName: string,
        connected: boolean,
        connectedType: union(databaseDriver, literal("")),
    }),
    databaseSelectFile: object({ selected: literal(true), fileName: string }),
    databaseConnect: object({
        connected: literal(true),
        displayName: string,
        tables: arrayOf(string),
    }),
    databaseDisconnect: object({ disconnected: literal(true) }),
    databaseQuery: object({
        columns: arrayOf(string),
        rows: arrayOf(arrayOf(union(string, nullValue))),
        truncated: boolean,
        affectedRows: number,
    }),
    databaseTables: object({ tables: arrayOf(string) }),
    saveLastUsedTool: object({ saved: literal(true) }),
} satisfies Record<OperationName, Validator>;

export function isOperationOutput<Name extends OperationName>(
    operation: Name,
    value: unknown,
): value is OperationOutput<Name> {
    return outputValidators[operation](value);
}
