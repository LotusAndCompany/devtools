import type { Effect } from "effect";

export const BRIDGE_CONTRACT_VERSION = "2" as const;

export type EmptyOperationInput = Record<string, never>;
export type JsonIndentation = "2" | "4" | "tab" | "minified";
export type DataFormat = "json" | "yaml-block" | "yaml-flow" | "toml";
export type TimestampSource = "seconds" | "milliseconds" | "iso";
export type QrCodeType =
  | "text"
  | "url"
  | "email"
  | "phone"
  | "sms"
  | "wifi"
  | "contact"
  | "calendar"
  | "geo";
export type DatabaseDriver = "QSQLITE" | "QMYSQL" | "QPSQL";
export type HttpMethod = "GET" | "POST" | "PUT" | "DELETE";
export type TextFileType = "text" | "data" | "markdown" | "html";

export type AboutInfo = {
  appVersion: string;
  revision: string;
  buildEnvironment: string;
  qtVersion: string;
  copyright: string;
  licenseText: string;
};

export type ApplicationSettings = {
  language: "en" | "ja_JP";
  theme: "system" | "light" | "dark";
  showSidebarOnStartup: boolean;
  showLastToolOnStartup: boolean;
  lastUsedTool?: number;
  alwaysOnTop: boolean;
  rememberWindowSize: boolean;
  rememberWindowPosition: boolean;
};

export type RegexOptions = {
  global: boolean;
  caseInsensitive: boolean;
  multiline: boolean;
  dotMatchesEverything: boolean;
  useUnicodeProperties: boolean;
  extendedPatternSyntax: boolean;
};

export type RegexSettings = {
  pattern: string;
  testText: string;
  replacePattern: string;
  options: RegexOptions;
};

export type PhraseTemplate = {
  filename: string;
  title: string;
  content: string;
};

export type CommandOption = {
  name: string;
  required: boolean;
  title: string;
  quotes: boolean;
};

export type CommandFunction = {
  name: string;
  description: string;
  options: CommandOption[];
};

export type DatabaseHistoryEntry = {
  index: number;
  type: DatabaseDriver;
  host: string;
  database: string;
  username: string;
  displayName: string;
};

export type DatabaseConnectionInput =
  | { historyIndex: number; password: string }
  | {
      type: "QSQLITE";
      host?: string;
      database?: string;
      username?: string;
      password?: string;
    }
  | {
      type: "QMYSQL" | "QPSQL";
      host: string;
      database: string;
      username: string;
      password: string;
    };

export type ImageEditInput =
  | { action: "rotate-left" | "rotate-right" | "flip-horizontal" | "flip-vertical" }
  | {
      action: "resize-size";
      width: number;
      height: number;
      keepAspectRatio: boolean;
      smoothScaling: boolean;
    }
  | {
      action: "resize-scale";
      horizontalPercent: number;
      verticalPercent: number;
      keepAspectRatio: boolean;
      smoothScaling: boolean;
    }
  | {
      action: "transparent";
      x: number;
      y: number;
      transparency: number;
      tolerance: number;
      colorMode: "rgb" | "hsl" | "hsv";
      contiguousOnly: boolean;
    };

export type ImageSplitInput =
  | {
      mode: "division";
      horizontalDivisions: number;
      verticalDivisions: number;
      ignoreRemainders: boolean;
    }
  | {
      mode: "cell-size";
      cellWidth: number;
      cellHeight: number;
      ignoreRemainders: boolean;
    };

export type ImagePreviewResult = {
  imageDataUrl: string;
  width: number;
  height: number;
  fileName?: string;
};

export type ImageTakeDroppedResult =
  | { found: false }
  | ({ found: true } & ImagePreviewResult & { fileName: string });

export type RegexCapture = {
  index: number;
  name: string;
  value: string;
  offset: number;
  length: number;
};

export type RegexMatch = {
  index: number;
  offset: number;
  length: number;
  value: string;
  groups: RegexCapture[];
};

export type HttpParameter = { key: string; value: string; description: string };

export type HttpResponse = {
  status: number;
  reason: string;
  elapsedMs: number;
  errorCode: number;
  error: string;
  body: string;
  bytes: number;
};

export type OperationContracts = {
  formatJson: {
    input: { text: string; indentation: JsonIndentation };
    output: { text: string };
  };
  openTextFile: {
    input: { fileType?: TextFileType };
    output: { text: string; fileName: string };
  };
  saveTextFile: {
    input: { text: string; suggestedName?: string; fileType?: TextFileType };
    output: { fileName: string; bytesWritten: number };
  };
  convertData: {
    input: { input: string; format: DataFormat; indentation: JsonIndentation };
    output: { output: string; message: string };
  };
  convertTimestamp: {
    input: { value: string; source: TimestampSource };
    output: { seconds: string; milliseconds: string; utc: string; local: string };
  };
  compareText: {
    input: { left: string; right: string };
    output: {
      rows: Array<{ left: string; right: string; leftChanged: boolean; rightChanged: boolean }>;
    };
  };
  renderMarkdown: { input: { markdown: string }; output: { html: string } };
  generateQr: {
    input: { type: QrCodeType; parameters: Record<string, string> };
    output: { content: string; imageDataUrl: string };
  };
  saveQrImage: {
    input: { type: QrCodeType; parameters: Record<string, string> };
    output: { fileName: string };
  };
  commandData: {
    input:
      | { action: "list"; category: "git" | "docker" | "compose" }
      | {
          action: "generate";
          category: "git" | "docker" | "compose";
          functionIndex: number;
          optionIndex: number;
          value: string;
        };
    output: { functions: CommandFunction[] } | { command: string };
  };
  getSettings: { input: EmptyOperationInput; output: ApplicationSettings };
  saveSettings: { input: ApplicationSettings; output: { saved: true } };
  copyToClipboard: {
    input: { text: string } | { imageDataUrl: string };
    output: { copied: true };
  };
  getAboutInfo: { input: EmptyOperationInput; output: AboutInfo };
  phraseData: {
    input:
      | { action: "list" }
      | { action: "save"; filename: string; title: string; content: string }
      | { action: "delete"; filename: string };
    output: { templates: PhraseTemplate[] } | { filename: string } | { deleted: true };
  };
  evaluateRegex: {
    input: {
      pattern: string;
      text: string;
      replacement: string;
    } & RegexOptions;
    output: { matches: RegexMatch[]; replacedText: string };
  };
  regexSettings: {
    input: { action: "load" } | ({ action: "save" } & RegexSettings);
    output: RegexSettings | { saved: true };
  };
  sendHttpRequest: {
    input: {
      method: HttpMethod;
      url: string;
      headers: Record<string, string>;
      username: string;
      password: string;
      body: string;
      parameters: HttpParameter[];
    };
    output: HttpResponse;
  };
  imageOpen: { input: EmptyOperationInput; output: ImagePreviewResult & { fileName: string } };
  imageEdit: { input: ImageEditInput; output: ImagePreviewResult };
  imageReset: { input: EmptyOperationInput; output: ImagePreviewResult };
  imageSave: { input: EmptyOperationInput; output: { fileName: string } };
  imageSplit: { input: ImageSplitInput; output: { saved: true } };
  imageTakeDropped: { input: EmptyOperationInput; output: ImageTakeDroppedResult };
  databaseHistory: {
    input: EmptyOperationInput;
    output: {
      history: DatabaseHistoryEntry[];
      selectedFileName: string;
      connected: boolean;
      connectedType: DatabaseDriver | "";
    };
  };
  databaseSelectFile: {
    input: EmptyOperationInput;
    output: { selected: true; fileName: string };
  };
  databaseConnect: {
    input: DatabaseConnectionInput;
    output: { connected: true; displayName: string; tables: string[] };
  };
  databaseDisconnect: { input: EmptyOperationInput; output: { disconnected: true } };
  databaseQuery: {
    input: { sql: string };
    output: {
      columns: string[];
      rows: Array<Array<string | null>>;
      truncated: boolean;
      affectedRows: number;
    };
  };
  databaseTables: { input: EmptyOperationInput; output: { tables: string[] } };
  saveLastUsedTool: { input: { toolId: number }; output: { saved: true } };
};

export type OperationName = keyof OperationContracts;
export type OperationInput<Name extends OperationName> = OperationContracts[Name]["input"];
export type OperationOutput<Name extends OperationName> = OperationContracts[Name]["output"];

export type BridgeError = { code: string; message: string };

export type OperationRequest<Name extends OperationName = OperationName> = {
  [Operation in Name]: {
    contractVersion: typeof BRIDGE_CONTRACT_VERSION;
    requestId: string;
    operation: Operation;
    payload: OperationInput<Operation>;
  };
}[Name];

export type BridgeEvent =
  | { type: "started"; requestId: string; operation: string }
  | { type: "progress"; requestId: string; percent: number; message: string }
  | { type: "completed"; requestId: string; result: Record<string, unknown> }
  | { type: "failed"; requestId: string; error: BridgeError }
  | { type: "cancelled"; requestId: string }
  | { type: "imageDropAvailable" }
  | { type: "navigate"; route: string };

export class NativeOperationError extends Error {
  readonly _tag = "NativeOperationError";

  constructor(
    message: string,
    readonly code: string,
  ) {
    super(message);
    this.name = this._tag;
  }
}

export class OperationCancelledError extends Error {
  readonly _tag = "OperationCancelledError";

  constructor(message: string) {
    super(message);
    this.name = this._tag;
  }
}

export type OperationRunner = <Name extends OperationName>(
  operation: Name,
  payload: OperationInput<Name>,
) => Effect.Effect<OperationOutput<Name>, NativeOperationError | OperationCancelledError>;
