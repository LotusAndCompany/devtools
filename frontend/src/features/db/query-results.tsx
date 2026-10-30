import { text } from "@/shared/tool-page";
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
            <table>
                <thead>
                    <tr>
                        {query.columns.map((column) => (
                            <th key={column}>{column}</th>
                        ))}
                    </tr>
                </thead>
                <tbody>
                    {query.rows.map((row, rowIndex) => (
                        <tr key={rowIndex}>
                            {row.map((value, columnIndex) => (
                                <td key={columnIndex}>
                                    {value === null ? <em>NULL</em> : String(value)}
                                </td>
                            ))}
                        </tr>
                    ))}
                </tbody>
            </table>
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
