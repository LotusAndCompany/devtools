import { describe, expect, it } from "vitest";
import { licenseTextOrFallback } from "./license";

describe("About license text", () => {
  it("shows the localized failure message when the license resource is empty", () => {
    expect(licenseTextOrFallback("", "ja")).toBe("ライセンス情報を読み込めませんでした。");
    expect(licenseTextOrFallback(undefined, "en")).toBe("Failed to load license information.");
  });

  it("preserves the loaded license text", () => {
    expect(licenseTextOrFallback("Apache License", "en")).toBe("Apache License");
  });
});
