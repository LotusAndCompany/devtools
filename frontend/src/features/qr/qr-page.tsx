import { useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { DateTimeInput } from "@/shared/components/ui/date-time-input";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { runUiEffect } from "@/shared/effect";
import type { QrCodeType } from "@/shared/bridge/types";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

const qrFields: Record<string, Array<[string, string, boolean?]>> = {
    text: [["text_content", "Text", true]],
    url: [["url_content", "URL", true]],
    email: [["email_address", "Email", true]],
    phone: [["phone_number", "Phone number", true]],
    sms: [
        ["sms_phone", "Phone number", true],
        ["sms_message", "Message", true],
    ],
    wifi: [
        ["wifi_ssid", "SSID", true],
        ["wifi_password", "Password"],
    ],
    contact: [
        ["contact_name", "Name", true],
        ["contact_phone", "Phone"],
        ["contact_email", "Email"],
    ],
    calendar: [
        ["cal_summary", "Event title", true],
        ["cal_start", "Start (ISO-8601)", true],
        ["cal_end", "End (ISO-8601)", true],
    ],
    geo: [
        ["geo_lat", "Latitude", true],
        ["geo_lng", "Longitude", true],
    ],
};

export function QrPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [type, setType] = useState<QrCodeType>("text");
    const [parameters, setParameters] = useState<Record<string, string>>({ text_content: "" });
    const [generatedInput, setGeneratedInput] = useState<{
        type: QrCodeType;
        parameters: Record<string, string>;
    } | null>(null);
    const [content, setContent] = useState("");
    const [image, setImage] = useState("");
    const [message, setMessage] = useState("");
    const fields = qrFields[type] ?? [];
    const generate = () =>
        Effect.gen(function* () {
            yield* Effect.sync(() => setMessage(""));
            const result = yield* execute("generateQr", { type, parameters });
            if (result) {
                yield* Effect.sync(() => {
                    setContent(String(result.content ?? ""));
                    setImage(String(result.imageDataUrl ?? ""));
                    setGeneratedInput({ type, parameters: { ...parameters } });
                });
            }
        });
    const clear = () => {
        setParameters(
            type === "text"
                ? { text_content: "" }
                : type === "wifi"
                  ? { wifi_security: "WPA", wifi_hidden: "false" }
                  : {},
        );
        setGeneratedInput(null);
        setContent("");
        setImage("");
        setMessage("");
    };
    const copy = () =>
        execute("copyToClipboard", { imageDataUrl: image }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (result)
                        setMessage(text(locale, "QR image copied.", "QR画像をコピーしました。"));
                }),
            ),
        );
    const save = () =>
        generatedInput
            ? execute("saveQrImage", generatedInput).pipe(
                  Effect.tap((result) =>
                      Effect.sync(() => {
                          if (result)
                              setMessage(
                                  text(
                                      locale,
                                      `Saved ${String(result.fileName ?? "QR image")}.`,
                                      `${String(result.fileName ?? "QR画像")}を保存しました。`,
                                  ),
                              );
                      }),
                  ),
              )
            : Effect.void;
    return (
        <div className="tool-page">
            <Heading title={text(locale, "QR Code Generation", "QRコード生成")} />
            <div className="qr-layout">
                <div className="input-card">
                    <Field label={text(locale, "QR type", "QRの種類")}>
                        <SelectBox
                            value={type}
                            onValueChange={(value) => {
                                setType(value);
                                setParameters(
                                    value === "text"
                                        ? { text_content: "" }
                                        : value === "wifi"
                                          ? { wifi_security: "WPA", wifi_hidden: "false" }
                                          : {},
                                );
                                setGeneratedInput(null);
                                setContent("");
                                setImage("");
                                setMessage("");
                            }}
                            options={[
                                { value: "text", label: "Text" },
                                { value: "url", label: "URL" },
                                { value: "email", label: "Email" },
                                { value: "phone", label: "Phone" },
                                { value: "sms", label: "SMS" },
                                { value: "wifi", label: "Wi-Fi" },
                                { value: "contact", label: "Contact" },
                                { value: "calendar", label: "Calendar" },
                                { value: "geo", label: "Location" },
                            ]}
                        />
                    </Field>
                    {fields.map(([key, label, required]) => (
                        <Field key={key} label={`${label}${required ? " *" : ""}`}>
                            {key.startsWith("cal_") ? (
                                <DateTimeInput
                                    locale={locale}
                                    value={parameters[key] ?? ""}
                                    onValueChange={(value) =>
                                        setParameters((current) => ({ ...current, [key]: value }))
                                    }
                                    aria-label={label}
                                />
                            ) : (
                                <Input
                                    type={key === "wifi_password" ? "password" : "text"}
                                    value={parameters[key] ?? ""}
                                    onChange={(event) =>
                                        setParameters((current) => ({
                                            ...current,
                                            [key]: event.target.value,
                                        }))
                                    }
                                />
                            )}
                        </Field>
                    ))}
                    {type === "wifi" && (
                        <>
                            <Field label={text(locale, "Security", "セキュリティ")}>
                                <SelectBox
                                    value={parameters.wifi_security ?? "WPA"}
                                    onValueChange={(wifiSecurity) =>
                                        setParameters((current) => ({
                                            ...current,
                                            wifi_security: wifiSecurity,
                                        }))
                                    }
                                    options={[
                                        { value: "WPA", label: "WPA" },
                                        { value: "WEP", label: "WEP" },
                                        { value: "nopass", label: text(locale, "None", "なし") },
                                    ]}
                                />
                            </Field>
                            <label className="inline-check">
                                <Checkbox
                                    checked={parameters.wifi_hidden === "true"}
                                    onCheckedChange={(checked) =>
                                        setParameters((current) => ({
                                            ...current,
                                            wifi_hidden: String(checked),
                                        }))
                                    }
                                />
                                {text(locale, "Hidden network", "非公開ネットワーク")}
                            </label>
                        </>
                    )}
                    <div className="button-row">
                        <Button onClick={() => runUiEffect(generate())} disabled={busy}>
                            {text(locale, "Generate", "生成")}
                        </Button>
                        <Button variant="outline" onClick={clear} disabled={busy}>
                            {text(locale, "Clear", "クリア")}
                        </Button>
                    </div>
                </div>
                <div className="qr-preview input-card">
                    {image ? (
                        <img
                            src={image}
                            alt={text(locale, "Generated QR code", "生成したQRコード")}
                        />
                    ) : (
                        <span>{text(locale, "QR code preview", "QRコードのプレビュー")}</span>
                    )}
                    {content && (
                        <>
                            <code>{content}</code>
                            <div className="button-row">
                                <Button
                                    variant="outline"
                                    onClick={() => runUiEffect(copy())}
                                    disabled={busy}
                                >
                                    {text(locale, "Copy image", "画像をコピー")}
                                </Button>
                                <Button
                                    variant="outline"
                                    onClick={() => runUiEffect(save())}
                                    disabled={busy}
                                >
                                    {text(locale, "Save image", "画像を保存")}
                                </Button>
                            </div>
                        </>
                    )}
                </div>
            </div>
            <OutputMessage
                locale={locale}
                error={error}
                message={message}
                onDismiss={() => setError("")}
            />
        </div>
    );
}
