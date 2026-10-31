import { useCallback, useEffect, useState } from "react";
import { AboutDialog } from "@/features/about";
import { SettingsDialog } from "@/features/settings";
import { AppShell } from "./app-shell";
import { ToolStage } from "./tool-stage";
import { pageForToolId, tools, type PageId } from "./tool-registry";
import { useDroppedImage, useNativeBridge } from "./use-native-bridge";
import type { ApplicationSettings } from "@/shared/bridge/types";
import type { Locale } from "@/shared/types";

function App() {
    const [locale, setLocale] = useState<Locale>(
        navigator.language.toLowerCase().startsWith("ja") ? "ja" : "en",
    );
    const [activePage, setActivePage] = useState<PageId>("welcome");
    const [aboutOpen, setAboutOpen] = useState(false);
    const [settingsOpen, setSettingsOpen] = useState(false);
    const [databaseNavigationVersion, setDatabaseNavigationVersion] = useState(0);
    const [sidebarOpen, setSidebarOpen] = useState(true);
    const [theme, setTheme] = useState<"system" | "light" | "dark">("system");

    const openPage = useCallback((page: PageId) => setActivePage(page), []);
    const onNavigate = useCallback(
        (route: string) => {
            if (route === "about") {
                setAboutOpen(true);
                return;
            }
            if (route === "settings") {
                setSettingsOpen(true);
                return;
            }
            if (route === "welcome") {
                openPage("welcome");
                return;
            }
            const tool = tools.find((item) => item.id === route);
            if (!tool) return;
            if (tool.id === "db") setDatabaseNavigationVersion((version) => version + 1);
            openPage(tool.id);
        },
        [openPage],
    );
    const onStartupSettings = useCallback(
        (settings: Partial<ApplicationSettings>) => {
            if (settings.language === "ja_JP") setLocale("ja");
            else if (settings.language === "en") setLocale("en");
            if (
                settings.theme === "light" ||
                settings.theme === "dark" ||
                settings.theme === "system"
            ) {
                setTheme(settings.theme);
            }
            if (typeof settings.showSidebarOnStartup === "boolean") {
                setSidebarOpen(settings.showSidebarOnStartup);
            }
            if (settings.showLastToolOnStartup && typeof settings.lastUsedTool === "number") {
                const page = pageForToolId(settings.lastUsedTool);
                if (page) onNavigate(page);
            }
        },
        [onNavigate],
    );
    const [droppedImage, acceptDroppedImage, clearDroppedImage] = useDroppedImage(openPage);
    const { run } = useNativeBridge({
        locale,
        activePage,
        onNavigate,
        onDroppedImage: acceptDroppedImage,
        onStartupSettings,
    });

    useEffect(() => {
        document.documentElement.lang = locale === "ja" ? "ja" : "en";
    }, [locale]);
    useEffect(() => {
        document.documentElement.dataset.theme = theme;
    }, [theme]);

    return (
        <>
            <AppShell
                locale={locale}
                activePage={activePage}
                sidebarOpen={sidebarOpen}
                settingsOpen={settingsOpen}
                onNavigate={openPage}
                onOpenSettings={() => setSettingsOpen(true)}
                onOpenAbout={() => setAboutOpen(true)}
                onSidebarOpenChange={setSidebarOpen}
            >
                <ToolStage
                    activePage={activePage}
                    locale={locale}
                    run={run}
                    databaseNavigationVersion={databaseNavigationVersion}
                    droppedImage={droppedImage}
                    onDroppedImageConsumed={clearDroppedImage}
                    onNavigate={openPage}
                />
            </AppShell>
            <AboutDialog open={aboutOpen} onOpenChange={setAboutOpen} locale={locale} run={run} />
            <SettingsDialog
                open={settingsOpen}
                onOpenChange={setSettingsOpen}
                locale={locale}
                run={run}
                onLocaleChange={setLocale}
                onThemeChange={setTheme}
            />
        </>
    );
}

export default App;
