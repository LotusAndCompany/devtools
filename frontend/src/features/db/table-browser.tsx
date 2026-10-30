import { Button } from "@/shared/components/ui/button";
import { Card } from "@/shared/components/ui/card";
import { Icon } from "@/shared/components/ui/icon";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface TableBrowserProps {
    locale: Locale;
    tables: string[];
    connected: boolean;
    onSelectTable: (table: string) => void;
}

export function TableBrowser({ locale, tables, connected, onSelectTable }: TableBrowserProps) {
    return (
        <Card as="aside" className="input-card db-sidebar">
            <h2>{text(locale, "Tables", "テーブル")}</h2>
            <div className="db-table-list">
                {tables.map((table) => (
                    <Button
                        key={table}
                        variant="ghost"
                        size="sm"
                        className="db-table-item"
                        onClick={() => onSelectTable(table)}
                    >
                        <Icon>table_chart</Icon>
                        {table}
                    </Button>
                ))}
                {tables.length === 0 && (
                    <span className="muted-text">
                        {connected
                            ? text(locale, "No tables found.", "テーブルがありません。")
                            : text(
                                  locale,
                                  "Connect to a database to view tables.",
                                  "接続するとテーブルを表示します。",
                              )}
                    </span>
                )}
            </div>
        </Card>
    );
}
