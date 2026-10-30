import { useCallback, useEffect, useRef, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/components/ui/button";
import { Dialog } from "@/components/ui/dialog";
import { Field } from "@/components/ui/field";
import { Icon } from "@/components/ui/icon";
import { Input } from "@/components/ui/input";
import { SelectBox } from "@/components/ui/select";
import { Switch } from "@/components/ui/switch";
import { Tabs } from "@/components/ui/tabs";
import { Textarea } from "@/components/ui/textarea";
import {
    BRIDGE_CONTRACT_VERSION,
    connectNativeBridge,
    NativeOperationError,
    runNativeOperation,
    type AboutInfo,
    type BridgeEvent,
    type NativeBridge,
    type OperationName,
} from "@/bridge";
import { ToolPage, type DroppedImage, type Locale, type OperationRunner } from "@/tool-pages";
import { forkUiEffect, runUiEffect } from "@/effect";
import { licenseTextOrFallback } from "@/about";

type PageId =
    | "welcome"
    | "image"
    | "phrase"
    | "command"
    | "http"
    | "data"
    | "db"
    | "qr"
    | "markdown"
    | "timestamp"
    | "regex"
    | "diff"
    | "settings";
type ToolInfo = { id: PageId; en: string; ja: string; detailEn: string; detailJa: string };

const tools: ToolInfo[] = [
    {
        id: "image",
        en: "Image Editor",
        ja: "画像編集",
        detailEn: "Resize, transform and split images",
        detailJa: "画像のサイズ変更、変形、分割",
    },
    {
        id: "phrase",
        en: "Phrase Generation",
        ja: "定型文生成",
        detailEn: "Create and manage text templates",
        detailJa: "定型文を作成、管理",
    },
    {
        id: "command",
        en: "Command Generation",
        ja: "コマンド生成",
        detailEn: "Build Git and Docker commands",
        detailJa: "GitやDockerのコマンドを作成",
    },
    {
        id: "http",
        en: "HTTP Request",
        ja: "HTTPリクエスト",
        detailEn: "Send and inspect API requests",
        detailJa: "APIリクエストを送信、確認",
    },
    {
        id: "data",
        en: "Data Conversion",
        ja: "データ変換",
        detailEn: "Convert JSON, YAML and TOML",
        detailJa: "JSON、YAML、TOMLを変換",
    },
    {
        id: "db",
        en: "Database",
        ja: "データベース",
        detailEn: "Connect and run SQL queries",
        detailJa: "データベースに接続してSQLを実行",
    },
    {
        id: "qr",
        en: "QR Code",
        ja: "QRコード",
        detailEn: "Create QR codes from common data",
        detailJa: "各種データからQRコードを作成",
    },
    {
        id: "markdown",
        en: "Markdown Preview",
        ja: "Markdownプレビュー",
        detailEn: "Edit Markdown and preview safely",
        detailJa: "Markdownを編集、安全にプレビュー",
    },
    {
        id: "timestamp",
        en: "Timestamp Conversion",
        ja: "タイムスタンプ変換",
        detailEn: "Convert Unix and ISO-8601 values",
        detailJa: "Unix時刻とISO-8601形式を変換",
    },
    {
        id: "regex",
        en: "Regex Tester",
        ja: "正規表現テスター",
        detailEn: "Test patterns, matches and replacement",
        detailJa: "パターン、マッチ、置換を確認",
    },
    {
        id: "diff",
        en: "Text Diff",
        ja: "テキスト差分",
        detailEn: "Compare two text versions",
        detailJa: "2つのテキストを比較",
    },
];

const toolIds: Record<string, number> = {
    http: 1,
    command: 2,
    image: 3,
    phrase: 6,
    data: 7,
    db: 8,
    qr: 9,
    markdown: 10,
    timestamp: 11,
    regex: 12,
    diff: 13,
};
const pageForToolId = (id: number): PageId | undefined =>
    Object.entries(toolIds).find(([, value]) => value === id)?.[0] as PageId | undefined;

const iconFor = (id: PageId) => {
    const icons: Record<PageId, string> = {
        welcome: "home",
        image: "image",
        phrase: "library_books",
        command: "terminal",
        http: "lan",
        data: "transform",
        db: "database",
        qr: "qr_code",
        markdown: "article",
        timestamp: "schedule",
        regex: "regular_expression",
        diff: "difference",
        settings: "settings",
    };
    return icons[id];
};

const titleFor = (id: PageId, locale: Locale) => {
    if (id === "welcome") return locale === "ja" ? "ホーム" : "Home";
    if (id === "settings") return locale === "ja" ? "設定" : "Settings";
    const item = tools.find((tool) => tool.id === id);
    return item ? (locale === "ja" ? item.ja : item.en) : id;
};

function App() {
    const [locale, setLocale] = useState<Locale>(
        navigator.language.toLowerCase().startsWith("ja") ? "ja" : "en",
    );
    const [bridge, setBridge] = useState<NativeBridge | null>(null);
    const [activePage, setActivePage] = useState<PageId>("welcome");
    const [aboutOpen, setAboutOpen] = useState(false);
    const [settingsOpen, setSettingsOpen] = useState(false);
    const [databaseNavigationVersion, setDatabaseNavigationVersion] = useState(0);
    const [search, setSearch] = useState("");
    const [sidebarOpen, setSidebarOpen] = useState(true);
    const [theme, setTheme] = useState<"system" | "light" | "dark">("system");
    const [droppedImage, setDroppedImage] = useState<DroppedImage | null>(null);
    const [pendingCount, setPendingCount] = useState(0);
    const pending = useRef(new Map<string, () => void>());
    const localeRef = useRef(locale);
    localeRef.current = locale;
    const searchRef = useRef<HTMLInputElement>(null);

    const openPage = useCallback((page: PageId) => {
        setActivePage(page);
    }, []);
    const acceptDroppedImage = useCallback(
        (result: Record<string, unknown>) => {
            if (typeof result.imageDataUrl !== "string" || !result.imageDataUrl) return;
            openPage("image");
            setDroppedImage({
                id: crypto.randomUUID(),
                imageDataUrl: result.imageDataUrl,
                fileName: String(result.fileName ?? ""),
                width: Number(result.width ?? 0),
                height: Number(result.height ?? 0),
            });
        },
        [openPage],
    );
    const clearDroppedImage = useCallback((id: string) => {
        setDroppedImage((current) => (current?.id === id ? null : current));
    }, []);

    useEffect(() => {
        document.documentElement.lang = locale === "ja" ? "ja" : "en";
    }, [locale]);
    useEffect(() => {
        document.documentElement.dataset.theme = theme;
    }, [theme]);

    function runWithBridge(
        native: NativeBridge,
        operation: OperationName,
        payload: Record<string, unknown>,
    ) {
        const requestId = crypto.randomUUID().replaceAll("-", "");
        return runNativeOperation(
            native,
            { contractVersion: BRIDGE_CONTRACT_VERSION, requestId, operation, payload },
            localeRef.current === "ja" ? "処理をキャンセルしました。" : "Operation cancelled.",
            (id, isPending, cancel) => {
                if (isPending) pending.current.set(id, cancel);
                else pending.current.delete(id);
                setPendingCount(pending.current.size);
            },
        );
    }

    useEffect(() => {
        let mounted = true;
        let unsubscribe: (() => void) | undefined;
        let nativeBridge: NativeBridge | null = null;
        const onEvent = (event: BridgeEvent) => {
            if (event.type === "imageDropAvailable") {
                if (nativeBridge) {
                    runUiEffect(
                        runWithBridge(nativeBridge, "imageTakeDropped", {}).pipe(
                            Effect.tap((result) => Effect.sync(() => acceptDroppedImage(result))),
                            Effect.catch(() => Effect.void),
                        ),
                    );
                }
                return;
            }
            if (event.type === "navigate") {
                if (event.route === "about") {
                    setAboutOpen(true);
                    return;
                }
                if (event.route === "settings") {
                    setSettingsOpen(true);
                    return;
                }
                if (["welcome", ...tools.map((item) => item.id)].includes(event.route)) {
                    if (event.route === "db") {
                        setDatabaseNavigationVersion((version) => version + 1);
                    }
                    openPage(event.route as PageId);
                }
            }
        };
        const setup = Effect.gen(function* () {
            const native = yield* connectNativeBridge();
            if (!mounted) return;
            nativeBridge = native;
            unsubscribe = native.subscribe(onEvent);
            yield* Effect.sync(() => setBridge(native));

            const [droppedImageResult, settings] = yield* Effect.all(
                [
                    runWithBridge(native, "imageTakeDropped", {}).pipe(
                        Effect.catch(() => Effect.succeed({} as Record<string, unknown>)),
                    ),
                    runWithBridge(native, "getSettings", {}).pipe(
                        Effect.catch(() => Effect.succeed({} as Record<string, unknown>)),
                    ),
                ],
                { concurrency: "unbounded" },
            );
            if (mounted && droppedImageResult.found === true) {
                yield* Effect.sync(() => acceptDroppedImage(droppedImageResult));
            }
            yield* Effect.sync(() => {
                if (mounted && settings.language === "ja_JP") setLocale("ja");
                else if (mounted && settings.language === "en") setLocale("en");
                if (
                    mounted &&
                    (settings.theme === "light" ||
                        settings.theme === "dark" ||
                        settings.theme === "system")
                )
                    setTheme(settings.theme);
                if (mounted && typeof settings.showSidebarOnStartup === "boolean")
                    setSidebarOpen(settings.showSidebarOnStartup);
                if (
                    mounted &&
                    settings.showLastToolOnStartup &&
                    typeof settings.lastUsedTool === "number"
                ) {
                    const page = pageForToolId(settings.lastUsedTool);
                    if (page) {
                        setActivePage(page);
                    }
                }
            });
        }).pipe(Effect.catch(() => Effect.void));
        const interruptSetup = forkUiEffect(setup);
        return () => {
            mounted = false;
            unsubscribe?.();
            interruptSetup();
        };
        // WebChannel subscription and native connection are app-scoped.
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [acceptDroppedImage]);

    const runOperation: OperationRunner = useCallback(
        (operation, payload) => {
            if (!bridge) {
                return Effect.fail(
                    new NativeOperationError(
                        locale === "ja"
                            ? "ネイティブ連携に接続していません。"
                            : "Native services are not connected.",
                        "NOT_CONNECTED",
                    ),
                );
            }
            return runWithBridge(bridge, operation, payload);
            // eslint-disable-next-line react-hooks/exhaustive-deps
        },
        [bridge, locale],
    );

    useEffect(() => {
        const toolId = toolIds[activePage];
        if (bridge && toolId) {
            runUiEffect(
                runWithBridge(bridge, "saveLastUsedTool", { toolId }).pipe(
                    Effect.catch(() => Effect.void),
                ),
            );
        }
        // Persist only tool pages; welcome and preferences are not tool selections.
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [activePage, bridge]);

    useEffect(() => {
        const onKeyDown = (event: KeyboardEvent) => {
            const command = event.metaKey || event.ctrlKey;
            const target = event.target as HTMLElement | null;
            const editing =
                target?.matches("input, textarea, select, [contenteditable='true']") ?? false;
            if (command && event.key.toLowerCase() === "k") {
                event.preventDefault();
                searchRef.current?.focus();
            } else if (!editing && event.key === "Escape") {
                setSearch("");
                searchRef.current?.blur();
            }
        };
        window.addEventListener("keydown", onKeyDown);
        return () => window.removeEventListener("keydown", onKeyDown);
    }, []);

    const filteredTools = tools.filter((tool) => {
        const query = search.trim().toLowerCase();
        return (
            !query ||
            `${tool.en} ${tool.ja} ${tool.detailEn} ${tool.detailJa}`.toLowerCase().includes(query)
        );
    });

    return (
        <div className={`app-shell ${sidebarOpen ? "sidebar-open" : "sidebar-closed"}`}>
            {sidebarOpen && (
                <aside className="app-sidebar" aria-label={locale === "ja" ? "ツール" : "Tools"}>
                    <div className="brand-row">
                        <img className="brand-logo" src="qrc:/logo/dev-tools_logo.png" alt="" />
                        <strong>DevTools</strong>
                    </div>
                    <label className="search-wrap">
                        <span aria-hidden="true">⌕</span>
                        <Input
                            className="search-control"
                            ref={searchRef}
                            value={search}
                            onChange={(event) => setSearch(event.target.value)}
                            placeholder={locale === "ja" ? "ツールを検索" : "Search tools"}
                            aria-label={locale === "ja" ? "ツールを検索" : "Search tools"}
                        />
                        <kbd>⌘K</kbd>
                    </label>
                    <Button
                        variant="ghost"
                        size="sm"
                        className={`nav-item ${activePage === "welcome" ? "active" : ""}`}
                        onClick={() => openPage("welcome")}
                    >
                        <Icon className="nav-icon">{iconFor("welcome")}</Icon>
                        {locale === "ja" ? "ホーム" : "Home"}
                    </Button>
                    <div className="sidebar-section-title">
                        {locale === "ja" ? "ツール" : "WORKSPACE"}
                    </div>
                    <nav className="tool-nav">
                        {filteredTools.map((tool) => (
                            <Button
                                key={tool.id}
                                variant="ghost"
                                size="sm"
                                className={`nav-item ${activePage === tool.id ? "active" : ""}`}
                                onClick={() => openPage(tool.id)}
                            >
                                <Icon className="nav-icon">{iconFor(tool.id)}</Icon>
                                <span>{locale === "ja" ? tool.ja : tool.en}</span>
                            </Button>
                        ))}
                    </nav>
                    <div className="sidebar-bottom">
                        <Button
                            variant="ghost"
                            size="sm"
                            className={`nav-item ${settingsOpen ? "active" : ""}`}
                            aria-haspopup="dialog"
                            onClick={() => setSettingsOpen(true)}
                        >
                            <Icon className="nav-icon">{iconFor("settings")}</Icon>
                            {locale === "ja" ? "設定" : "Settings"}
                        </Button>
                        <Button
                            variant="ghost"
                            size="sm"
                            className="nav-item"
                            onClick={() => setAboutOpen(true)}
                        >
                            <Icon className="nav-icon">info</Icon>
                            {locale === "ja" ? "このアプリについて" : "About"}
                        </Button>
                    </div>
                </aside>
            )}
            <main className="app-main">
                <header className="app-topbar">
                    <Button
                        variant="ghost"
                        aria-label={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"}
                        onClick={() => setSidebarOpen((open) => !open)}
                    >
                        <Icon>menu</Icon>
                    </Button>
                    <div className="topbar-title">{titleFor(activePage, locale)}</div>
                    {pendingCount > 0 && (
                        <Button
                            variant="outline"
                            onClick={() => {
                                for (const cancel of pending.current.values()) cancel();
                            }}
                        >
                            {locale === "ja" ? "キャンセル" : "Cancel"} ({pendingCount})
                        </Button>
                    )}
                </header>
                <div className="page-stack">
                    {activePage !== "db" && (
                        <section
                            className="page-host active-page"
                            data-page={activePage}
                            aria-label={titleFor(activePage, locale)}
                        >
                            {activePage === "welcome" ? (
                                <WelcomePage locale={locale} onOpen={openPage} />
                            ) : (
                                <ToolPage
                                    key={activePage}
                                    id={activePage}
                                    locale={locale}
                                    run={runOperation}
                                    active
                                    databaseNavigationVersion={databaseNavigationVersion}
                                    droppedImage={droppedImage}
                                    onDroppedImageConsumed={clearDroppedImage}
                                />
                            )}
                        </section>
                    )}
                    <section
                        className="page-host database-cache"
                        data-page="db"
                        aria-label={titleFor("db", locale)}
                        hidden={activePage !== "db"}
                    >
                        <ToolPage
                            id="db"
                            locale={locale}
                            run={runOperation}
                            active={activePage === "db"}
                            databaseNavigationVersion={databaseNavigationVersion}
                        />
                    </section>
                </div>
            </main>
            <AboutDialog
                open={aboutOpen}
                onOpenChange={setAboutOpen}
                locale={locale}
                run={runOperation}
            />
            <SettingsDialog
                open={settingsOpen}
                onOpenChange={setSettingsOpen}
                locale={locale}
                run={runOperation}
                onLocaleChange={setLocale}
                onThemeChange={setTheme}
            />
        </div>
    );
}

function WelcomePage({ locale, onOpen }: { locale: Locale; onOpen: (page: PageId) => void }) {
    return (
        <div className="welcome-page">
            <div className="tool-card-grid">
                {tools.map((tool) => (
                    <Button
                        key={tool.id}
                        variant="ghost"
                        className="tool-card h-auto"
                        onClick={() => onOpen(tool.id)}
                    >
                        <Icon className="tool-card-icon">{iconFor(tool.id)}</Icon>
                        <strong>{locale === "ja" ? tool.ja : tool.en}</strong>
                    </Button>
                ))}
            </div>
        </div>
    );
}

type AppSettings = {
    language: "en" | "ja_JP";
    theme: "system" | "light" | "dark";
    showSidebarOnStartup: boolean;
    showLastToolOnStartup: boolean;
    alwaysOnTop: boolean;
    rememberWindowSize: boolean;
    rememberWindowPosition: boolean;
};
const defaultSettings = (locale: Locale): AppSettings => ({
    language: locale === "ja" ? "ja_JP" : "en",
    theme: "system",
    showSidebarOnStartup: true,
    showLastToolOnStartup: false,
    alwaysOnTop: false,
    rememberWindowSize: true,
    rememberWindowPosition: true,
});

function SettingsDialog({
    open,
    onOpenChange,
    locale,
    run,
    onLocaleChange,
    onThemeChange,
}: {
    open: boolean;
    onOpenChange: (open: boolean) => void;
    locale: Locale;
    run: OperationRunner;
    onLocaleChange: (locale: Locale) => void;
    onThemeChange: (theme: "system" | "light" | "dark") => void;
}) {
    return (
        <Dialog.Root open={open} onOpenChange={onOpenChange}>
            <Dialog.Portal>
                <Dialog.Backdrop className="dialog-backdrop" />
                <Dialog.Viewport className="dialog-viewport">
                    <Dialog.Popup className="settings-dialog">
                        <Dialog.Title className="dialog-title">
                            {locale === "ja" ? "設定" : "Settings"}
                        </Dialog.Title>
                        <Dialog.Description className="sr-only">
                            {locale === "ja" ? "アプリケーション設定" : "Application settings"}
                        </Dialog.Description>
                        {open && (
                            <SettingsForm
                                locale={locale}
                                run={run}
                                onLocaleChange={onLocaleChange}
                                onThemeChange={onThemeChange}
                                onClose={() => onOpenChange(false)}
                            />
                        )}
                    </Dialog.Popup>
                </Dialog.Viewport>
            </Dialog.Portal>
        </Dialog.Root>
    );
}

function SettingsForm({
    locale,
    run,
    onLocaleChange,
    onThemeChange,
    onClose,
}: {
    locale: Locale;
    run: OperationRunner;
    onLocaleChange: (locale: Locale) => void;
    onThemeChange: (theme: "system" | "light" | "dark") => void;
    onClose: () => void;
}) {
    const [settings, setSettings] = useState(defaultSettings(locale));
    const [error, setError] = useState("");
    const [loading, setLoading] = useState(true);
    const [saving, setSaving] = useState(false);
    useEffect(() => {
        const settingsEffect = run("getSettings", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() =>
                    setSettings((current) => ({ ...current, ...result }) as AppSettings),
                ),
            ),
            Effect.catch((reason) =>
                Effect.sync(() =>
                    setError(reason instanceof Error ? reason.message : String(reason)),
                ),
            ),
            Effect.ensuring(Effect.sync(() => setLoading(false))),
        );
        return forkUiEffect(settingsEffect);
    }, [run]);
    const labels: Record<keyof AppSettings, string> = {
        language: locale === "ja" ? "言語" : "Language",
        theme: locale === "ja" ? "テーマ" : "Theme",
        showSidebarOnStartup:
            locale === "ja" ? "起動時にサイドバーを表示" : "Show sidebar on startup",
        showLastToolOnStartup:
            locale === "ja" ? "前回のツールを開く" : "Open last used tool on startup",
        alwaysOnTop: locale === "ja" ? "ウィンドウを常に手前に表示" : "Keep window on top",
        rememberWindowSize: locale === "ja" ? "ウィンドウサイズを記憶" : "Remember window size",
        rememberWindowPosition:
            locale === "ja" ? "ウィンドウ位置を記憶" : "Remember window position",
    };
    const save = () => {
        setError("");
        setSaving(true);
        runUiEffect(
            run("saveSettings", settings).pipe(
                Effect.tap(() =>
                    Effect.sync(() => {
                        const nextLocale = settings.language === "ja_JP" ? "ja" : "en";
                        onLocaleChange(nextLocale);
                        onThemeChange(settings.theme);
                        onClose();
                    }),
                ),
                Effect.catch((reason) =>
                    Effect.sync(() =>
                        setError(reason instanceof Error ? reason.message : String(reason)),
                    ),
                ),
                Effect.ensuring(Effect.sync(() => setSaving(false))),
            ),
        );
    };
    return (
        <div className="settings-page">
            <div className="settings-card">
                <Field label={labels.language}>
                    <SelectBox
                        disabled={loading || saving}
                        value={settings.language}
                        onValueChange={(language) =>
                            setSettings((value) => ({
                                ...value,
                                language: language as AppSettings["language"],
                            }))
                        }
                        options={[
                            { value: "en", label: "English" },
                            { value: "ja_JP", label: "日本語" },
                        ]}
                    />
                </Field>
                <Field label={labels.theme}>
                    <SelectBox
                        disabled={loading || saving}
                        value={settings.theme}
                        onValueChange={(theme) =>
                            setSettings((value) => ({
                                ...value,
                                theme: theme as AppSettings["theme"],
                            }))
                        }
                        options={[
                            {
                                value: "system",
                                label: locale === "ja" ? "システム設定に合わせる" : "Follow system",
                            },
                            { value: "light", label: locale === "ja" ? "ライト" : "Light" },
                            { value: "dark", label: locale === "ja" ? "ダーク" : "Dark" },
                        ]}
                    />
                </Field>
                {(Object.keys(labels) as Array<keyof AppSettings>)
                    .filter((key) => key !== "language" && key !== "theme")
                    .map((key) => (
                        <label key={key} className="settings-toggle">
                            <span>{labels[key]}</span>
                            <Switch
                                id={`settings-${key}`}
                                disabled={loading || saving}
                                checked={settings[key] as boolean}
                                onCheckedChange={(checked) =>
                                    setSettings((value) => ({ ...value, [key]: checked }))
                                }
                            />
                        </label>
                    ))}
                {error && (
                    <p className="operation-message error" role="alert">
                        {error}
                    </p>
                )}
                <div className="settings-actions dialog-footer">
                    <Dialog.Close render={<Button variant="outline" disabled={saving} />}>
                        {locale === "ja" ? "キャンセル" : "Cancel"}
                    </Dialog.Close>
                    <Button onClick={save} disabled={loading || saving}>
                        {locale === "ja" ? "保存" : "Save"}
                    </Button>
                </div>
            </div>
        </div>
    );
}

function AboutDialog({
    open,
    onOpenChange,
    locale,
    run,
}: {
    open: boolean;
    onOpenChange: (open: boolean) => void;
    locale: Locale;
    run: OperationRunner;
}) {
    const [info, setInfo] = useState<AboutInfo | null>(null);
    const [error, setError] = useState("");
    const [message, setMessage] = useState("");

    useEffect(() => {
        if (!open) return;
        const load = run("getAboutInfo", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    setInfo(result as AboutInfo);
                    setError("");
                }),
            ),
            Effect.catch((reason) =>
                Effect.sync(() => {
                    setError(reason instanceof Error ? reason.message : String(reason));
                }),
            ),
        );
        return forkUiEffect(load);
    }, [open, run]);

    const copy = () => {
        if (!info) return;
        const summary = [
            `DevTools ${info.appVersion}`,
            `Revision: ${info.revision}`,
            `Build Environment: ${info.buildEnvironment}`,
            `Qt Version: ${info.qtVersion}`,
            info.copyright,
        ].join("\n");
        runUiEffect(
            run("copyToClipboard", { text: summary }).pipe(
                Effect.tap(() =>
                    Effect.sync(() => setMessage(locale === "ja" ? "コピーしました。" : "Copied.")),
                ),
                Effect.catch((reason) =>
                    Effect.sync(() =>
                        setMessage(reason instanceof Error ? reason.message : String(reason)),
                    ),
                ),
            ),
        );
    };
    const metadata: Array<[string, string]> = info
        ? [
              [locale === "ja" ? "バージョン" : "Application Version", info.appVersion],
              [locale === "ja" ? "リビジョン" : "Revision", info.revision],
              [locale === "ja" ? "ビルド環境" : "Build Environment", info.buildEnvironment],
              [locale === "ja" ? "Qtバージョン" : "Qt Version", info.qtVersion],
              [locale === "ja" ? "著作権" : "Copyright", info.copyright],
          ]
        : [];

    return (
        <Dialog.Root open={open} onOpenChange={onOpenChange}>
            <Dialog.Portal>
                <Dialog.Backdrop className="dialog-backdrop" />
                <Dialog.Viewport className="dialog-viewport">
                    <Dialog.Popup className="about-dialog">
                        <Dialog.Title className="dialog-title">
                            {locale === "ja" ? "DevToolsについて" : "About DevTools"}
                        </Dialog.Title>
                        <Dialog.Description className="sr-only">
                            {locale === "ja"
                                ? "DevToolsのアプリケーション情報とライセンス"
                                : "DevTools application information and license"}
                        </Dialog.Description>
                        <Tabs.Root defaultValue="about" className="about-tabs">
                            <Tabs.List className="feature-tabs-list">
                                <Tabs.Tab value="about" className="feature-tab">
                                    {locale === "ja" ? "概要" : "About"}
                                </Tabs.Tab>
                                <Tabs.Tab value="license" className="feature-tab">
                                    {locale === "ja" ? "ライセンス" : "License"}
                                </Tabs.Tab>
                            </Tabs.List>
                            <Tabs.Panel value="about" className="about-panel">
                                <div className="about-brand">
                                    <img src="qrc:/logo/dev-tools_logo.png" alt="" />
                                    <strong>DevTools</strong>
                                </div>
                                {error && <p className="operation-message error">{error}</p>}
                                <dl className="about-metadata">
                                    {metadata.map(([label, value]) => (
                                        <div key={label}>
                                            <dt>{label}</dt>
                                            <dd>{value}</dd>
                                        </div>
                                    ))}
                                </dl>
                                <div className="about-actions">
                                    <span role="status">{message}</span>
                                    <Button variant="outline" onClick={copy} disabled={!info}>
                                        {locale === "ja" ? "コピー" : "Copy"}
                                    </Button>
                                </div>
                            </Tabs.Panel>
                            <Tabs.Panel value="license" className="about-panel">
                                <Textarea
                                    className="license-text"
                                    value={licenseTextOrFallback(info?.licenseText, locale)}
                                    readOnly
                                    spellCheck={false}
                                />
                            </Tabs.Panel>
                        </Tabs.Root>
                        <div className="dialog-footer">
                            <Dialog.Close render={<Button variant="outline" />}>
                                {locale === "ja" ? "OK" : "OK"}
                            </Dialog.Close>
                        </div>
                    </Dialog.Popup>
                </Dialog.Viewport>
            </Dialog.Portal>
        </Dialog.Root>
    );
}

export default App;
