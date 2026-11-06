import { Button } from "@/shared/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Empty, EmptyDescription, EmptyHeader } from "@/shared/components/ui/empty";
import {
    Item,
    ItemContent,
    ItemDescription,
    ItemGroup,
    ItemTitle,
} from "@/shared/components/ui/item";
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
        <Card role="complementary" className="input-card template-list">
            <CardHeader className="toolbar-row">
                <CardTitle>{text(locale, "Templates", "定型文一覧")}</CardTitle>
                <Button variant="outline" onClick={onNew}>
                    {text(locale, "New", "新規")}
                </Button>
            </CardHeader>
            <CardContent className="template-list-content">
                {templates.length === 0 ? (
                    <Empty className="template-empty">
                        <EmptyHeader>
                            <EmptyDescription>
                                {text(
                                    locale,
                                    "No saved templates.",
                                    "保存された定型文はありません。",
                                )}
                            </EmptyDescription>
                        </EmptyHeader>
                    </Empty>
                ) : (
                    <ItemGroup className="template-items">
                        {templates.map((item) => {
                            const selected = filename === item.filename;
                            return (
                                <Item
                                    key={String(item.filename)}
                                    variant="muted"
                                    size="sm"
                                    data-active={selected}
                                    render={
                                        <button
                                            type="button"
                                            aria-pressed={selected}
                                            onClick={() => onSelect(item)}
                                        />
                                    }
                                    className="template-item"
                                >
                                    <ItemContent>
                                        <ItemTitle>{String(item.title)}</ItemTitle>
                                        <ItemDescription>
                                            {String(item.content).slice(0, 72)}
                                        </ItemDescription>
                                    </ItemContent>
                                </Item>
                            );
                        })}
                    </ItemGroup>
                )}
            </CardContent>
        </Card>
    );
}
