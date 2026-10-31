import type { OperationRunner } from "@/shared/bridge/types";

export type Locale = "en" | "ja";

export type ToolId =
  | "image"
  | "phrase"
  | "command"
  | "http"
  | "data"
  | "db"
  | "qr"
  | "markdown"
  | "timestamp"
  | "regex"
  | "diff";

export type DroppedImage = {
  id: string;
  imageDataUrl: string;
  fileName: string;
  width: number;
  height: number;
};

export type ToolPageProps = {
  locale: Locale;
  run: OperationRunner;
  active?: boolean;
  databaseNavigationVersion?: number;
  droppedImage?: DroppedImage | null;
  onDroppedImageConsumed?: (id: string) => void;
};
