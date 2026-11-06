import { Effect } from "effect";
import { prepareBridgeValue } from "@/shared/bridge/contract";
import { BridgeConnectionError, type NativeBridge } from "@/shared/bridge/native-bridge";
import {
  BRIDGE_CONTRACT_VERSION,
  type BridgeError,
  type BridgeEvent,
  type OperationRequest,
} from "@/shared/bridge/types";

const EVENT_NAME = "devtools-native-event";

type NativeMessageHandler = {
  postMessage: (message: Record<string, unknown>) => void;
};

declare global {
  interface Window {
    webkit?: {
      messageHandlers?: {
        devtoolsNativeBridge?: NativeMessageHandler;
      };
    };
  }
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function bridgeEvent(value: unknown): BridgeEvent | null {
  if (!isRecord(value) || typeof value.type !== "string") return null;
  if (value.type === "imageDropAvailable") return { type: value.type };
  if (value.type === "navigate" && typeof value.route === "string") {
    return { type: value.type, route: value.route };
  }
  if (typeof value.requestId !== "string") return null;
  switch (value.type) {
    case "started":
      return typeof value.operation === "string"
        ? { type: value.type, requestId: value.requestId, operation: value.operation }
        : null;
    case "progress":
      return typeof value.percent === "number" && typeof value.message === "string"
        ? {
            type: value.type,
            requestId: value.requestId,
            percent: value.percent,
            message: value.message,
          }
        : null;
    case "completed":
      return isRecord(value.result)
        ? { type: value.type, requestId: value.requestId, result: value.result }
        : null;
    case "failed":
      if (!isRecord(value.error)) return null;
      return typeof value.error.code === "string" && typeof value.error.message === "string"
        ? {
            type: value.type,
            requestId: value.requestId,
            error: value.error as BridgeError,
          }
        : null;
    case "cancelled":
      return { type: value.type, requestId: value.requestId };
    default:
      return null;
  }
}

/* oxlint-disable unicorn/require-post-message-target-origin -- WKScriptMessageHandler is not Window.postMessage. */
function createBridge(handler: NativeMessageHandler): NativeBridge {
  return {
    contractVersion: BRIDGE_CONTRACT_VERSION,
    startOperation: (request) =>
      handler.postMessage({
        type: "startOperation",
        request: prepareBridgeValue(request) as OperationRequest,
      }),
    cancelOperation: (requestId) => handler.postMessage({ type: "cancelOperation", requestId }),
    subscribe: (listener) => {
      const onEvent = (event: Event) => {
        const detail = (event as CustomEvent<unknown>).detail;
        const parsed = bridgeEvent(detail);
        if (parsed) listener(parsed);
      };
      window.addEventListener(EVENT_NAME, onEvent);
      return () => window.removeEventListener(EVENT_NAME, onEvent);
    },
  };
}
/* oxlint-enable unicorn/require-post-message-target-origin */

export function connectNativeBridge(): Effect.Effect<NativeBridge, BridgeConnectionError> {
  return Effect.callback<NativeBridge, BridgeConnectionError>((resume, signal) => {
    const handler = window.webkit?.messageHandlers?.devtoolsNativeBridge;
    if (!handler) {
      resume(
        Effect.fail(
          new BridgeConnectionError("Launch this screen from DevTools to connect native services."),
        ),
      );
      return Effect.void;
    }

    let settled = false;
    const cleanup = () => {
      window.clearTimeout(timeout);
      window.removeEventListener(EVENT_NAME, onEvent);
      signal.removeEventListener("abort", cleanup);
    };
    const finish = (effect: Effect.Effect<NativeBridge, BridgeConnectionError>) => {
      if (settled) return;
      settled = true;
      cleanup();
      resume(effect);
    };
    const onEvent = (event: Event) => {
      const detail = (event as CustomEvent<unknown>).detail;
      if (!isRecord(detail) || detail.type !== "ready") return;
      if (detail.contractVersion !== BRIDGE_CONTRACT_VERSION) {
        finish(
          Effect.fail(
            new BridgeConnectionError(
              `Bridge contract mismatch: frontend ${BRIDGE_CONTRACT_VERSION}, native ${String(detail.contractVersion)}.`,
            ),
          ),
        );
        return;
      }
      finish(Effect.succeed(createBridge(handler)));
    };
    const timeout = window.setTimeout(
      () => finish(Effect.fail(new BridgeConnectionError("Native bridge connection timed out."))),
      10_000,
    );
    window.addEventListener(EVENT_NAME, onEvent);
    signal.addEventListener("abort", cleanup, { once: true });
    try {
      // oxlint-disable-next-line unicorn/require-post-message-target-origin -- This is not Window.postMessage.
      handler.postMessage({ type: "handshake" });
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      finish(Effect.fail(new BridgeConnectionError(message)));
    }
    return Effect.sync(cleanup);
  });
}
