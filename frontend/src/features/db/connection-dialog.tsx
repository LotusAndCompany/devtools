import { Button } from "@/shared/components/ui/button";
import {
    Dialog,
    DialogClose,
    DialogContent,
    DialogDescription,
    DialogFooter,
    DialogHeader,
    DialogTitle,
} from "@/shared/components/ui/dialog";
import { FormField } from "@/shared/components/form-field";
import { Input } from "@/shared/components/ui/input";
import {
    Item,
    ItemActions,
    ItemContent,
    ItemDescription,
    ItemGroup,
    ItemTitle,
} from "@/shared/components/ui/item";
import { SelectControl } from "@/shared/components/select-control";
import { text } from "@/shared/tool-page";
import type { DatabaseDriver } from "@/shared/bridge/types";
import type { Locale } from "@/shared/types";
import type { DatabaseHistoryEntry } from "./types";

interface DatabaseConnectionDialogProps {
    locale: Locale;
    open: boolean;
    history: DatabaseHistoryEntry[];
    busy: boolean;
    connected: boolean;
    type: DatabaseDriver;
    host: string;
    database: string;
    username: string;
    password: string;
    selectedFileName: string;
    onOpenChange: (open: boolean) => void;
    onTypeChange: (value: DatabaseDriver) => void;
    onHostChange: (value: string) => void;
    onDatabaseChange: (value: string) => void;
    onUsernameChange: (value: string) => void;
    onPasswordChange: (value: string) => void;
    onConnectSaved: (index: number) => void;
    onConnect: () => void;
    onSelectFile: () => void;
    onDisconnect: () => void;
}

export function DatabaseConnectionDialog({
    locale,
    open,
    history,
    busy,
    connected,
    type,
    host,
    database,
    username,
    password,
    selectedFileName,
    onOpenChange,
    onTypeChange,
    onHostChange,
    onDatabaseChange,
    onUsernameChange,
    onPasswordChange,
    onConnectSaved,
    onConnect,
    onSelectFile,
    onDisconnect,
}: DatabaseConnectionDialogProps) {
    const canConnect =
        !busy &&
        (type === "QSQLITE" ? Boolean(selectedFileName) : Boolean(host && database && username));

    return (
        <Dialog open={open} onOpenChange={onOpenChange}>
            <DialogContent
                className="max-h-[min(760px,calc(100vh-40px))] overflow-y-auto sm:max-w-[648px]"
                showCloseButton={false}
            >
                <DialogHeader>
                    <DialogTitle>
                        {text(locale, "Database Connection Settings", "データベース接続設定")}
                    </DialogTitle>
                    <DialogDescription className="sr-only">
                        {text(locale, "Configure a database connection", "データベース接続を設定")}
                    </DialogDescription>
                </DialogHeader>
                <div className="db-connection-content">
                    {history.length > 0 && (
                        <section className="db-history-section">
                            <h2>{text(locale, "Recent connections", "最近の接続")}</h2>
                            <ItemGroup className="history-list">
                                {history.map((item) => (
                                    <Item
                                        key={String(item.index)}
                                        variant="muted"
                                        size="sm"
                                        className="history-item"
                                    >
                                        <ItemContent>
                                            <ItemTitle>{String(item.displayName)}</ItemTitle>
                                            <ItemDescription>
                                                {String(item.host || item.database)}
                                            </ItemDescription>
                                        </ItemContent>
                                        <ItemActions>
                                            <Button
                                                variant="outline"
                                                size="sm"
                                                disabled={busy}
                                                onClick={() => onConnectSaved(Number(item.index))}
                                            >
                                                {text(locale, "Connect", "接続")}
                                            </Button>
                                        </ItemActions>
                                    </Item>
                                ))}
                            </ItemGroup>
                        </section>
                    )}
                    <div className={`db-connect-form${history.length > 0 ? " has-history" : ""}`}>
                        <h2>{text(locale, "New connection", "新しい接続")}</h2>
                        <FormField
                            label={text(locale, "Database type", "データベースの種類")}
                            nativeLabel={false}
                        >
                            <SelectControl
                                value={type}
                                onValueChange={onTypeChange}
                                options={[
                                    { value: "QSQLITE", label: "SQLite" },
                                    { value: "QMYSQL", label: "MySQL" },
                                    { value: "QPSQL", label: "PostgreSQL" },
                                ]}
                            />
                        </FormField>
                        {type === "QSQLITE" ? (
                            <>
                                <div className="selected-file">
                                    {selectedFileName ||
                                        text(locale, "No file selected", "ファイル未選択")}
                                </div>
                                <Button variant="outline" onClick={onSelectFile} disabled={busy}>
                                    {text(locale, "Choose SQLite file", "SQLiteファイルを選択")}
                                </Button>
                            </>
                        ) : (
                            <>
                                <FormField label={text(locale, "Host", "ホスト")}>
                                    <Input
                                        value={host}
                                        onChange={(event) => onHostChange(event.target.value)}
                                    />
                                </FormField>
                                <FormField label={text(locale, "Database", "データベース")}>
                                    <Input
                                        value={database}
                                        onChange={(event) => onDatabaseChange(event.target.value)}
                                    />
                                </FormField>
                                <FormField label={text(locale, "Username", "ユーザー名")}>
                                    <Input
                                        value={username}
                                        onChange={(event) => onUsernameChange(event.target.value)}
                                    />
                                </FormField>
                                <FormField label={text(locale, "Password", "パスワード")}>
                                    <Input
                                        type="password"
                                        value={password}
                                        onChange={(event) => onPasswordChange(event.target.value)}
                                    />
                                </FormField>
                            </>
                        )}
                        <div className="button-row">
                            <Button onClick={onConnect} disabled={!canConnect}>
                                {text(locale, "Connect", "接続")}
                            </Button>
                            <Button
                                variant="outline"
                                onClick={onDisconnect}
                                disabled={!connected || busy}
                            >
                                {text(locale, "Disconnect", "切断")}
                            </Button>
                        </div>
                    </div>
                </div>
                <DialogFooter>
                    <DialogClose render={<Button variant="outline" />}>
                        {text(locale, "Close", "閉じる")}
                    </DialogClose>
                </DialogFooter>
            </DialogContent>
        </Dialog>
    );
}
