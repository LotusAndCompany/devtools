import type { Locale } from "@/shared/types";

export function licenseTextOrFallback(licenseText: string | null | undefined, locale: Locale) {
  if (licenseText?.trim()) return licenseText;
  return locale === "ja"
    ? "ライセンス情報を読み込めませんでした。"
    : "Failed to load license information.";
}
