import type { DatabaseHistoryEntry } from "@/shared/bridge/types";

export type DatabaseQueryTab = {
  id: string;
  title: string;
  sql: string;
  columns: string[];
  rows: Array<Array<string | null>>;
  truncated: boolean;
};

export type { DatabaseHistoryEntry };
