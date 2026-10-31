import { Button } from "@/shared/components/ui/button";
import { Card } from "@/shared/components/ui/card";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { Area, OutputMessage, text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface PhraseEditorProps {
    locale: Locale;
    title: string;
    content: string;
    busy: boolean;
    filename: string;
    error: string;
    message: string;
    onTitleChange: (value: string) => void;
    onContentChange: (value: string) => void;
    onSave: () => void;
    onCopy: () => void;
    onDelete: () => void;
    onDismissError: () => void;
}

export function PhraseEditor({
    locale,
    title,
    content,
    busy,
    filename,
    error,
    message,
    onTitleChange,
    onContentChange,
    onSave,
    onCopy,
    onDelete,
    onDismissError,
}: PhraseEditorProps) {
    return (
        <Card className="input-card">
            <Field label={text(locale, "Title", "タイトル")}>
                <Input
                    className="form-control"
                    value={title}
                    onChange={(event) => onTitleChange(event.target.value)}
                />
            </Field>
            <Area
                label={text(locale, "Template text", "テンプレート本文")}
                value={content}
                onChange={onContentChange}
                rows={16}
            />
            <div className="button-row">
                <Button onClick={onSave} disabled={busy}>
                    {text(locale, "Save template", "定型文を保存")}
                </Button>
                <Button variant="outline" onClick={onCopy} disabled={!content}>
                    {text(locale, "Copy text", "本文をコピー")}
                </Button>
                <Button variant="outline" onClick={onDelete} disabled={busy || !filename}>
                    {text(locale, "Delete", "削除")}
                </Button>
            </div>
            <OutputMessage
                locale={locale}
                error={error}
                message={message}
                onDismiss={onDismissError}
            />
        </Card>
    );
}
