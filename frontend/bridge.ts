import { Effect } from "effect";
import { prepareBridgeValue } from "@/contract";

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

export class BridgeConnectionError extends Error {
  readonly _tag = "BridgeConnectionError";

  constructor(message: string) {
    super(message);
    this.name = this._tag;
  }
}

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
  imageDropAvailable: QtSignal<[]>;
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

function loadQWebChannel(): Effect.Effect<void, BridgeConnectionError> {
  if (window.QWebChannel) return Effect.void;

  return Effect.callback<void, BridgeConnectionError>((resume, signal) => {
    const script = document.createElement("script");
    script.src = "qrc:///qtwebchannel/qwebchannel.js";
    const cleanup = () => {
      script.removeEventListener("load", onLoad);
      script.removeEventListener("error", onError);
      signal.removeEventListener("abort", onAbort);
      script.remove();
    };
    const finish = (effect: Effect.Effect<void, BridgeConnectionError>) => {
      cleanup();
      resume(effect);
    };
    const onLoad = () => {
      if (window.QWebChannel) finish(Effect.void);
      else finish(Effect.fail(new BridgeConnectionError("WebChannel script did not initialize.")));
    };
    const onError = () =>
      finish(Effect.fail(new BridgeConnectionError("Could not load Qt WebChannel support.")));
    const onAbort = () => cleanup();

    script.addEventListener("load", onLoad, { once: true });
    script.addEventListener("error", onError, { once: true });
    signal.addEventListener("abort", onAbort, { once: true });
    document.head.appendChild(script);
    return Effect.sync(cleanup);
  });
}

export function connectNativeBridge(): Effect.Effect<NativeBridge, BridgeConnectionError> {
  return Effect.gen(function* () {
    const transport = window.qt?.webChannelTransport;
    if (!transport) {
      return yield* Effect.fail(
        new BridgeConnectionError(
          "Launch this screen from DevTools to connect native C++ services.",
        ),
      );
    }

    yield* loadQWebChannel();
    const QWebChannel = window.QWebChannel;
    if (!QWebChannel) {
      return yield* Effect.fail(new BridgeConnectionError("Qt WebChannel support is unavailable."));
    }

    const channel = yield* Effect.callback<QWebChannelInstance, BridgeConnectionError>(
      (resume, signal) => {
        let settled = false;
        let timeout: number;
        const cleanup = () => {
          window.clearTimeout(timeout);
          signal.removeEventListener("abort", cleanup);
        };
        const finish = (effect: Effect.Effect<QWebChannelInstance, BridgeConnectionError>) => {
          if (settled) return;
          settled = true;
          cleanup();
          resume(effect);
        };

        timeout = window.setTimeout(
          () =>
            finish(Effect.fail(new BridgeConnectionError("Native bridge connection timed out."))),
          10_000,
        );
        signal.addEventListener("abort", cleanup, { once: true });
        try {
          const webChannel = new QWebChannel(transport, (initialized) =>
            finish(Effect.succeed(initialized)),
          );
          void webChannel;
        } catch (error) {
          const message = error instanceof Error ? error.message : String(error);
          finish(Effect.fail(new BridgeConnectionError(message)));
        }
        return Effect.sync(cleanup);
      },
    );

    const native = channel.objects.nativeBridge as RawNativeBridge | undefined;
    if (!native || typeof native.contractVersion !== "string") {
      return yield* Effect.fail(new BridgeConnectionError("The native bridge was not registered."));
    }
    if (native.contractVersion !== BRIDGE_CONTRACT_VERSION) {
      return yield* Effect.fail(
        new BridgeConnectionError(
          "Bridge contract mismatch: frontend " +
            BRIDGE_CONTRACT_VERSION +
            ", native " +
            native.contractVersion +
            ".",
        ),
      );
    }

    return {
      contractVersion: native.contractVersion,
      startOperation: (request) =>
        native.startOperation(prepareBridgeValue(request) as OperationRequest),
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
        const imageDropAvailable = () => handler({ type: "imageDropAvailable" });
        const navigate = (route: string) => handler({ type: "navigate", route });

        native.operationStarted.connect(started);
        native.operationProgress.connect(progress);
        native.operationCompleted.connect(completed);
        native.operationFailed.connect(failed);
        native.operationCancelled.connect(cancelled);
        native.imageDropAvailable.connect(imageDropAvailable);
        native.navigationRequested.connect(navigate);

        return () => {
          native.operationStarted.disconnect(started);
          native.operationProgress.disconnect(progress);
          native.operationCompleted.disconnect(completed);
          native.operationFailed.disconnect(failed);
          native.operationCancelled.disconnect(cancelled);
          native.imageDropAvailable.disconnect(imageDropAvailable);
          native.navigationRequested.disconnect(navigate);
        };
      },
    };
  });
}

export function runNativeOperation(
  native: NativeBridge,
  request: OperationRequest,
  cancellationMessage: string,
  onPendingChange: (requestId: string, isPending: boolean, cancel: () => void) => void,
): Effect.Effect<Record<string, unknown>, NativeOperationError | OperationCancelledError> {
  return Effect.callback((resume) => {
    let settled = false;
    let unsubscribe: (() => void) | undefined;
    const cancel = () => {
      try {
        native.cancelOperation(request.requestId);
      } catch {
        // The native operation may already have completed while the UI is closing.
      }
    };
    const cleanup = () => {
      unsubscribe?.();
      unsubscribe = undefined;
      onPendingChange(request.requestId, false, cancel);
    };
    const finish = (
      effect: Effect.Effect<
        Record<string, unknown>,
        NativeOperationError | OperationCancelledError
      >,
    ) => {
      if (settled) return;
      settled = true;
      cleanup();
      resume(effect);
    };

    onPendingChange(request.requestId, true, cancel);
    try {
      unsubscribe = native.subscribe((event) => {
        if (event.type === "completed" && event.requestId === request.requestId) {
          finish(Effect.succeed(event.result));
        } else if (event.type === "failed" && event.requestId === request.requestId) {
          finish(
            Effect.fail(
              new NativeOperationError(event.error.message || event.error.code, event.error.code),
            ),
          );
        } else if (event.type === "cancelled" && event.requestId === request.requestId) {
          finish(Effect.fail(new OperationCancelledError(cancellationMessage)));
        }
      });
      native.startOperation(request);
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      finish(Effect.fail(new NativeOperationError(message, "START_FAILED")));
    }

    return Effect.sync(() => {
      if (settled) return;
      settled = true;
      cleanup();
      cancel();
    });
  });
}
