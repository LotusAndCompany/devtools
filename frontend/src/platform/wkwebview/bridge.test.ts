import { Effect } from "effect";
import { afterEach, describe, expect, it, vi } from "vitest";
import { connectNativeBridge } from "@/platform/wkwebview/bridge";
import { BRIDGE_CONTRACT_VERSION } from "@/shared/bridge/types";

const EVENT_NAME = "devtools-native-event";

function installNativeHandler(onMessage: (message: Record<string, unknown>) => void) {
  const postMessage = vi.fn<(message: Record<string, unknown>) => void>(onMessage);
  Object.defineProperty(window, "webkit", {
    configurable: true,
    value: { messageHandlers: { devtoolsNativeBridge: { postMessage } } },
  });
  return postMessage;
}

function ready(contractVersion: string = BRIDGE_CONTRACT_VERSION) {
  window.dispatchEvent(new CustomEvent(EVENT_NAME, { detail: { type: "ready", contractVersion } }));
}

describe("WKWebView native bridge", () => {
  afterEach(() => {
    vi.useRealTimers();
    delete (window as Window & { webkit?: unknown }).webkit;
  });

  it("handshakes, sends bounded operations, cancels, and receives native events", async () => {
    const postMessage = installNativeHandler((message) => {
      if (message.type === "handshake") queueMicrotask(() => ready());
    });
    const bridge = await Effect.runPromise(connectNativeBridge());
    const received: unknown[] = [];
    const unsubscribe = bridge.subscribe((event) => received.push(event));

    const request = {
      contractVersion: BRIDGE_CONTRACT_VERSION,
      requestId: "request_1",
      operation: "getAboutInfo",
      payload: {},
    } as const;
    bridge.startOperation(request);
    bridge.cancelOperation(request.requestId);
    window.dispatchEvent(
      new CustomEvent(EVENT_NAME, {
        detail: { type: "completed", requestId: "request_1", result: { appVersion: "1.0" } },
      }),
    );

    expect(postMessage).toHaveBeenNthCalledWith(1, { type: "handshake" });
    expect(postMessage).toHaveBeenNthCalledWith(2, { type: "startOperation", request });
    expect(postMessage).toHaveBeenNthCalledWith(3, {
      type: "cancelOperation",
      requestId: "request_1",
    });
    expect(received).toEqual([
      { type: "completed", requestId: "request_1", result: { appVersion: "1.0" } },
    ]);
    unsubscribe();
  });

  it("rejects a native host with a different contract version", async () => {
    installNativeHandler((message) => {
      if (message.type === "handshake") queueMicrotask(() => ready("3"));
    });

    await expect(Effect.runPromise(connectNativeBridge())).rejects.toThrow(
      `Bridge contract mismatch: frontend ${BRIDGE_CONTRACT_VERSION}, native 3.`,
    );
  });

  it("times out when the host does not answer the handshake", async () => {
    vi.useFakeTimers();
    installNativeHandler(() => undefined);

    const connection = Effect.runPromise(connectNativeBridge()).then(
      (bridge) => ({ bridge }),
      (error: unknown) => ({ error }),
    );
    await vi.advanceTimersByTimeAsync(10_000);
    const outcome = await connection;
    if (!("error" in outcome)) throw new Error("Expected the bridge handshake to time out.");
    expect(outcome.error).toMatchObject({ message: "Native bridge connection timed out." });
  });
});
