import { Effect } from "effect";
import { afterEach, describe, expect, it, vi } from "vitest";
import {
    BRIDGE_CONTRACT_VERSION,
    type BridgeEvent,
    type OperationRequest,
} from "@/shared/bridge/types";
import { connectTauriBridge } from "./bridge";

type TauriEventHandler = (event: { payload: BridgeEvent }) => void;

const { invokeMock, listenMock } = vi.hoisted(() => ({
    invokeMock: vi.fn<(command: string, args?: Record<string, unknown>) => Promise<unknown>>(),
    listenMock: vi.fn<(event: string, handler: TauriEventHandler) => Promise<() => void>>(),
}));

vi.mock("@tauri-apps/api/core", () => ({ invoke: invokeMock }));
vi.mock("@tauri-apps/api/event", () => ({ listen: listenMock }));

describe("Tauri native bridge", () => {
    afterEach(() => vi.resetAllMocks());

    it("forwards shared operation events and unsubscribes when the UI releases the bridge", async () => {
        let emit!: TauriEventHandler;
        const stopListening = vi.fn<() => void>();
        invokeMock.mockResolvedValue(undefined);
        listenMock.mockImplementation(async (_name: string, handler: TauriEventHandler) => {
            emit = handler;
            return stopListening;
        });
        const bridge = await Effect.runPromise(connectTauriBridge());
        const request: OperationRequest<"formatJson"> = {
            contractVersion: BRIDGE_CONTRACT_VERSION,
            requestId: "request-1",
            operation: "formatJson",
            payload: { text: "{}", indentation: "2" },
        };
        const events: BridgeEvent[] = [];
        const unsubscribe = bridge.subscribe((event) => events.push(event));

        bridge.startOperation(request);
        emit({ payload: { type: "started", requestId: "request-1", operation: "formatJson" } });
        emit({ payload: { type: "completed", requestId: "request-1", result: { text: "{}\n" } } });

        expect(invokeMock).toHaveBeenCalledWith("start_operation", { request });
        expect(events.map((event) => event.type)).toEqual(["started", "completed"]);
        unsubscribe();
        expect(stopListening).toHaveBeenCalledOnce();
    });

    it("converts command validation errors into correlated bridge failures", async () => {
        let emit!: TauriEventHandler;
        listenMock.mockImplementation(async (_name: string, handler: TauriEventHandler) => {
            emit = handler;
            return vi.fn<() => void>();
        });
        invokeMock.mockRejectedValue("INVALID_INPUT: Invalid request ID.");
        const bridge = await Effect.runPromise(connectTauriBridge());
        const events: BridgeEvent[] = [];
        bridge.subscribe((event) => events.push(event));
        bridge.startOperation({
            contractVersion: BRIDGE_CONTRACT_VERSION,
            requestId: "bad",
            operation: "formatJson",
            payload: { text: "{}", indentation: "2" },
        });
        await vi.waitFor(() => expect(events).toHaveLength(1));

        expect(events[0]).toEqual({
            type: "failed",
            requestId: "bad",
            error: { code: "INVALID_INPUT", message: "Invalid request ID." },
        });
        emit({ payload: { type: "cancelled", requestId: "bad" } });
        expect(events).toHaveLength(2);
    });

    it("sends cancellation through the Tauri command", async () => {
        listenMock.mockResolvedValue(vi.fn<() => void>());
        invokeMock.mockResolvedValue(undefined);
        const bridge = await Effect.runPromise(connectTauriBridge());

        bridge.cancelOperation("request-2");

        expect(invokeMock).toHaveBeenCalledWith("cancel_operation", { requestId: "request-2" });
    });
});
