import { text } from "@/shared/tool-page";
import {
    Table,
    TableBody,
    TableCaption,
    TableCell,
    TableHead,
    TableHeader,
    TableRow,
} from "@/shared/components/ui/table";
import type { Locale } from "@/shared/types";
import type { DatabaseQueryTab } from "./types";

interface QueryResultsProps {
    locale: Locale;
    query: DatabaseQueryTab;
}

export function QueryResults({ locale, query }: QueryResultsProps) {
    if (query.columns.length === 0) return null;

    return (
        <div className="table-wrap">
            <Table>
                <TableCaption className="sr-only">
                    {text(locale, "Query results", "クエリ結果")}
                </TableCaption>
                <TableHeader>
                    <TableRow>
                        {query.columns.map((column) => (
                            <TableHead key={column} scope="col">
                                {column}
                            </TableHead>
                        ))}
                    </TableRow>
                </TableHeader>
                <TableBody>
                    {query.rows.map((row, rowIndex) => (
                        <TableRow key={rowIndex}>
                            {row.map((value, columnIndex) => (
                                <TableCell key={columnIndex}>
                                    {value === null ? <em>NULL</em> : String(value)}
                                </TableCell>
                            ))}
                        </TableRow>
                    ))}
                </TableBody>
            </Table>
            {query.truncated && (
                <p className="muted-text">
                    {text(
                        locale,
                        "Showing the first 1,000 rows.",
                        "先頭の1,000行を表示しています。",
                    )}
                </p>
            )}
        </div>
    );
}
