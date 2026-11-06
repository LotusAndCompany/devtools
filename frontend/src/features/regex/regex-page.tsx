import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Card, CardAction, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { FormField } from "@/shared/components/form-field";
import { Field, FieldGroup, FieldLabel, FieldLegend, FieldSet } from "@/shared/components/ui/field";
import { Empty, EmptyDescription, EmptyHeader } from "@/shared/components/ui/empty";
import { Input } from "@/shared/components/ui/input";
import {
    Item,
    ItemContent,
    ItemDescription,
    ItemGroup,
    ItemTitle,
} from "@/shared/components/ui/item";
import { SelectControl } from "@/shared/components/select-control";
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
                <Card className="input-card">
                    <CardHeader className="toolbar-row">
                        <FormField label={text(locale, "Preset", "プリセット")} nativeLabel={false}>
                            <SelectControl
                                className="w-auto min-w-[120px]"
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
                        </FormField>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(run("copyToClipboard", { text: pattern }))}
                        >
                            {text(locale, "Copy pattern", "パターンをコピー")}
                        </Button>
                    </CardHeader>
                    <CardContent className="regex-input-content">
                        <FormField label={text(locale, "Pattern", "パターン")}>
                            <Input
                                className="code-input"
                                value={pattern}
                                onChange={(event) => setPattern(event.target.value)}
                                placeholder="(\\w+)"
                            />
                        </FormField>
                        <FieldSet className="field-set">
                            <FieldLegend variant="label">
                                {text(locale, "Flags", "オプション")}
                            </FieldLegend>
                            <FieldGroup className="flag-grid">
                                {flags.map(([key, en, ja]) => (
                                    <Field key={key} orientation="horizontal">
                                        <Checkbox
                                            id={`regex-flag-${key}`}
                                            checked={options[key]}
                                            onCheckedChange={() => setOption(key)}
                                        />
                                        <FieldLabel
                                            htmlFor={`regex-flag-${key}`}
                                            className="font-normal"
                                        >
                                            {text(locale, en, ja)}
                                        </FieldLabel>
                                    </Field>
                                ))}
                            </FieldGroup>
                        </FieldSet>
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
                    </CardContent>
                </Card>
                <div className="regex-results">
                    <Card role="region" className="input-card">
                        <CardHeader className="toolbar-row">
                            <CardTitle>{text(locale, "Matches", "マッチ")}</CardTitle>
                            <span className="muted-text">{matches.length}</span>
                        </CardHeader>
                        <CardContent>
                            {matches.length === 0 ? (
                                <Empty className="regex-empty">
                                    <EmptyHeader>
                                        <EmptyDescription>
                                            {text(
                                                locale,
                                                "No matches yet.",
                                                "マッチ結果はありません。",
                                            )}
                                        </EmptyDescription>
                                    </EmptyHeader>
                                </Empty>
                            ) : (
                                <ItemGroup className="match-list">
                                    {matches.map((match, index) => (
                                        <Item
                                            key={index}
                                            variant="muted"
                                            size="sm"
                                            className="match-item"
                                        >
                                            <ItemContent>
                                                <ItemTitle className="match-value">
                                                    {match.value}
                                                </ItemTitle>
                                                <ItemDescription>
                                                    {text(locale, "Offset", "位置")} {match.offset}{" "}
                                                    · {text(locale, "Length", "長さ")}{" "}
                                                    {match.length}
                                                </ItemDescription>
                                                {match.groups.slice(1).map((group) => (
                                                    <code key={group.index}>
                                                        {group.name || group.index}: {group.value}
                                                    </code>
                                                ))}
                                            </ItemContent>
                                        </Item>
                                    ))}
                                </ItemGroup>
                            )}
                        </CardContent>
                    </Card>
                    <Card role="region" className="input-card">
                        <CardHeader>
                            <CardTitle>{text(locale, "Replacement", "置換")}</CardTitle>
                        </CardHeader>
                        <CardContent className="regex-replacement-content">
                            <FormField label={text(locale, "Replacement pattern", "置換文字列")}>
                                <Input
                                    className="code-input"
                                    value={replacement}
                                    onChange={(event) => setReplacement(event.target.value)}
                                    placeholder="$1"
                                />
                            </FormField>
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
                        </CardContent>
                    </Card>
                    <Card role="region" className="input-card">
                        <CardHeader className="toolbar-row">
                            <CardTitle>
                                {text(locale, "Quick reference", "クイックリファレンス")}
                            </CardTitle>
                            <CardAction>
                                <Button
                                    variant="ghost"
                                    onClick={() => setQuickReference((value) => !value)}
                                >
                                    {quickReference ? "−" : "+"}
                                </Button>
                            </CardAction>
                        </CardHeader>
                        {quickReference && (
                            <CardContent className="quick-reference">
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
                                        className="quick-reference-item h-auto justify-start gap-2 rounded-sm border-0 bg-muted p-2 text-left font-normal"
                                        onClick={() => insert(token)}
                                    >
                                        <code>{token}</code>
                                        <span>{description}</span>
                                    </Button>
                                ))}
                            </CardContent>
                        )}
                    </Card>
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
