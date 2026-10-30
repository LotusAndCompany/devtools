import { Button } from "@/shared/components/ui/button";
import { Icon } from "@/shared/components/ui/icon";
import { titleFor, type PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type TopbarProps = {
    locale: Locale;
    activePage: PageId;
    pendingCount: number;
    onToggleSidebar: () => void;
    onCancelOperations: () => void;
};

export function Topbar({
    locale,
    activePage,
    pendingCount,
    onToggleSidebar,
    onCancelOperations,
}: TopbarProps) {
    return (
        <header className="app-topbar">
            <Button
                variant="ghost"
                aria-label={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"}
                onClick={onToggleSidebar}
            >
                <Icon>menu</Icon>
            </Button>
            <div className="topbar-title">{titleFor(activePage, locale)}</div>
            {pendingCount > 0 && (
                <Button variant="outline" onClick={onCancelOperations}>
                    {locale === "ja" ? "キャンセル" : "Cancel"} ({pendingCount})
                </Button>
            )}
        </header>
    );
}
