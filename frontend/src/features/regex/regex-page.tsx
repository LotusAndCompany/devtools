import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import type { RegexMatch, RegexOptions } from "@/shared/bridge/types";
import { Area, Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function RegexPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [pattern, setPattern] = useState("");
    const [input, setInput] = useState("");
    const [replacement, setReplacement] = useState("");
    const [options, setOptions] = useState<RegexOptions>({
        global: true,
        caseInsensitive: false,
        multiline: false,
        dotMatchesEverything: false,
        useUnicodeProperties: false,
        extendedPatternSyntax: false,
    });
    const [matches, setMatches] = useState<RegexMatch[]>([]);
    const [replacedText, setReplacedText] = useState("");
    const [quickReference, setQuickReference] = useState(true);
    const [settingsLoaded, setSettingsLoaded] = useState(false);
    const evaluate = () =>
        execute("evaluateRegex", {
            pattern,
            text: input,
            replacement,
            ...options,
        }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (!result) return;
                    setMatches(result.matches);
                    setReplacedText(result.replacedText);
                }),
            ),
        );
    useEffect(() => {
        const loadSettings = run("regexSettings", { action: "load" }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (!("options" in result)) return;
                    setPattern(result.pattern);
                    setInput(result.testText);
                    setReplacement(result.replacePattern);
                    setOptions((current) => ({ ...current, ...result.options }));
                }),
            ),
            Effect.catch(() => Effect.void),
            Effect.ensuring(Effect.sync(() => setSettingsLoaded(true))),
        );
        return forkUiEffect(loadSettings);
    }, [run]);
    useEffect(() => {
        if (!settingsLoaded) return;
        const timer = window.setTimeout(() => {
            runUiEffect(
                run("regexSettings", {
                    action: "save",
                    pattern,
                    testText: input,
                    replacePattern: replacement,
                    options,
                }).pipe(Effect.catch(() => Effect.void)),
            );
        }, 400);
        return () => window.clearTimeout(timer);
    }, [pattern, input, replacement, options, settingsLoaded, run]);
    const setOption = (key: keyof RegexOptions) =>
        setOptions((current) => ({ ...current, [key]: !current[key] }));
    const copyMatches = () => {
        const plain = matches
            .map((match, index) => [index + 1, ": ", match.value, " @ ", match.offset].join(""))
            .join("\n");
        return run("copyToClipboard", { text: plain });
    };
    const presets = [
        { label: "Email", pattern: "[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}" },
        { label: "URL", pattern: "https?://[^\\s]+" },
        { label: "IPv4", pattern: "\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b" },
        { label: "ISO date", pattern: "\\b\\d{4}-\\d{2}-\\d{2}\\b" },
        { label: "Phone (E.164)", pattern: "\\+[1-9]\\d{1,14}" },
        {
            label: "UUID v4",
            pattern: "\\b[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}\\b",
        },
    ];
    const flags: Array<[keyof RegexOptions, string, string]> = [
        ["global", "Global", "すべて検索"],
        ["caseInsensitive", "Case insensitive", "大文字小文字を無視"],
        ["multiline", "Multiline", "複数行"],
        ["dotMatchesEverything", "Dot matches newline", "ドットを改行にも一致"],
        ["useUnicodeProperties", "Unicode properties", "Unicodeプロパティ"],
        ["extendedPatternSyntax", "Extended syntax", "拡張構文"],
    ];
    const insert = (value: string) => setPattern((current) => current + value);
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Regex Tester", "正規表現テスター")} />
            <div className="regex-layout">
                <div className="input-card">
                    <div className="toolbar-row">
                        <Field label={text(locale, "Preset", "プリセット")}>
                            <SelectBox
                                className="compact"
                                defaultValue=""
                                onValueChange={(value) => {
                                    const preset = presets.find((item) => item.label === value);
                                    if (preset) setPattern(preset.pattern);
                                }}
                                options={[
                                    {
                                        value: "",
                                        label: text(
                                            locale,
                                            "Select preset pattern…",
                                            "プリセットを選択…",
                                        ),
                                    },
                                    ...presets.map((preset) => ({
                                        value: preset.label,
                                        label: preset.label,
                                    })),
                                ]}
                            />
                        </Field>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(run("copyToClipboard", { text: pattern }))}
                        >
                            {text(locale, "Copy pattern", "パターンをコピー")}
                        </Button>
                    </div>
                    <Field label={text(locale, "Pattern", "パターン")}>
                        <Input
                            className="form-control code-input"
                            value={pattern}
                            onChange={(event) => setPattern(event.target.value)}
                            placeholder="(\\w+)"
                        />
                    </Field>
                    <div className="flag-grid">
                        {flags.map(([key, en, ja]) => (
                            <label key={key} className="inline-check">
                                <Checkbox
                                    checked={options[key]}
                                    onCheckedChange={() => setOption(key)}
                                />
                                {text(locale, en, ja)}
                            </label>
                        ))}
                    </div>
                    <Area
                        label={text(locale, "Test text", "テスト文字列")}
                        value={input}
                        onChange={setInput}
                        rows={14}
                    />
                    <div className="action-row">
                        <Button onClick={() => runUiEffect(evaluate())} disabled={busy}>
                            {text(locale, "Test pattern", "テスト")}
                        </Button>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(copyMatches())}
                            disabled={!matches.length}
                        >
                            {text(locale, "Copy matches", "マッチをコピー")}
                        </Button>
                    </div>
                </div>
                <div className="regex-results">
                    <section className="input-card">
                        <div className="toolbar-row">
                            <h2>{text(locale, "Matches", "マッチ")}</h2>
                            <span className="muted-text">{matches.length}</span>
                        </div>
                        {matches.length === 0 ? (
                            <p className="muted-text">
                                {text(locale, "No matches yet.", "マッチ結果はありません。")}
                            </p>
                        ) : (
                            <div className="match-list">
                                {matches.map((match, index) => (
                                    <div key={index} className="match-item">
                                        <strong>{match.value}</strong>
                                        <span>
                                            {text(locale, "Offset", "位置")} {match.offset} ·{" "}
                                            {text(locale, "Length", "長さ")} {match.length}
                                        </span>
                                        {match.groups.slice(1).map((group) => (
                                            <code key={group.index}>
                                                {group.name || group.index}: {group.value}
                                            </code>
                                        ))}
                                    </div>
                                ))}
                            </div>
                        )}
                    </section>
                    <section className="input-card">
                        <h2>{text(locale, "Replacement", "置換")}</h2>
                        <Field label={text(locale, "Replacement pattern", "置換文字列")}>
                            <Input
                                className="form-control code-input"
                                value={replacement}
                                onChange={(event) => setReplacement(event.target.value)}
                                placeholder="$1"
                            />
                        </Field>
                        <Area
                            label={text(locale, "Result", "結果")}
                            value={replacedText}
                            readOnly
                            rows={8}
                        />
                        <Button
                            variant="outline"
                            onClick={() =>
                                runUiEffect(run("copyToClipboard", { text: replacedText }))
                            }
                            disabled={!replacedText}
                        >
                            {text(locale, "Copy result", "結果をコピー")}
                        </Button>
                    </section>
                    <section className="input-card">
                        <div className="toolbar-row">
                            <h2>{text(locale, "Quick reference", "クイックリファレンス")}</h2>
                            <Button
                                variant="ghost"
                                onClick={() => setQuickReference((value) => !value)}
                            >
                                {quickReference ? "−" : "+"}
                            </Button>
                        </div>
                        {quickReference && (
                            <div className="quick-reference">
                                {[
                                    ["\\d", text(locale, "Digit", "数字")],
                                    ["\\w", text(locale, "Word character", "単語文字")],
                                    ["\\s", text(locale, "Whitespace", "空白")],
                                    [".", text(locale, "Any character", "任意の文字")],
                                    ["^", text(locale, "Start of line", "行頭")],
                                    ["$", text(locale, "End of line", "行末")],
                                    [
                                        "(?:…)",
                                        text(locale, "Non-capturing group", "非キャプチャグループ"),
                                    ],
                                    [
                                        "(?<name>…)",
                                        text(locale, "Named capture", "名前付きキャプチャ"),
                                    ],
                                ].map(([token, description]) => (
                                    <Button
                                        key={token}
                                        variant="ghost"
                                        className="quick-reference-item h-auto"
                                        onClick={() => insert(token)}
                                    >
                                        <code>{token}</code>
                                        <span>{description}</span>
                                    </Button>
                                ))}
                            </div>
                        )}
                    </section>
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
