import { afterEach, describe, expect, it, vi } from "vitest";
import { Effect } from "effect";
import {
  BRIDGE_CONTRACT_VERSION,
  BridgeConnectionError,
  connectNativeBridge,
} from "@/platform/qt/bridge";
import {
  MAX_BRIDGE_MESSAGE_BYTES,
  MAX_BRIDGE_VALUE_BYTES,
  normalizeBridgeValue,
  prepareBridgeValue,
} from "@/shared/bridge/contract";

function QWebChannelThatDoesNotComplete() {
  return undefined;
}

function QWebChannelWithWrongContract(
  _transport: unknown,
  initialized: (channel: { objects: Record<string, unknown> }) => void,
) {
  initialized({ objects: { nativeBridge: { contractVersion: "3" } } });
}

describe("bridge value serialization", () => {
  it("keeps null and empty strings distinct", () => {
    expect(normalizeBridgeValue({ empty: "", missing: null })).toEqual({
      empty: "",
      missing: null,
    });
  });

  it("uses ISO UTC strings for dates and decimal strings for bigint values", () => {
    expect(
      normalizeBridgeValue({
        date: new Date("2026-10-28T12:34:56.000Z"),
        id: 9_007_199_254_740_993n,
      }),
    ).toEqual({ date: "2026-10-28T12:34:56.000Z", id: "9007199254740993" });
  });

  it("encodes bytes as an explicit base64 DTO", () => {
    expect(normalizeBridgeValue({ bytes: new Uint8Array([0, 1, 2, 255]) })).toEqual({
      bytes: { $type: "bytes", base64: "AAEC/w==" },
    });
  });

  it("rejects unsafe numeric int64 values, undefined and non-finite numbers", () => {
    expect(() => normalizeBridgeValue(Number.MAX_SAFE_INTEGER + 1)).toThrow(/decimal strings/);
    expect(() => normalizeBridgeValue(undefined)).toThrow(/Undefined/);
    expect(() => normalizeBridgeValue(Number.NaN)).toThrow(/finite/);
  });

  it("enforces per-value and serialized message limits", () => {
    expect(normalizeBridgeValue("a".repeat(MAX_BRIDGE_VALUE_BYTES))).toHaveLength(
      MAX_BRIDGE_VALUE_BYTES,
    );
    expect(() => normalizeBridgeValue("a".repeat(MAX_BRIDGE_VALUE_BYTES + 1))).toThrow(/1 MiB/);
    expect(() =>
      prepareBridgeValue(["a".repeat(MAX_BRIDGE_VALUE_BYTES), "b".repeat(MAX_BRIDGE_VALUE_BYTES)]),
    ).toThrow(new RegExp(`${MAX_BRIDGE_MESSAGE_BYTES / (1024 * 1024)} MiB`));
  });
});
describe("native bridge initialization", () => {
  afterEach(() => {
    vi.useRealTimers();
    vi.unstubAllGlobals();
  });

  it("fails clearly when the Qt transport is missing", async () => {
    vi.stubGlobal("window", {} as Window);

    await expect(Effect.runPromise(connectNativeBridge())).rejects.toBeInstanceOf(
      BridgeConnectionError,
    );
  });

  it("times out when the Qt WebChannel handshake never completes", async () => {
    vi.useFakeTimers();
    vi.stubGlobal("window", {
      qt: { webChannelTransport: {} },
      QWebChannel: QWebChannelThatDoesNotComplete,
      setTimeout,
      clearTimeout,
    } as unknown as Window);

    const connection = Effect.runPromise(connectNativeBridge()).then(
      (bridge) => ({ bridge }),
      (error: unknown) => ({ error }),
    );
    await vi.advanceTimersByTimeAsync(10_000);
    const outcome = await connection;
    if (!("error" in outcome)) throw new Error("Expected the bridge handshake to time out.");
    expect(outcome.error).toMatchObject({ message: "Native bridge connection timed out." });
  });

  it("rejects a native bridge with an incompatible contract version", async () => {
    vi.stubGlobal("window", {
      qt: { webChannelTransport: {} },
      QWebChannel: QWebChannelWithWrongContract,
      setTimeout,
      clearTimeout,
    } as unknown as Window);

    await expect(Effect.runPromise(connectNativeBridge())).rejects.toThrow(
      `Bridge contract mismatch: frontend ${BRIDGE_CONTRACT_VERSION}, native 3.`,
    );
  });
});
