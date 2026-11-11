import { afterEach, describe, expect, it, vi } from "vitest";
import { Effect, Fiber } from "effect";
import { runNativeOperation } from "@/shared/bridge/operation-client";
import type { NativeBridge } from "@/shared/bridge/native-bridge";
import {
    BRIDGE_CONTRACT_VERSION,
    NativeOperationError,
    OperationCancelledError,
    type BridgeEvent,
    type OperationRequest,
} from "@/shared/bridge/types";

const request: OperationRequest = {
    contractVersion: BRIDGE_CONTRACT_VERSION,
    requestId: "request-1",
    operation: "formatJson",
    payload: { text: "{}", indentation: "2" },
};

function createBridge(start?: (emit: (event: BridgeEvent) => void) => void) {
    let emitEvent: ((event: BridgeEvent) => void) | undefined;
    const unsubscribe = vi.fn<() => void>(() => {
        emitEvent = undefined;
    });
    const bridge: NativeBridge = {
        contractVersion: BRIDGE_CONTRACT_VERSION,
        startOperation: () => start?.((event) => emitEvent?.(event)),
        cancelOperation: vi.fn<(requestId: string) => void>(),
        subscribe: vi.fn<(handler: (event: BridgeEvent) => void) => () => void>((handler) => {
            emitEvent = handler;
            return unsubscribe;
        }),
    };
    return { bridge, unsubscribe };
}

describe("native operation effects", () => {
    const pendingChanges: Array<[string, boolean]> = [];
    const onPendingChange = (id: string, isPending: boolean) => {
        pendingChanges.push([id, isPending]);
    };

    afterEach(() => pendingChanges.splice(0));

    it("resolves when the native operation completes", async () => {
        const { bridge } = createBridge((emit) =>
            emit({ type: "completed", requestId: request.requestId, result: { output: "{}" } }),
        );

        await expect(
            Effect.runPromise(runNativeOperation(bridge, request, "Cancelled", onPendingChange)),
        ).resolves.toEqual({ output: "{}" });
        expect(pendingChanges).toEqual([
            [request.requestId, true],
            [request.requestId, false],
        ]);
    });

    it("preserves native operation errors", async () => {
        const { bridge } = createBridge((emit) =>
            emit({
                type: "failed",
                requestId: request.requestId,
                error: { code: "IO_ERROR", message: "Could not write file." },
            }),
        );

        await expect(
            Effect.runPromise(runNativeOperation(bridge, request, "Cancelled", onPendingChange)),
        ).rejects.toMatchObject<Partial<NativeOperationError>>({
            code: "IO_ERROR",
            message: "Could not write file.",
        });
    });

    it("reports native cancellation as a typed error", async () => {
        const { bridge } = createBridge((emit) =>
            emit({ type: "cancelled", requestId: request.requestId }),
        );

        await expect(
            Effect.runPromise(
                runNativeOperation(bridge, request, "Operation cancelled.", onPendingChange),
            ),
        ).rejects.toBeInstanceOf(OperationCancelledError);
    });

    it("cancels and unsubscribes when its Effect fiber is interrupted", async () => {
        const { bridge, unsubscribe } = createBridge();
        const fiber = Effect.runFork(
            runNativeOperation(bridge, request, "Operation cancelled.", onPendingChange),
        );

        await Effect.runPromise(Fiber.interrupt(fiber));

        expect(bridge.cancelOperation).toHaveBeenCalledWith(request.requestId);
        expect(unsubscribe).toHaveBeenCalledOnce();
        expect(pendingChanges.at(-1)).toEqual([request.requestId, false]);
    });
});
