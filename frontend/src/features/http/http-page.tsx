import { useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { Tabs } from "@/shared/components/ui/tabs";
import { runUiEffect } from "@/shared/effect";
import { Area, Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function HttpPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [method, setMethod] = useState("GET");
    const [url, setUrl] = useState("");
    const [headers, setHeaders] = useState("");
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [body, setBody] = useState("");
    const [parameters, setParameters] = useState<
        Array<{ key: string; value: string; description: string }>
    >(Array.from({ length: 10 }, () => ({ key: "", value: "", description: "" })));
    const [response, setResponse] = useState<Record<string, unknown> | null>(null);
    const [httpTab, setHttpTab] = useState("parameters");
    const send = () =>
        Effect.gen(function* () {
            const parsedHeaders: Record<string, string> = {};
            for (const line of headers.split(/\r?\n/)) {
                if (!line.trim()) continue;
                const separator = line.indexOf(":");
                if (separator <= 0) {
                    yield* Effect.sync(() =>
                        setError(
                            text(
                                locale,
                                "Each header must use the name: value format.",
                                "ヘッダーは name: value の形式で入力してください。",
                            ),
                        ),
                    );
                    return;
                }
                parsedHeaders[line.slice(0, separator).trim()] = line.slice(separator + 1).trim();
            }
            const result = yield* execute("sendHttpRequest", {
                method,
                url,
                headers: parsedHeaders,
                username,
                password,
                body,
                parameters: parameters.filter((parameter) => parameter.key.trim()),
            });
            if (result) yield* Effect.sync(() => setResponse(result));
        });
    return (
        <div className="tool-page">
            <Heading title={text(locale, "HTTP Request", "HTTPリクエスト")} />
            <section className="input-card">
                <div className="http-request-row">
                    <Field label={text(locale, "Method", "メソッド")}>
                        <SelectBox
                            value={method}
                            onValueChange={setMethod}
                            options={[
                                { value: "GET", label: "GET" },
                                { value: "POST", label: "POST" },
                                { value: "PUT", label: "PUT" },
                                { value: "DELETE", label: "DELETE" },
                            ]}
                        />
                    </Field>
                    <Field label="URL">
                        <Input
                            value={url}
                            onChange={(event) => setUrl(event.target.value)}
                            placeholder="https://api.example.com/v1/items"
                        />
                    </Field>
                    <Button onClick={() => runUiEffect(send())} disabled={busy}>
                        {busy ? text(locale, "Sending…", "送信中…") : text(locale, "Send", "送信")}
                    </Button>
                </div>
            </section>
            <div className="http-workspace">
                <Tabs.Root
                    className="input-card"
                    value={httpTab}
                    onValueChange={(value) => setHttpTab(String(value))}
                >
                    <Tabs.List className="feature-tabs-list">
                        <Tabs.Tab className="feature-tab" value="parameters">
                            {text(locale, "Parameters", "パラメーター")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="authentication">
                            {text(locale, "Authentication", "認証")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="headers">
                            {text(locale, "Headers", "ヘッダー")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="body">
                            {text(locale, "Body", "本文")}
                        </Tabs.Tab>
                    </Tabs.List>
                    <Tabs.Panel className="http-tab-panel" value="parameters">
                        <div className="http-parameter-grid">
                            <strong>{text(locale, "Key", "キー")}</strong>
                            <strong>{text(locale, "Value", "値")}</strong>
                            <strong>{text(locale, "Description", "説明")}</strong>
                            {parameters.map((parameter, index) => (
                                <div className="http-parameter-row" key={index}>
                                    <Input
                                        aria-label={`${text(locale, "Parameter key", "パラメーターキー")} ${index + 1}`}
                                        value={parameter.key}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? { ...item, key: event.target.value }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                    <Input
                                        aria-label={`${text(locale, "Parameter value", "パラメーター値")} ${index + 1}`}
                                        value={parameter.value}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? { ...item, value: event.target.value }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                    <Input
                                        aria-label={`${text(locale, "Parameter description", "パラメーター説明")} ${index + 1}`}
                                        value={parameter.description}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? {
                                                              ...item,
                                                              description: event.target.value,
                                                          }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                </div>
                            ))}
                        </div>
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="authentication">
                        <div className="auth-card">
                            <Field label={text(locale, "Username", "ユーザー名")}>
                                <Input
                                    autoComplete="username"
                                    value={username}
                                    onChange={(event) => setUsername(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Password", "パスワード")}>
                                <Input
                                    type="password"
                                    autoComplete="current-password"
                                    value={password}
                                    onChange={(event) => setPassword(event.target.value)}
                                />
                            </Field>
                        </div>
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="headers">
                        <Area
                            label={text(
                                locale,
                                "Headers (one name: value per line)",
                                "ヘッダー (1行に1つ name: value)",
                            )}
                            value={headers}
                            onChange={setHeaders}
                            rows={12}
                            placeholder="Accept: application/json"
                        />
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="body">
                        <Area
                            label={text(locale, "Request body", "リクエスト本文")}
                            value={body}
                            onChange={setBody}
                            rows={12}
                        />
                    </Tabs.Panel>
                </Tabs.Root>
                <section className="input-card http-response-panel">
                    <div className="toolbar-row">
                        <h2>{text(locale, "Response", "レスポンス")}</h2>
                        {response && (
                            <span className="muted-text">
                                {String(response.status ?? "")} {String(response.reason ?? "")} ·{" "}
                                {String(response.elapsedMs ?? "")} ms ·{" "}
                                {String(response.bytes ?? "")} B
                            </span>
                        )}
                    </div>
                    {response ? (
                        <Area
                            label={text(locale, "Response body", "レスポンス本文")}
                            value={String(response.body ?? "")}
                            readOnly
                            rows={16}
                        />
                    ) : (
                        <div className="empty-state">
                            {text(locale, "No response yet.", "レスポンスはありません。")}
                        </div>
                    )}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
            {response && (
                <section className="input-card response-card">
                    <div className="toolbar-row">
                        <h2>{text(locale, "Response", "レスポンス")}</h2>
                        <span className="muted-text">
                            {String(response.status ?? "")} {String(response.reason ?? "")} ·{" "}
                            {String(response.elapsedMs ?? "")} ms · {String(response.bytes ?? "")} B
                        </span>
                    </div>
                    <Area
                        label={text(locale, "Response body", "レスポンス本文")}
                        value={String(response.body ?? "")}
                        readOnly
                        rows={16}
                    />
                </section>
            )}
        </div>
    );
}
