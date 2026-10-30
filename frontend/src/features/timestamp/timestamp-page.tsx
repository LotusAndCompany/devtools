import { useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { runUiEffect } from "@/shared/effect";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

type TimestampValues = {
    seconds: string;
    milliseconds: string;
    local: string;
    utc: string;
    isoUtc: string;
    isoLocal: string;
};

const padDatePart = (value: number) => String(value).padStart(2, "0");

function toLocalInput(date: Date, utc = false) {
    const year = utc ? date.getUTCFullYear() : date.getFullYear();
    const month = padDatePart((utc ? date.getUTCMonth() : date.getMonth()) + 1);
    const day = padDatePart(utc ? date.getUTCDate() : date.getDate());
    const hour = padDatePart(utc ? date.getUTCHours() : date.getHours());
    const minute = padDatePart(utc ? date.getUTCMinutes() : date.getMinutes());
    return `${year}-${month}-${day}T${hour}:${minute}`;
}

function toLocalIso(date: Date) {
    const offset = -date.getTimezoneOffset();
    const sign = offset >= 0 ? "+" : "-";
    const absoluteOffset = Math.abs(offset);
    const seconds = padDatePart(date.getSeconds());
    const milliseconds = String(date.getMilliseconds()).padStart(3, "0");
    return `${toLocalInput(date)}:${seconds}.${milliseconds}${sign}${padDatePart(Math.floor(absoluteOffset / 60))}:${padDatePart(absoluteOffset % 60)}`;
}

function timestampValues(date: Date): TimestampValues {
    return {
        seconds: String(Math.floor(date.getTime() / 1000)),
        milliseconds: String(date.getTime()),
        local: toLocalInput(date),
        utc: toLocalInput(date, true),
        isoUtc: date.toISOString(),
        isoLocal: toLocalIso(date),
    };
}

export function TimestampPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [values, setValues] = useState<TimestampValues>(() => timestampValues(new Date()));
    const convert = (key: keyof TimestampValues) =>
        Effect.gen(function* () {
            const source =
                key === "seconds" ? "seconds" : key === "milliseconds" ? "milliseconds" : "iso";
            const localDate = new Date(values.local);
            const value =
                key === "local"
                    ? Number.isNaN(localDate.getTime())
                        ? values.local
                        : localDate.toISOString()
                    : key === "utc"
                      ? `${values.utc}:00Z`
                      : key === "isoUtc"
                        ? values.isoUtc
                        : key === "isoLocal"
                          ? values.isoLocal
                          : values[key];
            const result = yield* execute("convertTimestamp", { value, source });
            if (!result) return;
            const instant = new Date(String(result.utc));
            yield* Effect.sync(() =>
                setValues({
                    seconds: String(result.seconds ?? ""),
                    milliseconds: String(result.milliseconds ?? ""),
                    local: toLocalInput(instant),
                    utc: toLocalInput(instant, true),
                    isoUtc: String(result.utc ?? ""),
                    isoLocal: String(result.local ?? ""),
                }),
            );
        });
    const copy = (value: string) => run("copyToClipboard", { text: value });
    const row = (
        key: keyof TimestampValues,
        label: string,
        type: "text" | "datetime-local" = "text",
    ) => (
        <div className="timestamp-row" key={key}>
            <Field label={label}>
                <Input
                    type={type}
                    value={values[key]}
                    onChange={(event) =>
                        setValues((current) => ({ ...current, [key]: event.target.value }))
                    }
                />
            </Field>
            <Button
                variant="outline"
                size="sm"
                disabled={busy}
                onClick={() => runUiEffect(convert(key))}
            >
                {text(locale, "Convert", "変換")}
            </Button>
            <Button variant="ghost" size="sm" onClick={() => runUiEffect(copy(values[key]))}>
                {text(locale, "Copy", "コピー")}
            </Button>
        </div>
    );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Timestamp Conversion", "タイムスタンプ変換")} />
            <div className="timestamp-toolbar">
                <Button variant="outline" onClick={() => setValues(timestampValues(new Date()))}>
                    {text(locale, "Now", "現在時刻")}
                </Button>
            </div>
            <div className="timestamp-groups">
                <section className="input-card timestamp-group">
                    <h2>{text(locale, "Unix Timestamp", "Unixタイムスタンプ")}</h2>
                    {row("seconds", text(locale, "Seconds", "秒"))}
                    {row("milliseconds", text(locale, "Milliseconds", "ミリ秒"))}
                </section>
                <section className="input-card timestamp-group">
                    <h2>{text(locale, "Date & Time", "日時")}</h2>
                    {row("local", text(locale, "Local", "ローカル"), "datetime-local")}
                    {row("utc", text(locale, "UTC", "UTC"), "datetime-local")}
                </section>
                <section className="input-card timestamp-group">
                    <h2>ISO 8601</h2>
                    {row("isoUtc", text(locale, "UTC", "UTC"))}
                    {row("isoLocal", text(locale, "Local", "ローカル"))}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
