import type { ReactNode } from "react";
import { Sidebar } from "./sidebar";
import { Topbar } from "./topbar";
import type { PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type AppShellProps = {
    locale: Locale;
    activePage: PageId;
    sidebarOpen: boolean;
    settingsOpen: boolean;
    pendingCount: number;
    onNavigate: (page: PageId) => void;
    onOpenSettings: () => void;
    onOpenAbout: () => void;
    onToggleSidebar: () => void;
    onCancelOperations: () => void;
    children: ReactNode;
};

export function AppShell({
    locale,
    activePage,
    sidebarOpen,
    settingsOpen,
    pendingCount,
    onNavigate,
    onOpenSettings,
    onOpenAbout,
    onToggleSidebar,
    onCancelOperations,
    children,
}: AppShellProps) {
    return (
        <div className={`app-shell ${sidebarOpen ? "sidebar-open" : "sidebar-closed"}`}>
            {sidebarOpen && (
                <Sidebar
                    locale={locale}
                    activePage={activePage}
                    settingsOpen={settingsOpen}
                    onNavigate={onNavigate}
                    onOpenSettings={onOpenSettings}
                    onOpenAbout={onOpenAbout}
                />
            )}
            <main className="app-main">
                <Topbar
                    locale={locale}
                    activePage={activePage}
                    pendingCount={pendingCount}
                    onToggleSidebar={onToggleSidebar}
                    onCancelOperations={onCancelOperations}
                />
                {children}
            </main>
        </div>
    );
}
