export type DatabaseQueryTab = {
  id: string;
  title: string;
  sql: string;
  columns: string[];
  rows: unknown[][];
  truncated: boolean;
};

export type DatabaseHistoryEntry = Record<string, unknown>;
