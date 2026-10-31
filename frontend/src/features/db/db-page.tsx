import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import type { DatabaseConnectionInput, DatabaseDriver } from "@/shared/bridge/types";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";
import { DatabaseConnectionDialog } from "./connection-dialog";
import { QueryWorkspace } from "./query-workspace";
import { TableBrowser } from "./table-browser";
import type { DatabaseHistoryEntry, DatabaseQueryTab } from "./types";

export function DatabasePage({ locale, run, active, databaseNavigationVersion }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [type, setType] = useState<DatabaseDriver>("QSQLITE");
    const [connectedType, setConnectedType] = useState<DatabaseDriver>("QSQLITE");
    const [host, setHost] = useState("localhost");
    const [database, setDatabase] = useState("");
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [history, setHistory] = useState<DatabaseHistoryEntry[]>([]);
    const [selectedFileName, setSelectedFileName] = useState("");
    const [connected, setConnected] = useState(false);
    const [displayName, setDisplayName] = useState("");
    const [tables, setTables] = useState<string[]>([]);
    const [connectionDialogOpen, setConnectionDialogOpen] = useState(false);
    const [queryTabs, setQueryTabs] = useState<DatabaseQueryTab[]>([
        {
            id: "query-1",
            title: "SQL",
            sql: "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;",
            columns: [],
            rows: [],
            truncated: false,
        },
    ]);
    const [activeQueryId, setActiveQueryId] = useState("query-1");
    const activeQuery = queryTabs.find((tab) => tab.id === activeQueryId) ?? queryTabs[0];
    const updateActiveQuery = (
        update: (query: (typeof queryTabs)[number]) => (typeof queryTabs)[number],
    ) =>
        setQueryTabs((current) =>
            current.map((query) => (query.id === activeQueryId ? update(query) : query)),
        );
    const addQueryTab = () => {
        const id = `query-${crypto.randomUUID()}`;
        setQueryTabs((current) => [
            ...current,
            {
                id,
                title: `SQL ${current.length + 1}`,
                sql: "",
                columns: [],
                rows: [],
                truncated: false,
            },
        ]);
        setActiveQueryId(id);
    };
    const closeQueryTab = (id: string) => {
        if (queryTabs.length < 2) return;
        const index = queryTabs.findIndex((query) => query.id === id);
        const next = queryTabs.filter((query) => query.id !== id);
        setQueryTabs(next);
        if (activeQueryId === id) setActiveQueryId(next[Math.max(0, index - 1)].id);
    };
    const refreshHistory = () =>
        run("databaseHistory", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    setHistory(result.history);
                    setSelectedFileName(result.selectedFileName);
                    if (result.connected) {
                        setConnected(true);
                        if (result.connectedType) {
                            setType(result.connectedType);
                            setConnectedType(result.connectedType);
                        }
                    }
                }),
            ),
        );
    useEffect(() => {
        if (!active) return;
        let mounted = true;
        const initialize = Effect.gen(function* () {
            const result = yield* run("databaseHistory", {});
            if (!mounted) return;
            const selected = result.selectedFileName;
            yield* Effect.sync(() => {
                setHistory(result.history);
                setSelectedFileName(selected);
                if (result.connected) {
                    setConnected(true);
                    if (result.connectedType) {
                        setType(result.connectedType);
                        setConnectedType(result.connectedType);
                    }
                }
            });
            if (selected && !result.connected) {
                const connectedResult = yield* run("databaseConnect", { type: "QSQLITE" });
                if (mounted && connectedResult.connected) {
                    yield* Effect.sync(() => {
                        setConnected(true);
                        setConnectedType("QSQLITE");
                        setDisplayName(connectedResult.displayName || `SQLite: ${selected}`);
                        setTables(connectedResult.tables);
                    });
                }
            }
        }).pipe(Effect.catch(() => Effect.void));
        const interrupt = forkUiEffect(initialize);
        return () => {
            mounted = false;
            interrupt();
        };
        // The bridge runner changes when Qt WebChannel attaches, so refresh this cache then.
        // oxlint-disable-next-line react/exhaustive-effect-dependencies
    }, [active, databaseNavigationVersion, run]);
    const selectFile = () =>
        execute("databaseSelectFile", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (!result) return;
                    setSelectedFileName(result.fileName);
                    setType("QSQLITE");
                }),
            ),
        );
    const connect = (historyIndex?: number) =>
        Effect.gen(function* () {
            const connectionType =
                historyIndex === undefined
                    ? type
                    : (history.find((item) => item.index === historyIndex)?.type ?? type);
            const payload: DatabaseConnectionInput =
                historyIndex === undefined
                    ? { type, host, database, username, password }
                    : { historyIndex, password };
            const result = yield* execute("databaseConnect", payload);
            if (!result) return;
            yield* Effect.sync(() => {
                setConnected(true);
                setType(connectionType);
                setConnectedType(connectionType);
                setDisplayName(
                    result.displayName ||
                        (connectionType === "QSQLITE"
                            ? `SQLite: ${selectedFileName}`
                            : `${database}@${host}`),
                );
                setTables(result.tables);
                setConnectionDialogOpen(false);
            });
            yield* refreshHistory();
        });
    const query = () =>
        execute("databaseQuery", { sql: activeQuery.sql }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (!result) return;
                    updateActiveQuery((current) => ({
                        ...current,
                        columns: result.columns,
                        rows: result.rows,
                        truncated: result.truncated,
                    }));
                }),
            ),
        );
    const loadTables = () =>
        execute("databaseTables", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (result) setTables(result.tables);
                }),
            ),
        );
    const disconnect = () =>
        execute("databaseDisconnect", {}).pipe(
            Effect.tap(() =>
                Effect.sync(() => {
                    setConnected(false);
                    setDisplayName("");
                    setTables([]);
                    setSelectedFileName("");
                }),
            ),
        );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Database", "データベース")} />
            <div className="database-toolbar">
                {connected && <span className="connection-name">{displayName}</span>}
                {connected && (
                    <Button variant="ghost" size="sm" onClick={() => runUiEffect(disconnect())}>
                        {text(locale, "Disconnect", "切断")}
                    </Button>
                )}
                <Button
                    variant="outline"
                    size="sm"
                    onClick={() => runUiEffect(loadTables())}
                    disabled={!connected || busy}
                >
                    {text(locale, "Refresh", "更新")}
                </Button>
                <Button
                    variant="outline"
                    size="sm"
                    onClick={() => {
                        setConnectionDialogOpen(true);
                        runUiEffect(refreshHistory());
                    }}
                >
                    {text(locale, "Connection settings", "接続設定")}
                </Button>
                <Button size="sm" onClick={addQueryTab}>
                    {text(locale, "New SQL", "SQLを追加")}
                </Button>
            </div>
            <div className="db-layout">
                <TableBrowser
                    locale={locale}
                    tables={tables}
                    connected={connected}
                    onSelectTable={(table) =>
                        updateActiveQuery((current) => ({
                            ...current,
                            sql: `SELECT * FROM ${quoteIdentifier(table, connectedType)} LIMIT 1000;`,
                        }))
                    }
                />
                <QueryWorkspace
                    locale={locale}
                    tabs={queryTabs}
                    activeId={activeQueryId}
                    connected={connected}
                    busy={busy}
                    onActiveChange={setActiveQueryId}
                    onQueryChange={(id, sql) =>
                        setQueryTabs((current) =>
                            current.map((item) => (item.id === id ? { ...item, sql } : item)),
                        )
                    }
                    onClose={closeQueryTab}
                    onExecute={() => runUiEffect(query())}
                />
            </div>
            <DatabaseConnectionDialog
                locale={locale}
                open={connectionDialogOpen}
                history={history}
                busy={busy}
                connected={connected}
                type={type}
                host={host}
                database={database}
                username={username}
                password={password}
                selectedFileName={selectedFileName}
                onOpenChange={setConnectionDialogOpen}
                onTypeChange={setType}
                onHostChange={setHost}
                onDatabaseChange={setDatabase}
                onUsernameChange={setUsername}
                onPasswordChange={setPassword}
                onConnectSaved={(index) => runUiEffect(connect(index))}
                onConnect={() => runUiEffect(connect())}
                onSelectFile={() => runUiEffect(selectFile())}
                onDisconnect={() => runUiEffect(disconnect())}
            />
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

function quoteIdentifier(identifier: string, driver: DatabaseDriver) {
    const quote = driver === "QMYSQL" ? "`" : '"';
    return `${quote}${identifier.replaceAll(quote, quote + quote)}${quote}`;
}
