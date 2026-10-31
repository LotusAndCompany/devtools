import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { FormField } from "@/shared/components/form-field";
import { SelectControl } from "@/shared/components/select-control";
import {
    Table,
    TableBody,
    TableCell,
    TableHead,
    TableHeader,
    TableRow,
} from "@/shared/components/ui/table";
import { runUiEffect } from "@/shared/effect";
import type { OperationOutput } from "@/shared/bridge/types";
import { Area, Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function DiffPage({ locale, run }: ToolPageProps) {
    const { error, setError } = useOperation(run);
    const [left, setLeft] = useState("");
    const [right, setRight] = useState("");
    const [rows, setRows] = useState<OperationOutput<"compareText">["rows"]>([]);
    const [mode, setMode] = useState("inline");
    useEffect(() => {
        const timer = window.setTimeout(() => {
            runUiEffect(
                run("compareText", { left, right }).pipe(
                    Effect.tap((result) =>
                        Effect.sync(() => {
                            setRows(result.rows);
                            setError("");
                        }),
                    ),
                    Effect.catch((reason) =>
                        Effect.sync(() =>
                            setError(reason instanceof Error ? reason.message : String(reason)),
                        ),
                    ),
                ),
            );
        }, 150);
        return () => window.clearTimeout(timer);
    }, [left, right, run, setError]);
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Text Diff", "テキスト差分")} />
            <div className="diff-layout">
                <div className="diff-editors">
                    <Card className="input-card">
                        <CardContent>
                            <Area
                                label={text(locale, "Original", "変更前")}
                                value={left}
                                onChange={setLeft}
                            />
                        </CardContent>
                    </Card>
                    <Card className="input-card">
                        <CardContent>
                            <Area
                                label={text(locale, "Modified", "変更後")}
                                value={right}
                                onChange={setRight}
                            />
                        </CardContent>
                    </Card>
                </div>
                <Card role="region" className="input-card">
                    <CardHeader className="diff-result-heading">
                        <CardTitle>{text(locale, "Diff", "差分")}</CardTitle>
                        <FormField
                            label={text(locale, "View", "表示")}
                            className="diff-view-field"
                            nativeLabel={false}
                        >
                            <SelectControl
                                className="w-auto min-w-[120px]"
                                value={mode}
                                onValueChange={setMode}
                                options={[
                                    {
                                        value: "inline",
                                        label: text(locale, "Inline", "インライン"),
                                    },
                                    {
                                        value: "side-by-side",
                                        label: text(locale, "Side-by-side", "左右に並べる"),
                                    },
                                ]}
                            />
                        </FormField>
                    </CardHeader>
                    <CardContent>
                        {mode === "inline" ? (
                            <div className="diff-table diff-inline">
                                {rows.map((row, index) => {
                                    const leftText = row.left;
                                    const rightText = row.right;
                                    if (row.leftChanged && row.rightChanged) {
                                        return (
                                            <div className="diff-row" key={index}>
                                                <code className="removed">− {leftText}</code>
                                                <code className="added">+ {rightText}</code>
                                            </div>
                                        );
                                    }
                                    const changed = Boolean(row.leftChanged || row.rightChanged);
                                    return (
                                        <div className="diff-row" key={index}>
                                            <code className={changed ? "added" : ""}>
                                                {changed ? "+ " : "  "}
                                                {rightText || leftText}
                                            </code>
                                        </div>
                                    );
                                })}
                            </div>
                        ) : (
                            <div className="diff-table">
                                <Table aria-label={text(locale, "Diff result", "差分結果")}>
                                    <TableHeader>
                                        <TableRow>
                                            <TableHead scope="col">
                                                {text(locale, "Original", "変更前")}
                                            </TableHead>
                                            <TableHead scope="col">
                                                {text(locale, "Modified", "変更後")}
                                            </TableHead>
                                        </TableRow>
                                    </TableHeader>
                                    <TableBody>
                                        {rows.map((row, index) => (
                                            <TableRow className="diff-row" key={index}>
                                                <TableCell>
                                                    <code
                                                        className={row.leftChanged ? "removed" : ""}
                                                    >
                                                        {row.left}
                                                    </code>
                                                </TableCell>
                                                <TableCell>
                                                    <code
                                                        className={row.rightChanged ? "added" : ""}
                                                    >
                                                        {row.right}
                                                    </code>
                                                </TableCell>
                                            </TableRow>
                                        ))}
                                    </TableBody>
                                </Table>
                            </div>
                        )}
                    </CardContent>
                </Card>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
