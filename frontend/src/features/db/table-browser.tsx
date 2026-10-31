import { Table2 } from "lucide-react";
import { Button } from "@/shared/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Empty, EmptyDescription, EmptyHeader, EmptyTitle } from "@/shared/components/ui/empty";
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
        <Card role="complementary" className="input-card db-sidebar">
            <CardHeader>
                <CardTitle>{text(locale, "Tables", "テーブル")}</CardTitle>
            </CardHeader>
            <CardContent className="db-table-list">
                {tables.length === 0 ? (
                    <Empty className="db-empty">
                        <EmptyHeader>
                            <EmptyTitle>
                                {connected
                                    ? text(locale, "No tables", "テーブルがありません")
                                    : text(locale, "Not connected", "未接続")}
                            </EmptyTitle>
                            {!connected && (
                                <EmptyDescription>
                                    {text(
                                        locale,
                                        "Connect to a database to view tables.",
                                        "接続するとテーブルを表示します。",
                                    )}
                                </EmptyDescription>
                            )}
                        </EmptyHeader>
                    </Empty>
                ) : (
                    tables.map((table) => (
                        <Button
                            key={table}
                            variant="ghost"
                            size="sm"
                            className="db-table-item"
                            onClick={() => onSelectTable(table)}
                        >
                            <Table2 aria-hidden="true" />
                            {table}
                        </Button>
                    ))
                )}
            </CardContent>
        </Card>
    );
}
