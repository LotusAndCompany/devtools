import { Button } from "@/shared/components/ui/button";
import { Card } from "@/shared/components/ui/card";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/shared/components/ui/tabs";
import { Area, text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";
import { QueryResults } from "./query-results";
import type { DatabaseQueryTab } from "./types";

interface QueryWorkspaceProps {
    locale: Locale;
    tabs: DatabaseQueryTab[];
    activeId: string;
    connected: boolean;
    busy: boolean;
    onActiveChange: (id: string) => void;
    onQueryChange: (id: string, sql: string) => void;
    onClose: (id: string) => void;
    onExecute: () => void;
}

export function QueryWorkspace({
    locale,
    tabs,
    activeId,
    connected,
    busy,
    onActiveChange,
    onQueryChange,
    onClose,
    onExecute,
}: QueryWorkspaceProps) {
    return (
        <div className="db-workspace">
            <Card className="db-query-card">
                <Tabs
                    className="db-query-tabs"
                    value={activeId}
                    onValueChange={(value) => onActiveChange(String(value))}
                >
                    <TabsList>
                        {tabs.map((queryTab) => (
                            <TabsTrigger
                                key={queryTab.id}
                                className="feature-tab db-query-tab"
                                value={queryTab.id}
                            >
                                <span>{queryTab.title}</span>
                                {tabs.length > 1 && (
                                    <span
                                        className="db-query-tab-close"
                                        role="button"
                                        tabIndex={0}
                                        aria-label={text(locale, "Close query", "クエリを閉じる")}
                                        onClick={(event) => {
                                            event.stopPropagation();
                                            onClose(queryTab.id);
                                        }}
                                        onKeyDown={(event) => {
                                            if (event.key === "Enter" || event.key === " ") {
                                                event.preventDefault();
                                                event.stopPropagation();
                                                onClose(queryTab.id);
                                            }
                                        }}
                                    >
                                        ×
                                    </span>
                                )}
                            </TabsTrigger>
                        ))}
                    </TabsList>
                    {tabs.map((queryTab) => (
                        <TabsContent
                            key={queryTab.id}
                            className="db-query-panel"
                            value={queryTab.id}
                        >
                            <Area
                                label="SQL"
                                value={queryTab.sql}
                                onChange={(sql) => onQueryChange(queryTab.id, sql)}
                                rows={9}
                            />
                            <div className="action-row">
                                <Button onClick={onExecute} disabled={!connected || busy}>
                                    {text(locale, "Execute", "実行")}
                                </Button>
                            </div>
                            <QueryResults locale={locale} query={queryTab} />
                        </TabsContent>
                    ))}
                </Tabs>
            </Card>
        </div>
    );
}
