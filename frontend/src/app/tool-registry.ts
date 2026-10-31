import type { ComponentType } from "react";
import {
  ArrowLeftRight,
  BookOpen,
  Clock,
  Database,
  FileText,
  GitCompareArrows,
  House,
  Image,
  Network,
  QrCode,
  Regex,
  Settings,
  Terminal,
  type LucideIcon,
} from "lucide-react";
import { CommandPage } from "@/features/command";
import { DataPage } from "@/features/data";
import { DatabasePage } from "@/features/db";
import { DiffPage } from "@/features/diff";
import { HttpPage } from "@/features/http";
import { ImagePage } from "@/features/image";
import { MarkdownPage } from "@/features/markdown";
import { PhrasePage } from "@/features/phrase";
import { QrPage } from "@/features/qr";
import { RegexPage } from "@/features/regex";
import { TimestampPage } from "@/features/timestamp";
import type { Locale, ToolId, ToolPageProps } from "@/shared/types";

export type { ToolId } from "@/shared/types";

export type PageId = "welcome" | ToolId;

export type ToolDefinition = {
  id: ToolId;
  nativeId: number;
  en: string;
  ja: string;
  detailEn: string;
  detailJa: string;
  icon: LucideIcon;
  screen: ComponentType<ToolPageProps>;
  keepAlive?: boolean;
};

export const tools: ToolDefinition[] = [
  {
    id: "image",
    nativeId: 3,
    en: "Image Editor",
    ja: "画像編集",
    detailEn: "Resize, transform and split images",
    detailJa: "画像のサイズ変更、変形、分割",
    icon: Image,
    screen: ImagePage,
  },
  {
    id: "phrase",
    nativeId: 6,
    en: "Phrase Generation",
    ja: "定型文生成",
    detailEn: "Create and manage text templates",
    detailJa: "定型文を作成、管理",
    icon: BookOpen,
    screen: PhrasePage,
  },
  {
    id: "command",
    nativeId: 2,
    en: "Command Generation",
    ja: "コマンド生成",
    detailEn: "Build Git and Docker commands",
    detailJa: "GitやDockerのコマンドを作成",
    icon: Terminal,
    screen: CommandPage,
  },
  {
    id: "http",
    nativeId: 1,
    en: "HTTP Request",
    ja: "HTTPリクエスト",
    detailEn: "Send and inspect API requests",
    detailJa: "APIリクエストを送信、確認",
    icon: Network,
    screen: HttpPage,
  },
  {
    id: "data",
    nativeId: 7,
    en: "Data Conversion",
    ja: "データ変換",
    detailEn: "Convert JSON, YAML and TOML",
    detailJa: "JSON、YAML、TOMLを変換",
    icon: ArrowLeftRight,
    screen: DataPage,
  },
  {
    id: "db",
    nativeId: 8,
    en: "Database",
    ja: "データベース",
    detailEn: "Connect and run SQL queries",
    detailJa: "データベースに接続してSQLを実行",
    icon: Database,
    screen: DatabasePage,
    keepAlive: true,
  },
  {
    id: "qr",
    nativeId: 9,
    en: "QR Code",
    ja: "QRコード",
    detailEn: "Create QR codes from common data",
    detailJa: "各種データからQRコードを作成",
    icon: QrCode,
    screen: QrPage,
  },
  {
    id: "markdown",
    nativeId: 10,
    en: "Markdown Preview",
    ja: "Markdownプレビュー",
    detailEn: "Edit Markdown and preview safely",
    detailJa: "Markdownを編集、安全にプレビュー",
    icon: FileText,
    screen: MarkdownPage,
  },
  {
    id: "timestamp",
    nativeId: 11,
    en: "Timestamp Conversion",
    ja: "タイムスタンプ変換",
    detailEn: "Convert Unix and ISO-8601 values",
    detailJa: "Unix時刻とISO-8601形式を変換",
    icon: Clock,
    screen: TimestampPage,
  },
  {
    id: "regex",
    nativeId: 12,
    en: "Regex Tester",
    ja: "正規表現テスター",
    detailEn: "Test patterns, matches and replacement",
    detailJa: "パターン、マッチ、置換を確認",
    icon: Regex,
    screen: RegexPage,
  },
  {
    id: "diff",
    nativeId: 13,
    en: "Text Diff",
    ja: "テキスト差分",
    detailEn: "Compare two text versions",
    detailJa: "2つのテキストを比較",
    icon: GitCompareArrows,
    screen: DiffPage,
  },
];

const pageByNativeId = new Map(tools.map((tool) => [tool.nativeId, tool.id]));

export function pageForToolId(id: number): ToolId | undefined {
  return pageByNativeId.get(id);
}

export function toolForPage(page: PageId): ToolDefinition | undefined {
  return tools.find((tool) => tool.id === page);
}

export function titleFor(page: PageId, locale: Locale) {
  if (page === "welcome") return locale === "ja" ? "ホーム" : "Home";
  const tool = toolForPage(page);
  return tool ? (locale === "ja" ? tool.ja : tool.en) : page;
}

export function iconFor(page: PageId) {
  if (page === "welcome") return House;
  return toolForPage(page)?.icon ?? Settings;
}
