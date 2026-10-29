import type { Locale } from "@/tool-pages";

export function licenseTextOrFallback(licenseText: string | null | undefined, locale: Locale) {
  if (licenseText?.trim()) return licenseText;
  return locale === "ja"
    ? "ライセンス情報を読み込めませんでした。"
    : "Failed to load license information.";
}
