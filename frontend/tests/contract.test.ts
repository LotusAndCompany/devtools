import { afterEach, describe, expect, it, vi } from "vitest";
import { connectNativeBridge } from "@/bridge";
import {
  MAX_BRIDGE_MESSAGE_BYTES,
  MAX_BRIDGE_VALUE_BYTES,
  normalizeBridgeValue,
  prepareBridgeValue,
} from "@/contract";

describe("bridge value serialization", () => {
  it("keeps null and empty strings distinct", () => {
    expect(normalizeBridgeValue({ empty: "", missing: null })).toEqual({ empty: "", missing: null });
  });

  it("uses ISO UTC strings for dates and decimal strings for bigint values", () => {
    expect(
      normalizeBridgeValue({ date: new Date("2026-10-28T12:34:56.000Z"), id: 9_007_199_254_740_993n }),
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
    expect(normalizeBridgeValue("a".repeat(MAX_BRIDGE_VALUE_BYTES))).toHaveLength(MAX_BRIDGE_VALUE_BYTES);
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

  it("times out when the Qt WebChannel handshake never completes", async () => {
    vi.useFakeTimers();
    vi.stubGlobal("window", {
      qt: { webChannelTransport: {} },
      QWebChannel: class {},
      setTimeout,
      clearTimeout,
    } as unknown as Window);

    const connection = connectNativeBridge();
    const rejection = expect(connection).rejects.toThrow("Native bridge connection timed out.");
    await vi.advanceTimersByTimeAsync(10_000);
    await rejection;
  });
});
