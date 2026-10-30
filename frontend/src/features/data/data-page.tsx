import { useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Field } from "@/shared/components/ui/field";
import { SelectBox } from "@/shared/components/ui/select";
import { runUiEffect } from "@/shared/effect";
import { Area, Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function DataPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [input, setInput] = useState("");
    const [output, setOutput] = useState("");
    const [format, setFormat] = useState("json");
    const [indentation, setIndentation] = useState("2");
    const convert = () =>
        Effect.gen(function* () {
            const result = yield* execute("convertData", { input, format, indentation });
            if (result) yield* Effect.sync(() => setOutput(String(result.output ?? "")));
        });
    const open = () =>
        Effect.gen(function* () {
            const result = yield* execute("openTextFile", { fileType: "data" });
            if (result) yield* Effect.sync(() => setInput(String(result.text ?? "")));
        });
    const save = () =>
        execute("saveTextFile", {
            text: output,
            suggestedName: "converted." + (format.startsWith("yaml") ? "yaml" : format),
            fileType: "data",
        });
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Data Conversion", "データ変換")} />
            <div className="toolbar-row data-toolbar">
                <Button variant="outline" onClick={() => runUiEffect(open())} disabled={busy}>
                    {text(locale, "Open file", "ファイルを開く")}
                </Button>
                <Field label={text(locale, "Output format", "出力形式")}>
                    <SelectBox
                        className="compact"
                        value={format}
                        onValueChange={setFormat}
                        options={[
                            { value: "json", label: "JSON" },
                            { value: "yaml-block", label: "YAML (block)" },
                            { value: "yaml-flow", label: "YAML (flow)" },
                            { value: "toml", label: "TOML" },
                        ]}
                    />
                </Field>
                <Field label={text(locale, "Indentation", "インデント")}>
                    <SelectBox
                        className="compact"
                        value={indentation}
                        onValueChange={setIndentation}
                        options={[
                            { value: "2", label: "2" },
                            { value: "4", label: "4" },
                            { value: "tab", label: "Tab" },
                            { value: "minified", label: "Minified" },
                        ]}
                    />
                </Field>
                <Button onClick={() => runUiEffect(convert())} disabled={busy}>
                    {text(locale, "Convert", "変換")}
                </Button>
            </div>
            <div className="two-column">
                <Area
                    label={text(locale, "Input", "入力")}
                    value={input}
                    onChange={setInput}
                    placeholder={'{"name":"DevTools"}'}
                />
                <Area
                    label={text(locale, "Output", "出力")}
                    value={output}
                    onChange={setOutput}
                    readOnly={false}
                />
            </div>
            <div className="action-row">
                <Button
                    variant="outline"
                    onClick={() => runUiEffect(save())}
                    disabled={!output || busy}
                >
                    {text(locale, "Save output", "出力を保存")}
                </Button>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
