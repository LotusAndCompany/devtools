import { Button } from "@/shared/components/ui/button";
import { Card, CardContent } from "@/shared/components/ui/card";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/shared/components/ui/tabs";
import { X } from "lucide-react";
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
                <CardContent className="db-query-card-content">
                    <Tabs
                        className="db-query-tabs"
                        value={activeId}
                        onValueChange={(value) => onActiveChange(String(value))}
                    >
                        <div className="db-query-tabs-header">
                            <TabsList>
                                {tabs.map((queryTab) => (
                                    <TabsTrigger
                                        key={queryTab.id}
                                        className="feature-tab db-query-tab"
                                        value={queryTab.id}
                                    >
                                        {queryTab.title}
                                    </TabsTrigger>
                                ))}
                            </TabsList>
                            {tabs.length > 1 && (
                                <Button
                                    variant="ghost"
                                    size="icon-sm"
                                    aria-label={text(
                                        locale,
                                        "Close active query",
                                        "選択中のクエリを閉じる",
                                    )}
                                    title={text(
                                        locale,
                                        "Close active query",
                                        "選択中のクエリを閉じる",
                                    )}
                                    onClick={() => onClose(activeId)}
                                >
                                    <X aria-hidden="true" />
                                </Button>
                            )}
                        </div>
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
                </CardContent>
            </Card>
        </div>
    );
}
