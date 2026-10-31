import { useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Empty, EmptyDescription, EmptyHeader } from "@/shared/components/ui/empty";
import { FormField } from "@/shared/components/form-field";
import { Input } from "@/shared/components/ui/input";
import { SelectControl } from "@/shared/components/select-control";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/shared/components/ui/tabs";
import { runUiEffect } from "@/shared/effect";
import type { HttpMethod, HttpResponse } from "@/shared/bridge/types";
import { Area, Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function HttpPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [method, setMethod] = useState<HttpMethod>("GET");
    const [url, setUrl] = useState("");
    const [headers, setHeaders] = useState("");
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [body, setBody] = useState("");
    const [parameters, setParameters] = useState<
        Array<{ key: string; value: string; description: string }>
    >(Array.from({ length: 10 }, () => ({ key: "", value: "", description: "" })));
    const [response, setResponse] = useState<HttpResponse | null>(null);
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
            <Card className="input-card">
                <CardContent>
                    <div className="http-request-row">
                        <FormField label={text(locale, "Method", "メソッド")} nativeLabel={false}>
                            <SelectControl
                                value={method}
                                onValueChange={setMethod}
                                options={[
                                    { value: "GET", label: "GET" },
                                    { value: "POST", label: "POST" },
                                    { value: "PUT", label: "PUT" },
                                    { value: "DELETE", label: "DELETE" },
                                ]}
                            />
                        </FormField>
                        <FormField label="URL">
                            <Input
                                value={url}
                                onChange={(event) => setUrl(event.target.value)}
                                placeholder="https://api.example.com/v1/items"
                            />
                        </FormField>
                        <Button onClick={() => runUiEffect(send())} disabled={busy}>
                            {busy
                                ? text(locale, "Sending…", "送信中…")
                                : text(locale, "Send", "送信")}
                        </Button>
                    </div>
                </CardContent>
            </Card>
            <div className="http-workspace">
                <Card className="http-tabs-card py-0">
                    <CardContent className="http-tabs-content p-0">
                        <Tabs
                            className="http-tabs"
                            value={httpTab}
                            onValueChange={(value) => setHttpTab(String(value))}
                        >
                            <TabsList variant="line">
                                <TabsTrigger value="parameters">
                                    {text(locale, "Parameters", "パラメーター")}
                                </TabsTrigger>
                                <TabsTrigger value="authentication">
                                    {text(locale, "Authentication", "認証")}
                                </TabsTrigger>
                                <TabsTrigger value="headers">
                                    {text(locale, "Headers", "ヘッダー")}
                                </TabsTrigger>
                                <TabsTrigger value="body">
                                    {text(locale, "Body", "本文")}
                                </TabsTrigger>
                            </TabsList>
                            <TabsContent className="http-tab-panel" value="parameters">
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
                                                                ? {
                                                                      ...item,
                                                                      key: event.target.value,
                                                                  }
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
                                                                ? {
                                                                      ...item,
                                                                      value: event.target.value,
                                                                  }
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
                                                                      description:
                                                                          event.target.value,
                                                                  }
                                                                : item,
                                                        ),
                                                    )
                                                }
                                            />
                                        </div>
                                    ))}
                                </div>
                            </TabsContent>
                            <TabsContent className="http-tab-panel" value="authentication">
                                <div className="auth-card">
                                    <FormField label={text(locale, "Username", "ユーザー名")}>
                                        <Input
                                            autoComplete="username"
                                            value={username}
                                            onChange={(event) => setUsername(event.target.value)}
                                        />
                                    </FormField>
                                    <FormField label={text(locale, "Password", "パスワード")}>
                                        <Input
                                            type="password"
                                            autoComplete="current-password"
                                            value={password}
                                            onChange={(event) => setPassword(event.target.value)}
                                        />
                                    </FormField>
                                </div>
                            </TabsContent>
                            <TabsContent className="http-tab-panel" value="headers">
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
                            </TabsContent>
                            <TabsContent className="http-tab-panel" value="body">
                                <Area
                                    label={text(locale, "Request body", "リクエスト本文")}
                                    value={body}
                                    onChange={setBody}
                                    rows={12}
                                />
                            </TabsContent>
                        </Tabs>
                    </CardContent>
                </Card>
                <Card role="region" className="input-card http-response-panel">
                    <CardHeader className="toolbar-row">
                        <CardTitle>{text(locale, "Response", "レスポンス")}</CardTitle>
                        {response && (
                            <span className="muted-text">
                                {String(response.status ?? "")} {String(response.reason ?? "")} ·{" "}
                                {String(response.elapsedMs ?? "")} ms ·{" "}
                                {String(response.bytes ?? "")} B
                            </span>
                        )}
                    </CardHeader>
                    <CardContent className="http-response-content">
                        {response ? (
                            <Area
                                label={text(locale, "Response body", "レスポンス本文")}
                                value={String(response.body ?? "")}
                                readOnly
                                rows={16}
                            />
                        ) : (
                            <Empty className="empty-state">
                                <EmptyHeader>
                                    <EmptyDescription>
                                        {text(
                                            locale,
                                            "No response yet.",
                                            "レスポンスはありません。",
                                        )}
                                    </EmptyDescription>
                                </EmptyHeader>
                            </Empty>
                        )}
                    </CardContent>
                </Card>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
