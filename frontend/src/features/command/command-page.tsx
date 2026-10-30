import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { Textarea } from "@/shared/components/ui/textarea";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import type { CommandFunction } from "@/shared/bridge/types";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function CommandPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [category, setCategory] = useState<"git" | "docker" | "compose">("git");
    const [functions, setFunctions] = useState<CommandFunction[]>([]);
    const [functionIndex, setFunctionIndex] = useState(0);
    const [optionIndex, setOptionIndex] = useState(0);
    const [value, setValue] = useState("");
    const [command, setCommand] = useState("");
    useEffect(() => {
        const loadFunctions = run("commandData", { action: "list", category }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => setFunctions("functions" in result ? result.functions : [])),
            ),
            Effect.catch(() => Effect.sync(() => setFunctions([]))),
        );
        return forkUiEffect(loadFunctions);
    }, [category, run]);
    const options = functions[functionIndex]?.options ?? [];
    const generate = () =>
        execute("commandData", {
            action: "generate",
            category,
            functionIndex,
            optionIndex,
            value,
        }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => setCommand(result && "command" in result ? result.command : "")),
            ),
        );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Command Generation", "コマンド生成")} />
            <div className="input-card">
                <div className="form-grid">
                    <Field label={text(locale, "Category", "カテゴリ")}>
                        <SelectBox
                            value={category}
                            onValueChange={(nextCategory) => {
                                setCategory(nextCategory);
                                setFunctionIndex(0);
                                setOptionIndex(0);
                            }}
                            options={[
                                { value: "git", label: "Git" },
                                { value: "docker", label: "Docker" },
                                { value: "compose", label: "Docker Compose" },
                            ]}
                        />
                    </Field>
                    <Field label={text(locale, "Command", "コマンド")}>
                        <SelectBox
                            value={String(functionIndex)}
                            onValueChange={(nextIndex) => {
                                setFunctionIndex(Number(nextIndex));
                                setOptionIndex(0);
                            }}
                            options={functions.map((item, index) => ({
                                value: String(index),
                                label: String(item.name),
                            }))}
                        />
                    </Field>
                    <Field label={text(locale, "Option", "オプション")}>
                        <SelectBox
                            value={String(optionIndex)}
                            onValueChange={(nextIndex) => setOptionIndex(Number(nextIndex))}
                            options={options.map((item, index) => ({
                                value: String(index),
                                label: `${String(item.name)}${item.title ? ` — ${String(item.title)}` : ""}`,
                            }))}
                        />
                    </Field>
                    {Boolean(options[optionIndex]?.required || options[optionIndex]?.quotes) && (
                        <Field
                            label={String(
                                options[optionIndex]?.title || text(locale, "Value", "値"),
                            )}
                        >
                            <Input
                                className="form-control"
                                value={value}
                                onChange={(event) => setValue(event.target.value)}
                            />
                        </Field>
                    )}
                </div>
                <div className="action-row">
                    <Button
                        variant="outline"
                        onClick={() => {
                            setCategory("git");
                            setFunctionIndex(0);
                            setOptionIndex(0);
                            setValue("");
                        }}
                        disabled={busy}
                    >
                        {text(locale, "Reset", "リセット")}
                    </Button>
                    <Button
                        onClick={() => runUiEffect(generate())}
                        disabled={busy || !functions.length}
                    >
                        {text(locale, "Generate command", "コマンドを作成")}
                    </Button>
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
            <section className="input-card command-output">
                <div className="toolbar-row">
                    <h2>{text(locale, "Generated Result", "生成結果")}</h2>
                    <Button
                        variant="outline"
                        disabled={!command}
                        onClick={() => runUiEffect(run("copyToClipboard", { text: command }))}
                    >
                        {text(locale, "Copy", "コピー")}
                    </Button>
                    <Button variant="ghost" onClick={() => setCommand("")} disabled={!command}>
                        {text(locale, "Clear", "クリア")}
                    </Button>
                </div>
                <Textarea value={command} readOnly rows={5} />
            </section>
        </div>
    );
}
