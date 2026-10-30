import { useState } from "react";
import { Effect } from "effect";
import { Field } from "@/shared/components/ui/field";
import { MessageDialog } from "@/shared/components/ui/message-dialog";
import { Textarea } from "@/shared/components/ui/textarea";
import {
    OperationCancelledError,
    type OperationRunner,
    type OperationName,
} from "@/shared/bridge/types";
import type { Locale } from "@/shared/types";

export const text = (locale: Locale, en: string, ja: string) => (locale === "ja" ? ja : en);

export function useOperation(run: OperationRunner) {
    const [busy, setBusy] = useState(false);
    const [error, setError] = useState("");
    const execute = (operation: OperationName, payload: Record<string, unknown>) =>
        Effect.gen(function* () {
            yield* Effect.sync(() => {
                setBusy(true);
                setError("");
            });
            return yield* run(operation, payload);
        }).pipe(
            Effect.catch((reason) =>
                Effect.sync(() => {
                    if (reason instanceof OperationCancelledError) return null;
                    setError(reason instanceof Error ? reason.message : String(reason));
                    return null;
                }),
            ),
            Effect.ensuring(Effect.sync(() => setBusy(false))),
        );
    return { busy, error, execute, setError };
}

export function Heading({ title }: { title: string }) {
    return (
        <header className="page-heading sr-only">
            <h1>{title}</h1>
        </header>
    );
}

export function Area({
    label,
    value,
    onChange,
    placeholder,
    rows = 12,
    readOnly = false,
}: {
    label: string;
    value: string;
    onChange?: (value: string) => void;
    placeholder?: string;
    rows?: number;
    readOnly?: boolean;
}) {
    return (
        <Field label={label}>
            <Textarea
                className="form-control code-area"
                rows={rows}
                value={value}
                onChange={(event) => onChange?.(event.target.value)}
                placeholder={placeholder}
                readOnly={readOnly}
                spellCheck={false}
            />
        </Field>
    );
}

export function OutputMessage({
    locale,
    error,
    message,
    onDismiss,
}: {
    locale: Locale;
    error: string;
    message?: string;
    onDismiss: () => void;
}) {
    return (
        <>
            {message && !error && (
                <p className="operation-message success" role="status">
                    {message}
                </p>
            )}
            <MessageDialog
                open={Boolean(error)}
                onOpenChange={(open) => {
                    if (!open) onDismiss();
                }}
                title={text(locale, "Error", "エラー")}
                description={error}
                closeLabel={text(locale, "OK", "閉じる")}
            />
        </>
    );
}
