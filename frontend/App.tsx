import { useCallback, useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import {
  BRIDGE_CONTRACT_VERSION,
  connectNativeBridge,
  type BridgeEvent,
  type NativeBridge,
  type OperationName,
} from "@/bridge";
import { ToolPage, type DroppedImage, type Locale, type OperationRunner } from "@/tool-pages";

type PageId = "welcome" | "image" | "phrase" | "command" | "http" | "data" | "db" | "qr" | "markdown" | "timestamp" | "regex" | "diff" | "settings" | "about";
type ToolInfo = { id: PageId; en: string; ja: string; detailEn: string; detailJa: string };

const tools: ToolInfo[] = [
  { id: "image", en: "Image Editor", ja: "画像編集", detailEn: "Resize, transform and split images", detailJa: "画像のサイズ変更、変形、分割" },
  { id: "phrase", en: "Phrase Generation", ja: "定型文生成", detailEn: "Create and manage text templates", detailJa: "定型文を作成、管理" },
  { id: "command", en: "Command Generation", ja: "コマンド生成", detailEn: "Build Git and Docker commands", detailJa: "GitやDockerのコマンドを作成" },
  { id: "http", en: "HTTP Request", ja: "HTTPリクエスト", detailEn: "Send and inspect API requests", detailJa: "APIリクエストを送信、確認" },
  { id: "data", en: "Data Conversion", ja: "データ変換", detailEn: "Convert JSON, YAML and TOML", detailJa: "JSON、YAML、TOMLを変換" },
  { id: "db", en: "Database", ja: "データベース", detailEn: "Connect and run SQL queries", detailJa: "データベースに接続してSQLを実行" },
  { id: "qr", en: "QR Code", ja: "QRコード", detailEn: "Create QR codes from common data", detailJa: "各種データからQRコードを作成" },
  { id: "markdown", en: "Markdown Preview", ja: "Markdownプレビュー", detailEn: "Edit Markdown and preview safely", detailJa: "Markdownを編集、安全にプレビュー" },
  { id: "timestamp", en: "Timestamp Conversion", ja: "タイムスタンプ変換", detailEn: "Convert Unix and ISO-8601 values", detailJa: "Unix時刻とISO-8601形式を変換" },
  { id: "regex", en: "Regex Tester", ja: "正規表現テスター", detailEn: "Test patterns, matches and replacement", detailJa: "パターン、マッチ、置換を確認" },
  { id: "diff", en: "Text Diff", ja: "テキスト差分", detailEn: "Compare two text versions", detailJa: "2つのテキストを比較" },
];

const toolIds: Record<string, number> = { http: 1, command: 2, image: 3, phrase: 6, data: 7, db: 8, qr: 9, markdown: 10, timestamp: 11, regex: 12, diff: 13 };
const pageForToolId = (id: number): PageId | undefined => Object.entries(toolIds).find(([, value]) => value === id)?.[0] as PageId | undefined;

const titleFor = (id: PageId, locale: Locale) => {
  if (id === "welcome") return locale === "ja" ? "ホーム" : "Home";
  if (id === "settings") return locale === "ja" ? "設定" : "Settings";
  if (id === "about") return locale === "ja" ? "DevToolsについて" : "About DevTools";
  const item = tools.find((tool) => tool.id === id);
  return item ? (locale === "ja" ? item.ja : item.en) : id;
};

type PendingRequest = { resolve: (value: Record<string, unknown>) => void; reject: (error: Error) => void };

function App() {
  const [locale, setLocale] = useState<Locale>(navigator.language.toLowerCase().startsWith("ja") ? "ja" : "en");
  const [bridge, setBridge] = useState<NativeBridge | null>(null);
  const [connectionError, setConnectionError] = useState("");
  const [openPages, setOpenPages] = useState<PageId[]>(["welcome"]);
  const [activePage, setActivePage] = useState<PageId>("welcome");
  const [search, setSearch] = useState("");
  const [sidebarOpen, setSidebarOpen] = useState(true);
  const [theme, setTheme] = useState<"system" | "light" | "dark">("system");
  const [droppedImage, setDroppedImage] = useState<DroppedImage | null>(null);
  const [pendingCount, setPendingCount] = useState(0);
  const pending = useRef(new Map<string, PendingRequest>());
  const searchRef = useRef<HTMLInputElement>(null);

  const openPage = useCallback((page: PageId) => {
    setOpenPages((current) => current.includes(page) ? current : [...current, page]);
    setActivePage(page);
  }, []);
  const acceptDroppedImage = useCallback((result: Record<string, unknown>) => {
    if (typeof result.imageDataUrl !== "string" || !result.imageDataUrl) return;
    openPage("image");
    setDroppedImage({
      id: crypto.randomUUID(),
      imageDataUrl: result.imageDataUrl,
      fileName: String(result.fileName ?? ""),
      width: Number(result.width ?? 0),
      height: Number(result.height ?? 0),
    });
  }, [openPage]);
  const clearDroppedImage = useCallback((id: string) => {
    setDroppedImage((current) => current?.id === id ? null : current);
  }, []);

  useEffect(() => { document.documentElement.lang = locale === "ja" ? "ja" : "en"; }, [locale]);
  useEffect(() => { document.documentElement.dataset.theme = theme; }, [theme]);

  function runWithBridge(native: NativeBridge, operation: OperationName, payload: Record<string, unknown>) {
    const requestId = crypto.randomUUID().replaceAll("-", "");
    return new Promise<Record<string, unknown>>((resolve, reject) => {
      pending.current.set(requestId, { resolve, reject });
      setPendingCount(pending.current.size);
      try {
        native.startOperation({ contractVersion: BRIDGE_CONTRACT_VERSION, requestId, operation, payload });
      } catch (error) {
        pending.current.delete(requestId);
        setPendingCount(pending.current.size);
        reject(error instanceof Error ? error : new Error(String(error)));
      }
    });
  }

  useEffect(() => {
    let mounted = true;
    let unsubscribe: (() => void) | undefined;
    let nativeBridge: NativeBridge | null = null;
    const onEvent = (event: BridgeEvent) => {
      if (event.type === "imageDropAvailable") {
        if (nativeBridge) {
          void runWithBridge(nativeBridge, "imageTakeDropped", {}).then(acceptDroppedImage).catch(() => {});
        }
        return;
      }
      if (event.type === "navigate") {
        if (["settings", "about", "welcome", ...tools.map((item) => item.id)].includes(event.route)) openPage(event.route as PageId);
        return;
      }
      const request = pending.current.get(event.requestId);
      if (!request) return;
      if (event.type === "completed") {
        pending.current.delete(event.requestId);
        setPendingCount(pending.current.size);
        request.resolve(event.result);
      } else if (event.type === "failed") {
        pending.current.delete(event.requestId);
        setPendingCount(pending.current.size);
        request.reject(new Error(event.error.message || event.error.code));
      } else if (event.type === "cancelled") {
        pending.current.delete(event.requestId);
        setPendingCount(pending.current.size);
        request.reject(new Error(locale === "ja" ? "処理をキャンセルしました。" : "Operation cancelled."));
      }
    };
    void connectNativeBridge().then((native) => {
      if (!mounted) return;
      nativeBridge = native;
      unsubscribe = native.subscribe(onEvent);
      setBridge(native);
      setConnectionError("");
      void runWithBridge(native, "imageTakeDropped", {}).then((result) => {
        if (mounted && result.found === true) acceptDroppedImage(result);
      }).catch(() => {});
      void runWithBridge(native, "getSettings", {}).then((settings) => {
        if (mounted && settings.language === "ja_JP") setLocale("ja");
        else if (mounted && settings.language === "en") setLocale("en");
        if (mounted && (settings.theme === "light" || settings.theme === "dark" || settings.theme === "system")) setTheme(settings.theme);
        if (mounted && typeof settings.showSidebarOnStartup === "boolean") setSidebarOpen(settings.showSidebarOnStartup);
        if (mounted && settings.showLastToolOnStartup && typeof settings.lastUsedTool === "number") {
          const page = pageForToolId(settings.lastUsedTool);
          if (page) { setOpenPages(["welcome", page]); setActivePage(page); }
        }
      }).catch(() => {});
    }).catch((error: unknown) => {
      if (mounted) setConnectionError(error instanceof Error ? error.message : String(error));
    });
    return () => { mounted = false; unsubscribe?.(); };
    // WebChannel subscription and native connection are app-scoped.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [acceptDroppedImage]);

  const runOperation: OperationRunner = useCallback((operation, payload) => {
    if (!bridge) return Promise.reject(new Error(locale === "ja" ? "ネイティブ連携に接続していません。" : "Native services are not connected."));
    return runWithBridge(bridge, operation, payload);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [bridge, locale]);

  useEffect(() => {
    const toolId = toolIds[activePage];
    if (bridge && toolId) void runWithBridge(bridge, "saveLastUsedTool", { toolId }).catch(() => {});
    // Persist only tool pages; welcome and preferences are not tool selections.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [activePage, bridge]);

  const closePage = (page: PageId) => {
    if (page === "welcome") return;
    const next = openPages.filter((id) => id !== page);
    setOpenPages(next.length ? next : ["welcome"]);
    if (activePage === page) setActivePage(next.at(-1) ?? "welcome");
  };

  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      const command = event.metaKey || event.ctrlKey;
      const target = event.target as HTMLElement | null;
      const editing = target?.matches("input, textarea, select, [contenteditable='true']") ?? false;
      if (command && event.key.toLowerCase() === "k") { event.preventDefault(); searchRef.current?.focus(); }
      else if (command && event.key.toLowerCase() === "w") { event.preventDefault(); closePage(activePage); }
      else if (command && /^[1-9]$/.test(event.key)) {
        const page = openPages[Number(event.key) - 1];
        if (page) { event.preventDefault(); setActivePage(page); }
      } else if (!editing && event.key === "Escape") { setSearch(""); searchRef.current?.blur(); }
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [activePage, openPages]);

  const filteredTools = tools.filter((tool) => {
    const query = search.trim().toLowerCase();
    return !query || `${tool.en} ${tool.ja} ${tool.detailEn} ${tool.detailJa}`.toLowerCase().includes(query);
  });

  return (
    <div className={`app-shell ${sidebarOpen ? "sidebar-open" : "sidebar-closed"}`}>
      {sidebarOpen && <aside className="app-sidebar" aria-label={locale === "ja" ? "ツール" : "Tools"}>
        <div className="brand-row"><div className="brand-mark" aria-hidden="true">D</div><div><strong>DevTools</strong><span>{locale === "ja" ? "デスクトップツール" : "Desktop toolkit"}</span></div></div>
        <label className="search-wrap"><span aria-hidden="true">⌕</span><input ref={searchRef} value={search} onChange={(event) => setSearch(event.target.value)} placeholder={locale === "ja" ? "ツールを検索" : "Search tools"} aria-label={locale === "ja" ? "ツールを検索" : "Search tools"} /><kbd>⌘K</kbd></label>
        <button className={`nav-item ${activePage === "welcome" ? "active" : ""}`} onClick={() => openPage("welcome")}><span className="nav-icon">⌂</span>{locale === "ja" ? "ホーム" : "Home"}</button>
        <div className="sidebar-section-title">{locale === "ja" ? "ツール" : "WORKSPACE"}</div>
        <nav className="tool-nav">{filteredTools.map((tool) => <button key={tool.id} className={`nav-item ${activePage === tool.id ? "active" : ""}`} onClick={() => openPage(tool.id)} title={locale === "ja" ? tool.detailJa : tool.detailEn}><span className="nav-icon" aria-hidden="true">{tool.en.slice(0, 1)}</span><span>{locale === "ja" ? tool.ja : tool.en}</span></button>)}</nav>
        <div className="sidebar-bottom">
          <button className={`nav-item ${activePage === "settings" ? "active" : ""}`} onClick={() => openPage("settings")}><span className="nav-icon">⚙</span>{locale === "ja" ? "設定" : "Settings"}</button>
          <button className={`nav-item ${activePage === "about" ? "active" : ""}`} onClick={() => openPage("about")}><span className="nav-icon">ⓘ</span>{locale === "ja" ? "このアプリについて" : "About"}</button>
        </div>
      </aside>}
      <main className="app-main">
        <header className="app-topbar">
          <Button variant="ghost" aria-label={locale === "ja" ? "サイドバーの表示切替" : "Toggle sidebar"} onClick={() => setSidebarOpen((open) => !open)}>☰</Button>
          <div className="topbar-title">{titleFor(activePage, locale)}</div>
          <div className="topbar-status" title={connectionError || undefined}><span className={`status-dot ${bridge ? "connected" : "disconnected"}`} />{bridge ? (locale === "ja" ? "接続済み" : "Connected") : (locale === "ja" ? "未接続" : "Disconnected")}</div>
          <Button variant="outline" onClick={() => setLocale((value) => value === "ja" ? "en" : "ja")}>{locale === "ja" ? "EN" : "日本語"}</Button>
          {pendingCount > 0 && <Button variant="outline" onClick={() => { for (const id of pending.current.keys()) bridge?.cancelOperation(id); }}>{locale === "ja" ? "キャンセル" : "Cancel"} ({pendingCount})</Button>}
        </header>
        <div className="tabs-bar" role="tablist" aria-label={locale === "ja" ? "開いている画面" : "Open pages"}>
          {openPages.map((page, index) => <div key={page} className={`page-tab ${activePage === page ? "active" : ""}`} role="presentation"><button role="tab" aria-selected={activePage === page} onClick={() => setActivePage(page)}>{titleFor(page, locale)}{index < 9 && <kbd>⌘{index + 1}</kbd>}</button>{page !== "welcome" && <button className="tab-close" aria-label={`${locale === "ja" ? "閉じる" : "Close"} ${titleFor(page, locale)}`} onClick={() => closePage(page)}>×</button>}</div>)}
          <span className="contract-label">Bridge {bridge?.contractVersion ?? BRIDGE_CONTRACT_VERSION}</span>
        </div>
        <div className="page-stack">{openPages.map((page) => <section key={page} className="page-host" hidden={activePage !== page} aria-label={titleFor(page, locale)}>
          {page === "welcome" ? <WelcomePage locale={locale} onOpen={openPage} /> :
            page === "settings" ? <SettingsPage locale={locale} run={runOperation} onLocaleChange={setLocale} onThemeChange={setTheme} /> :
              page === "about" ? <AboutPage locale={locale} /> : <ToolPage id={page} locale={locale} run={runOperation} active={activePage === page} droppedImage={droppedImage} onDroppedImageConsumed={clearDroppedImage} />}
        </section>)}</div>
      </main>
    </div>
  );
}

function WelcomePage({ locale, onOpen }: { locale: Locale; onOpen: (page: PageId) => void }) {
  return <div className="welcome-page"><div className="welcome-copy"><div className="eyebrow">DEVELOPER TOOLKIT</div><h1>{locale === "ja" ? "必要なツールを、ひとつの場所に。" : "Your everyday tools, in one place."}</h1><p>{locale === "ja" ? "DevToolsはC++の処理を保ちながら、ReactとTypeScriptで画面を提供します。左のメニューからツールを選んでください。" : "DevTools keeps its native C++ services and presents them through a shared React and TypeScript interface. Choose a tool from the sidebar."}</p></div><div className="tool-card-grid">{tools.map((tool) => <button key={tool.id} className="tool-card" onClick={() => onOpen(tool.id)}><span className="tool-card-icon">{tool.en.slice(0, 1)}</span><strong>{locale === "ja" ? tool.ja : tool.en}</strong><span>{locale === "ja" ? tool.detailJa : tool.detailEn}</span></button>)}</div></div>;
}

type AppSettings = { language: "en" | "ja_JP"; theme: "system" | "light" | "dark"; showSidebarOnStartup: boolean; showLastToolOnStartup: boolean; alwaysOnTop: boolean; rememberWindowSize: boolean; rememberWindowPosition: boolean };
const defaultSettings = (locale: Locale): AppSettings => ({ language: locale === "ja" ? "ja_JP" : "en", theme: "system", showSidebarOnStartup: true, showLastToolOnStartup: false, alwaysOnTop: false, rememberWindowSize: true, rememberWindowPosition: true });

function SettingsPage({ locale, run, onLocaleChange, onThemeChange }: { locale: Locale; run: OperationRunner; onLocaleChange: (locale: Locale) => void; onThemeChange: (theme: "system" | "light" | "dark") => void }) {
  const [settings, setSettings] = useState(defaultSettings(locale));
  const [message, setMessage] = useState("");
  useEffect(() => { void run("getSettings", {}).then((result) => setSettings((current) => ({ ...current, ...result } as AppSettings))).catch(() => {}); }, [run]);
  const labels: Record<keyof AppSettings, string> = { language: locale === "ja" ? "言語" : "Language", theme: locale === "ja" ? "テーマ" : "Theme", showSidebarOnStartup: locale === "ja" ? "起動時にサイドバーを表示" : "Show sidebar on startup", showLastToolOnStartup: locale === "ja" ? "前回のツールを開く" : "Open last used tool on startup", alwaysOnTop: locale === "ja" ? "ウィンドウを常に手前に表示" : "Keep window on top", rememberWindowSize: locale === "ja" ? "ウィンドウサイズを記憶" : "Remember window size", rememberWindowPosition: locale === "ja" ? "ウィンドウ位置を記憶" : "Remember window position" };
  const save = async () => { setMessage(""); try { await run("saveSettings", settings); onLocaleChange(settings.language === "ja_JP" ? "ja" : "en"); onThemeChange(settings.theme); setMessage(locale === "ja" ? "設定を保存しました。" : "Settings saved."); } catch (error) { setMessage(error instanceof Error ? error.message : String(error)); } };
  return <div className="settings-page"><PageHeading title={locale === "ja" ? "設定" : "Settings"} description={locale === "ja" ? "DevToolsの起動と表示を設定します。" : "Choose how DevTools starts and appears."} /><div className="settings-card"><label className="field-label" htmlFor="app-language">{labels.language}</label><select id="app-language" className="form-control" value={settings.language} onChange={(event) => setSettings((value) => ({ ...value, language: event.target.value as AppSettings["language"] }))}><option value="en">English</option><option value="ja_JP">日本語</option></select><label className="field-label" htmlFor="app-theme">{labels.theme}</label><select id="app-theme" className="form-control" value={settings.theme} onChange={(event) => setSettings((value) => ({ ...value, theme: event.target.value as AppSettings["theme"] }))}><option value="system">{locale === "ja" ? "システム設定に合わせる" : "Follow system"}</option><option value="light">{locale === "ja" ? "ライト" : "Light"}</option><option value="dark">{locale === "ja" ? "ダーク" : "Dark"}</option></select>{(Object.keys(labels) as Array<keyof AppSettings>).filter((key) => key !== "language" && key !== "theme").map((key) => <label key={key} className="settings-toggle"><span>{labels[key]}</span><input type="checkbox" checked={settings[key] as boolean} onChange={() => setSettings((value) => ({ ...value, [key]: !value[key] }))} /></label>)}<div className="settings-actions"><span role="status">{message}</span><Button onClick={() => void save()}>{locale === "ja" ? "保存" : "Save"}</Button></div></div></div>;
}

function AboutPage({ locale }: { locale: Locale }) {
  return <div className="about-page"><PageHeading title={locale === "ja" ? "DevToolsについて" : "About DevTools"} description={locale === "ja" ? "日々の開発作業に使うデスクトップツールです。" : "A desktop toolkit for everyday development work."} /><div className="about-card"><div className="brand-mark large">D</div><div><strong>DevTools</strong><p>{locale === "ja" ? "C++のネイティブ処理とReactのユーザーインターフェースを組み合わせています。" : "Combines native C++ services with a React user interface."}</p></div></div></div>;
}

function PageHeading({ title, description }: { title: string; description: string }) { return <header className="page-heading"><h1>{title}</h1><p>{description}</p></header>; }

export default App;
