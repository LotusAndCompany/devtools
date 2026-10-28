import { prepareBridgeValue } from "@/contract";

export const BRIDGE_CONTRACT_VERSION = "1" as const;

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
  | "phraseData"
  | "evaluateRegex"
  | "regexSettings"
  | "sendHttpRequest"
  | "imageOpen"
  | "imageEdit"
  | "imageReset"
  | "imageSave"
  | "imageSplit"
  | "databaseHistory"
  | "databaseSelectFile"
  | "databaseConnect"
  | "databaseDisconnect"
  | "databaseQuery"
  | "databaseTables"
  | "saveLastUsedTool";
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
  | { type: "navigate"; route: string };

type QtSignal<Args extends unknown[]> = {
  connect: (handler: (...args: Args) => void) => void;
  disconnect: (handler: (...args: Args) => void) => void;
};

type RawNativeBridge = {
  contractVersion: string;
  startOperation: (request: OperationRequest) => void;
  cancelOperation: (requestId: string) => void;
  operationStarted: QtSignal<[string, string]>;
  operationProgress: QtSignal<[string, number, string]>;
  operationCompleted: QtSignal<[string, Record<string, unknown>]>;
  operationFailed: QtSignal<[string, BridgeError]>;
  operationCancelled: QtSignal<[string]>;
  navigationRequested: QtSignal<[string]>;
};

type QWebChannelInstance = {
  objects: Record<string, unknown>;
};

declare global {
  interface Window {
    qt?: { webChannelTransport: unknown };
    QWebChannel?: new (
      transport: unknown,
      initialized: (channel: QWebChannelInstance) => void,
    ) => unknown;
  }
}

export type NativeBridge = {
  contractVersion: string;
  startOperation: (request: OperationRequest) => void;
  cancelOperation: (requestId: string) => void;
  subscribe: (handler: (event: BridgeEvent) => void) => () => void;
};

function loadQWebChannel(): Promise<void> {
  if (window.QWebChannel) {
    return Promise.resolve();
  }

  return new Promise((resolve, reject) => {
    const script = document.createElement("script");
    script.src = "qrc:///qtwebchannel/qwebchannel.js";
    script.onload = () => (window.QWebChannel ? resolve() : reject(new Error("WebChannel script did not initialize.")));
    script.onerror = () => reject(new Error("Could not load Qt WebChannel support."));
    document.head.appendChild(script);
  });
}

export async function connectNativeBridge(): Promise<NativeBridge> {
  const transport = window.qt?.webChannelTransport;
  if (!transport) {
    throw new Error("Launch this screen from DevTools to connect native C++ services.");
  }
  await loadQWebChannel();
  const QWebChannel = window.QWebChannel;
  if (!QWebChannel) {
    throw new Error("Qt WebChannel support is unavailable.");
  }

  const channel = await new Promise<QWebChannelInstance>((resolve, reject) => {
    const timeout = window.setTimeout(() => reject(new Error("Native bridge connection timed out.")), 10_000);
    try {
      new QWebChannel(transport, (initialized) => {
        window.clearTimeout(timeout);
        resolve(initialized);
      });
    } catch (error) {
      window.clearTimeout(timeout);
      reject(error);
    }
  });
  const native = channel.objects.nativeBridge as RawNativeBridge | undefined;
  if (!native || typeof native.contractVersion !== "string") {
    throw new Error("The native bridge was not registered.");
  }
  if (native.contractVersion !== BRIDGE_CONTRACT_VERSION) {
    throw new Error(`Bridge contract mismatch: frontend ${BRIDGE_CONTRACT_VERSION}, native ${native.contractVersion}.`);
  }

  return {
    contractVersion: native.contractVersion,
    startOperation: (request) => native.startOperation(prepareBridgeValue(request) as OperationRequest),
    cancelOperation: (requestId) => native.cancelOperation(requestId),
    subscribe: (handler) => {
      const started = (requestId: string, operation: string) =>
        handler({ type: "started", requestId, operation });
      const progress = (requestId: string, percent: number, message: string) =>
        handler({ type: "progress", requestId, percent, message });
      const completed = (requestId: string, result: Record<string, unknown>) =>
        handler({ type: "completed", requestId, result });
      const failed = (requestId: string, error: BridgeError) =>
        handler({ type: "failed", requestId, error });
      const cancelled = (requestId: string) => handler({ type: "cancelled", requestId });
      const navigate = (route: string) => handler({ type: "navigate", route });

      native.operationStarted.connect(started);
      native.operationProgress.connect(progress);
      native.operationCompleted.connect(completed);
      native.operationFailed.connect(failed);
      native.operationCancelled.connect(cancelled);
      native.navigationRequested.connect(navigate);

      return () => {
        native.operationStarted.disconnect(started);
        native.operationProgress.disconnect(progress);
        native.operationCompleted.disconnect(completed);
        native.operationFailed.disconnect(failed);
        native.operationCancelled.disconnect(cancelled);
        native.navigationRequested.disconnect(navigate);
      };
    },
  };
}
