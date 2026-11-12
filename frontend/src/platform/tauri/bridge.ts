import { setTheme as setTauriAppTheme } from "@tauri-apps/api/app";
import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import { getCurrentWindow } from "@tauri-apps/api/window";
import { Effect } from "effect";
import { BridgeConnectionError, type NativeBridge } from "@/shared/bridge/native-bridge";
import {
    BRIDGE_CONTRACT_VERSION,
    type ApplicationSettings,
    type BridgeEvent,
    type OperationRequest,
} from "@/shared/bridge/types";

const EVENT_NAME = "native-bridge";

export function setNativeTheme(theme: ApplicationSettings["theme"]): Promise<void> {
    return setTauriAppTheme(theme === "system" ? null : theme);
}

export function openExternalUrl(url: string): Promise<void> {
    return invoke<void>("plugin:opener|open_url", { url, with: null });
}

export async function showMainWindow(theme: ApplicationSettings["theme"]): Promise<void> {
    await setNativeTheme(theme).catch(() => {});
    await getCurrentWindow().show();
}

function failedEvent(requestId: string, error: unknown): BridgeEvent {
    const message = error instanceof Error ? error.message : String(error);
    const separator = message.indexOf(":");
    const code = separator > 0 ? message.slice(0, separator) : "START_FAILED";
    return {
        type: "failed",
        requestId,
        error: {
            code: /^[A-Z_]+$/.test(code) ? code : "START_FAILED",
            message: separator > 0 ? message.slice(separator + 1).trim() : message,
        },
    };
}

function isRecord(value: unknown): value is Record<string, unknown> {
    return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isRequestId(value: unknown): value is string {
    return typeof value === "string" && /^[A-Za-z0-9_-]{1,64}$/.test(value);
}

function parseBridgeEvent(value: unknown): BridgeEvent | undefined {
    if (!isRecord(value)) return;

    switch (value.type) {
        case "started":
            return isRequestId(value.requestId) && typeof value.operation === "string"
                ? { type: "started", requestId: value.requestId, operation: value.operation }
                : undefined;
        case "progress":
            return isRequestId(value.requestId) &&
                typeof value.percent === "number" &&
                Number.isFinite(value.percent) &&
                typeof value.message === "string"
                ? {
                      type: "progress",
                      requestId: value.requestId,
                      percent: value.percent,
                      message: value.message,
                  }
                : undefined;
        case "completed":
            return isRequestId(value.requestId) && "result" in value
                ? { type: "completed", requestId: value.requestId, result: value.result }
                : undefined;
        case "failed":
            return isRequestId(value.requestId) &&
                isRecord(value.error) &&
                typeof value.error.code === "string" &&
                typeof value.error.message === "string"
                ? {
                      type: "failed",
                      requestId: value.requestId,
                      error: { code: value.error.code, message: value.error.message },
                  }
                : undefined;
        case "cancelled":
            return isRequestId(value.requestId)
                ? { type: "cancelled", requestId: value.requestId }
                : undefined;
        case "imageDropAvailable":
            return { type: "imageDropAvailable" };
        case "navigate":
            return typeof value.route === "string"
                ? { type: "navigate", route: value.route }
                : undefined;
        default:
            return undefined;
    }
}

function invalidEvent(requestId: string): BridgeEvent {
    return {
        type: "failed",
        requestId,
        error: { code: "INVALID_RESPONSE", message: "Native bridge returned an invalid event." },
    };
}

export function connectTauriBridge(): Effect.Effect<NativeBridge, BridgeConnectionError> {
    return Effect.callback<NativeBridge, BridgeConnectionError>((resume, signal) => {
        let settled = false;
        let unlisten: (() => void) | undefined;
        const listeners = new Set<(event: BridgeEvent) => void>();
        const cleanup = () => {
            unlisten?.();
            signal.removeEventListener("abort", cleanup);
        };
        const finish = (
            effect: Effect.Effect<NativeBridge, BridgeConnectionError>,
            keepListening = false,
        ) => {
            if (settled) return;
            settled = true;
            signal.removeEventListener("abort", cleanup);
            if (!keepListening) unlisten?.();
            resume(effect);
        };

        void listen<unknown>(EVENT_NAME, ({ payload }) => {
            const event = parseBridgeEvent(payload);
            if (event) {
                for (const listener of listeners) listener(event);
            } else if (isRecord(payload) && isRequestId(payload.requestId)) {
                const failure = invalidEvent(payload.requestId);
                for (const listener of listeners) listener(failure);
            }
        }).then(
            (stopListening) => {
                unlisten = stopListening;
                if (signal.aborted) {
                    cleanup();
                    return;
                }
                finish(
                    Effect.succeed({
                        contractVersion: BRIDGE_CONTRACT_VERSION,
                        startOperation: (request: OperationRequest) => {
                            void invoke("start_operation", { request }).catch((error: unknown) => {
                                const event = failedEvent(request.requestId, error);
                                for (const listener of listeners) listener(event);
                            });
                        },
                        cancelOperation: (requestId: string) => {
                            void invoke("cancel_operation", { requestId }).catch(() => {});
                        },
                        subscribe: (listener: (event: BridgeEvent) => void) => {
                            listeners.add(listener);
                            return () => {
                                listeners.delete(listener);
                                if (listeners.size === 0) {
                                    unlisten?.();
                                    unlisten = undefined;
                                }
                            };
                        },
                    }),
                    true,
                );
            },
            (error: unknown) =>
                finish(
                    Effect.fail(
                        new BridgeConnectionError(
                            error instanceof Error ? error.message : String(error),
                        ),
                    ),
                ),
        );

        signal.addEventListener("abort", cleanup, { once: true });
        return Effect.sync(cleanup);
    });
}
