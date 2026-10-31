import { Menu } from "lucide-react";
import { SidebarTrigger } from "@/shared/components/ui/sidebar";
import { titleFor, type PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type TopbarProps = {
    locale: Locale;
    activePage: PageId;
};

export function Topbar({ locale, activePage }: TopbarProps) {
    return (
        <header data-slot="app-topbar" className="app-topbar">
            <SidebarTrigger
                aria-label={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"}
                title={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"}
            >
                <Menu aria-hidden="true" />
            </SidebarTrigger>
            <div data-slot="app-topbar-title" className="topbar-title">
                {titleFor(activePage, locale)}
            </div>
        </header>
    );
}
