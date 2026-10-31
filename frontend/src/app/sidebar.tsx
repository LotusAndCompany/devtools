import { useCallback, useEffect, useRef, useState } from "react";
import { House, Info, Search, Settings } from "lucide-react";
import { InputGroup, InputGroupAddon, InputGroupInput } from "@/shared/components/ui/input-group";
import {
    Sidebar as SidebarPrimitive,
    SidebarContent,
    SidebarFooter,
    SidebarGroup,
    SidebarGroupContent,
    SidebarGroupLabel,
    SidebarHeader,
    SidebarMenu,
    SidebarMenuButton,
    SidebarMenuItem,
    SidebarSeparator,
    useSidebar,
} from "@/shared/components/ui/sidebar";
import { devToolsLogoUrl } from "@/shared/lib/resources";
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
    const focusSearchAfterExpand = useRef(false);
    const { isMobile, open, setOpen } = useSidebar();

    const focusSearch = useCallback(() => {
        if (!isMobile && !open) {
            focusSearchAfterExpand.current = true;
            setOpen(true);
            return;
        }
        searchRef.current?.focus();
    }, [isMobile, open, setOpen]);

    useEffect(() => {
        if (open && focusSearchAfterExpand.current) {
            searchRef.current?.focus();
            focusSearchAfterExpand.current = false;
        }
    }, [open]);

    useEffect(() => {
        const onKeyDown = (event: KeyboardEvent) => {
            const command = event.metaKey || event.ctrlKey;
            const target = event.target as HTMLElement | null;
            const editing =
                target?.matches("input, textarea, select, [contenteditable='true']") ?? false;
            if (command && event.key.toLowerCase() === "k") {
                event.preventDefault();
                focusSearch();
            } else if (!editing && event.key === "Escape") {
                setSearch("");
                searchRef.current?.blur();
            }
        };
        window.addEventListener("keydown", onKeyDown);
        return () => window.removeEventListener("keydown", onKeyDown);
    }, [focusSearch]);

    const searchLabel = locale === "ja" ? "ツールを検索" : "Search tools";
    const homeLabel = locale === "ja" ? "ホーム" : "Home";
    const settingsLabel = locale === "ja" ? "設定" : "Settings";
    const aboutLabel = locale === "ja" ? "このアプリについて" : "About";

    const query = search.trim().toLowerCase();
    const filteredTools = tools.filter(
        (tool) =>
            !query ||
            `${tool.en} ${tool.ja} ${tool.detailEn} ${tool.detailJa}`.toLowerCase().includes(query),
    );

    return (
        <SidebarPrimitive collapsible="icon" className="app-sidebar-panel">
            <SidebarHeader className="app-sidebar-header">
                <div data-slot="brand-row" className="brand-row">
                    <img
                        data-slot="brand-logo"
                        className="brand-logo"
                        src={devToolsLogoUrl}
                        alt=""
                    />
                    <strong>DevTools</strong>
                </div>
                <InputGroup className="app-sidebar-search" aria-label={searchLabel}>
                    <InputGroupAddon aria-hidden="true">
                        <Search />
                    </InputGroupAddon>
                    <InputGroupInput
                        ref={searchRef}
                        value={search}
                        onChange={(event) => setSearch(event.target.value)}
                        placeholder={searchLabel}
                        aria-label={searchLabel}
                    />
                </InputGroup>
                <SidebarMenu className="app-sidebar-search-menu">
                    <SidebarMenuItem>
                        <SidebarMenuButton
                            aria-label={searchLabel}
                            tooltip={searchLabel}
                            onClick={focusSearch}
                        >
                            <Search aria-hidden="true" />
                        </SidebarMenuButton>
                    </SidebarMenuItem>
                </SidebarMenu>
            </SidebarHeader>
            <SidebarContent>
                <nav className="app-sidebar-nav" aria-label={locale === "ja" ? "ツール" : "Tools"}>
                    <SidebarGroup>
                        <SidebarGroupContent>
                            <SidebarMenu className="app-sidebar-menu">
                                <SidebarMenuItem>
                                    <SidebarMenuButton
                                        isActive={activePage === "welcome"}
                                        tooltip={homeLabel}
                                        onClick={() => onNavigate("welcome")}
                                    >
                                        <House aria-hidden="true" />
                                        <span>{homeLabel}</span>
                                    </SidebarMenuButton>
                                </SidebarMenuItem>
                            </SidebarMenu>
                        </SidebarGroupContent>
                    </SidebarGroup>
                    <SidebarGroup>
                        <SidebarGroupLabel>
                            {locale === "ja" ? "ツール" : "Tools"}
                        </SidebarGroupLabel>
                        <SidebarGroupContent>
                            <SidebarMenu className="app-sidebar-menu">
                                {filteredTools.map((tool) => {
                                    const ToolIcon = iconFor(tool.id);
                                    const toolLabel = locale === "ja" ? tool.ja : tool.en;
                                    return (
                                        <SidebarMenuItem key={tool.id}>
                                            <SidebarMenuButton
                                                isActive={activePage === tool.id}
                                                tooltip={toolLabel}
                                                onClick={() => onNavigate(tool.id)}
                                            >
                                                <ToolIcon aria-hidden="true" />
                                                <span>{toolLabel}</span>
                                            </SidebarMenuButton>
                                        </SidebarMenuItem>
                                    );
                                })}
                            </SidebarMenu>
                        </SidebarGroupContent>
                    </SidebarGroup>
                </nav>
            </SidebarContent>
            <SidebarFooter className="app-sidebar-footer">
                <SidebarSeparator />
                <SidebarGroup>
                    <SidebarGroupContent>
                        <SidebarMenu className="app-sidebar-menu">
                            <SidebarMenuItem>
                                <SidebarMenuButton
                                    isActive={settingsOpen}
                                    aria-haspopup="dialog"
                                    tooltip={settingsLabel}
                                    onClick={onOpenSettings}
                                >
                                    <Settings aria-hidden="true" />
                                    <span>{settingsLabel}</span>
                                </SidebarMenuButton>
                            </SidebarMenuItem>
                            <SidebarMenuItem>
                                <SidebarMenuButton tooltip={aboutLabel} onClick={onOpenAbout}>
                                    <Info aria-hidden="true" />
                                    <span>{aboutLabel}</span>
                                </SidebarMenuButton>
                            </SidebarMenuItem>
                        </SidebarMenu>
                    </SidebarGroupContent>
                </SidebarGroup>
            </SidebarFooter>
        </SidebarPrimitive>
    );
}
