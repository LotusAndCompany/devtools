import { Button } from "@/shared/components/ui/button";
import { Icon } from "@/shared/components/ui/icon";
import { titleFor, type PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type TopbarProps = {
    locale: Locale;
    activePage: PageId;
    onToggleSidebar: () => void;
};

export function Topbar({ locale, activePage, onToggleSidebar }: TopbarProps) {
    return (
        <header data-slot="app-topbar" className="app-topbar">
            <Button
                variant="ghost"
                aria-label={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"}
                onClick={onToggleSidebar}
            >
                <Icon>menu</Icon>
            </Button>
            <div data-slot="app-topbar-title" className="topbar-title">
                {titleFor(activePage, locale)}
            </div>
        </header>
    );
}
