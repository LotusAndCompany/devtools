import { useEffect, useRef, useState } from "react";
import { Button } from "@/shared/components/ui/button";
import { Icon } from "@/shared/components/ui/icon";
import { Input } from "@/shared/components/ui/input";
import { tools, iconFor, type PageId } from "./tool-registry";
import type { Locale } from "@/shared/types";

type SidebarProps = {
    locale: Locale;
    activePage: PageId;
    settingsOpen: boolean;
    onNavigate: (page: PageId) => void;
    onOpenSettings: () => void;
    onOpenAbout: () => void;
};

export function Sidebar({
    locale,
    activePage,
    settingsOpen,
    onNavigate,
    onOpenSettings,
    onOpenAbout,
}: SidebarProps) {
    const [search, setSearch] = useState("");
    const searchRef = useRef<HTMLInputElement>(null);

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

    const query = search.trim().toLowerCase();
    const filteredTools = tools.filter(
        (tool) =>
            !query ||
            `${tool.en} ${tool.ja} ${tool.detailEn} ${tool.detailJa}`.toLowerCase().includes(query),
    );

    return (
        <aside
            data-slot="app-sidebar"
            className="app-sidebar"
            aria-label={locale === "ja" ? "ツール" : "Tools"}
        >
            <div data-slot="brand-row" className="brand-row">
                <img
                    data-slot="brand-logo"
                    className="brand-logo"
                    src="qrc:/logo/dev-tools_logo.png"
                    alt=""
                />
                <strong>DevTools</strong>
            </div>
            <label data-slot="search-wrap" className="search-wrap">
                <span aria-hidden="true">⌕</span>
                <Input
                    className="search-control"
                    ref={searchRef}
                    value={search}
                    onChange={(event) => setSearch(event.target.value)}
                    placeholder={locale === "ja" ? "ツールを検索" : "Search tools"}
                    aria-label={locale === "ja" ? "ツールを検索" : "Search tools"}
                />
            </label>
            <Button
                variant="ghost"
                size="sm"
                className={`nav-item ${activePage === "welcome" ? "active" : ""}`}
                onClick={() => onNavigate("welcome")}
            >
                <Icon className="nav-icon">{iconFor("welcome")}</Icon>
                {locale === "ja" ? "ホーム" : "Home"}
            </Button>
            <div data-slot="sidebar-section-title" className="sidebar-section-title">
                {locale === "ja" ? "ツール" : "WORKSPACE"}
            </div>
            <nav data-slot="tool-nav" className="tool-nav">
                {filteredTools.map((tool) => (
                    <Button
                        key={tool.id}
                        variant="ghost"
                        size="sm"
                        className={`nav-item ${activePage === tool.id ? "active" : ""}`}
                        onClick={() => onNavigate(tool.id)}
                    >
                        <Icon className="nav-icon">{iconFor(tool.id)}</Icon>
                        <span>{locale === "ja" ? tool.ja : tool.en}</span>
                    </Button>
                ))}
            </nav>
            <div data-slot="sidebar-bottom" className="sidebar-bottom">
                <Button
                    variant="ghost"
                    size="sm"
                    className={`nav-item ${settingsOpen ? "active" : ""}`}
                    aria-haspopup="dialog"
                    onClick={onOpenSettings}
                >
                    <Icon className="nav-icon">settings</Icon>
                    {locale === "ja" ? "設定" : "Settings"}
                </Button>
                <Button variant="ghost" size="sm" className="nav-item" onClick={onOpenAbout}>
                    <Icon className="nav-icon">info</Icon>
                    {locale === "ja" ? "このアプリについて" : "About"}
                </Button>
            </div>
        </aside>
    );
}
