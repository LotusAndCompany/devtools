import { Button } from "@/shared/components/ui/button";
import { Tabs } from "@/shared/components/ui/tabs";
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
            <Tabs.Root
                className="db-query-tabs input-card"
                value={activeId}
                onValueChange={(value) => onActiveChange(String(value))}
            >
                <Tabs.List className="feature-tabs-list">
                    {tabs.map((queryTab) => (
                        <Tabs.Tab
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
                        </Tabs.Tab>
                    ))}
                </Tabs.List>
                {tabs.map((queryTab) => (
                    <Tabs.Panel key={queryTab.id} className="db-query-panel" value={queryTab.id}>
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
                    </Tabs.Panel>
                ))}
            </Tabs.Root>
        </div>
    );
}
