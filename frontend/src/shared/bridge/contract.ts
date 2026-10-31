export const MAX_BRIDGE_VALUE_BYTES = 1024 * 1024;
export const MAX_BRIDGE_MESSAGE_BYTES = 2 * 1024 * 1024;

export type BridgeValue =
  | null
  | boolean
  | number
  | string
  | BridgeValue[]
  | { [key: string]: BridgeValue };

function utf8Size(value: string) {
  return new TextEncoder().encode(value).byteLength;
}

function encodeBase64(bytes: Uint8Array) {
  let binary = "";
  const chunkSize = 0x8000;
  for (let offset = 0; offset < bytes.length; offset += chunkSize) {
    const chunk = bytes.subarray(offset, Math.min(offset + chunkSize, bytes.length));
    binary += String.fromCharCode(...chunk);
  }
  return btoa(binary);
}

export function normalizeBridgeValue(value: unknown): BridgeValue {
  if (value === null || typeof value === "boolean") return value;
  if (typeof value === "string") {
    if (utf8Size(value) > MAX_BRIDGE_VALUE_BYTES) {
      throw new RangeError("A bridge string exceeds the 1 MiB limit.");
    }
    return value;
  }
  if (typeof value === "number") {
    if (!Number.isFinite(value) || (Number.isInteger(value) && !Number.isSafeInteger(value))) {
      throw new TypeError("Numbers must be finite; 64-bit integers must use decimal strings.");
    }
    return value;
  }
  if (typeof value === "bigint") return value.toString(10);
  if (value instanceof Date) {
    if (Number.isNaN(value.getTime()))
      throw new TypeError("Invalid dates cannot cross the bridge.");
    return value.toISOString();
  }
  if (value instanceof Uint8Array) {
    if (value.byteLength > MAX_BRIDGE_VALUE_BYTES) {
      throw new RangeError("A binary bridge value exceeds the 1 MiB limit.");
    }
    return { $type: "bytes", base64: encodeBase64(value) };
  }
  if (Array.isArray(value)) return value.map(normalizeBridgeValue);
  if (typeof value === "object") {
    const prototype = Object.getPrototypeOf(value);
    if (prototype !== Object.prototype && prototype !== null) {
      throw new TypeError("Only plain objects can cross the bridge.");
    }
    const normalized: Record<string, BridgeValue> = Object.create(null) as Record<
      string,
      BridgeValue
    >;
    for (const [key, entry] of Object.entries(value)) {
      normalized[key] = normalizeBridgeValue(entry);
    }
    return normalized;
  }
  throw new TypeError("Undefined, functions and symbols cannot cross the bridge.");
}

export function prepareBridgeValue(value: unknown): BridgeValue {
  const normalized = normalizeBridgeValue(value);
  const serialized = JSON.stringify(normalized);
  if (utf8Size(serialized) > MAX_BRIDGE_MESSAGE_BYTES) {
    throw new RangeError("A bridge message exceeds the 2 MiB limit.");
  }
  return normalized;
}
