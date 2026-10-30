import type { Effect } from "effect";

export const BRIDGE_CONTRACT_VERSION = "2" as const;

export type OperationName =
  | "formatJson"
  | "openTextFile"
  | "saveTextFile"
  | "convertData"
  | "convertTimestamp"
  | "compareText"
  | "renderMarkdown"
  | "generateQr"
  | "saveQrImage"
  | "commandData"
  | "getSettings"
  | "saveSettings"
  | "copyToClipboard"
  | "getAboutInfo"
  | "phraseData"
  | "evaluateRegex"
  | "regexSettings"
  | "sendHttpRequest"
  | "imageOpen"
  | "imageEdit"
  | "imageReset"
  | "imageSave"
  | "imageSplit"
  | "imageTakeDropped"
  | "databaseHistory"
  | "databaseSelectFile"
  | "databaseConnect"
  | "databaseDisconnect"
  | "databaseQuery"
  | "databaseTables"
  | "saveLastUsedTool";

export type AboutInfo = {
  appVersion: string;
  revision: string;
  buildEnvironment: string;
  qtVersion: string;
  copyright: string;
  licenseText: string;
};

export type BridgeError = { code: string; message: string };

export type OperationRequest = {
  contractVersion: typeof BRIDGE_CONTRACT_VERSION;
  requestId: string;
  operation: OperationName;
  payload: Record<string, unknown>;
};

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

export type OperationRunner = (
  operation: OperationName,
  payload: Record<string, unknown>,
) => Effect.Effect<Record<string, unknown>, NativeOperationError | OperationCancelledError>;
