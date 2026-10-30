import { useEffect, useState } from "react";
import { Effect } from "effect";
import { SelectBox } from "@/shared/components/ui/select";
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
                    <section className="input-card">
                        <Area
                            label={text(locale, "Original", "変更前")}
                            value={left}
                            onChange={setLeft}
                        />
                    </section>
                    <section className="input-card">
                        <Area
                            label={text(locale, "Modified", "変更後")}
                            value={right}
                            onChange={setRight}
                        />
                    </section>
                </div>
                <section className="input-card">
                    <div className="diff-result-heading">
                        <h2>{text(locale, "Diff", "差分")}</h2>
                        <SelectBox
                            className="compact"
                            value={mode}
                            onValueChange={setMode}
                            options={[
                                { value: "inline", label: text(locale, "Inline", "インライン") },
                                {
                                    value: "side-by-side",
                                    label: text(locale, "Side-by-side", "左右に並べる"),
                                },
                            ]}
                        />
                    </div>
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
                        <div
                            className="diff-table"
                            role="table"
                            aria-label={text(locale, "Diff result", "差分結果")}
                        >
                            {rows.map((row, index) => (
                                <div key={index} className="diff-row">
                                    <code className={row.leftChanged ? "removed" : ""}>
                                        {row.left}
                                    </code>
                                    <code className={row.rightChanged ? "added" : ""}>
                                        {row.right}
                                    </code>
                                </div>
                            ))}
                        </div>
                    )}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
