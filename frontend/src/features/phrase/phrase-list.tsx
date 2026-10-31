import { Button } from "@/shared/components/ui/button";
import { Card } from "@/shared/components/ui/card";
import type { PhraseTemplate } from "@/shared/bridge/types";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface PhraseListProps {
    locale: Locale;
    templates: PhraseTemplate[];
    filename: string;
    onNew: () => void;
    onSelect: (template: PhraseTemplate) => void;
}

export function PhraseList({ locale, templates, filename, onNew, onSelect }: PhraseListProps) {
    return (
        <Card as="aside" className="input-card template-list">
            <div className="toolbar-row">
                <h2>{text(locale, "Templates", "定型文一覧")}</h2>
                <Button variant="outline" onClick={onNew}>
                    {text(locale, "New", "新規")}
                </Button>
            </div>
            {templates.length === 0 && (
                <p className="muted-text">
                    {text(locale, "No saved templates.", "保存された定型文はありません。")}
                </p>
            )}
            {templates.map((item) => (
                <Button
                    key={String(item.filename)}
                    variant="ghost"
                    className={`template-item ${filename === item.filename ? "active" : ""}`}
                    onClick={() => onSelect(item)}
                >
                    <strong>{String(item.title)}</strong>
                    <span>{String(item.content).slice(0, 72)}</span>
                </Button>
            ))}
        </Card>
    );
}
