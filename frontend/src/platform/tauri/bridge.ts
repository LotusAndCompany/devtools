import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import { Effect } from "effect";
import { BridgeConnectionError, type NativeBridge } from "@/shared/bridge/native-bridge";
import {
    BRIDGE_CONTRACT_VERSION,
    type BridgeEvent,
    type OperationRequest,
} from "@/shared/bridge/types";

const EVENT_NAME = "native-bridge";

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

        void listen<BridgeEvent>(EVENT_NAME, ({ payload }) => {
            for (const listener of listeners) listener(payload);
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
