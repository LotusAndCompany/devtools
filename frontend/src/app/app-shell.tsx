import type { CSSProperties, ReactNode } from "react";
import { Sidebar } from "./sidebar";
import { Topbar } from "./topbar";
import { SidebarInset, SidebarProvider } from "@/shared/components/ui/sidebar";
import type { PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type AppShellProps = {
    locale: Locale;
    activePage: PageId;
    sidebarOpen: boolean;
    settingsOpen: boolean;
    onNavigate: (page: PageId) => void;
    onOpenSettings: () => void;
    onOpenAbout: () => void;
    onSidebarOpenChange: (open: boolean) => void;
    children: ReactNode;
};

export function AppShell({
    locale,
    activePage,
    sidebarOpen,
    settingsOpen,
    onNavigate,
    onOpenSettings,
    onOpenAbout,
    onSidebarOpenChange,
    children,
}: AppShellProps) {
    return (
        <SidebarProvider
            open={sidebarOpen}
            onOpenChange={onSidebarOpenChange}
            className="app-shell"
            style={{ "--sidebar-width": "var(--app-sidebar-width)" } as CSSProperties}
        >
            <Sidebar
                locale={locale}
                activePage={activePage}
                settingsOpen={settingsOpen}
                onNavigate={onNavigate}
                onOpenSettings={onOpenSettings}
                onOpenAbout={onOpenAbout}
            />
            <SidebarInset className="app-main">
                <Topbar locale={locale} activePage={activePage} />
                {children}
            </SidebarInset>
        </SidebarProvider>
    );
}
