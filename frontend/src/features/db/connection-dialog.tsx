import { Button } from "@/shared/components/ui/button";
import { Dialog } from "@/shared/components/ui/dialog";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
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
        <Dialog.Root open={open} onOpenChange={onOpenChange}>
            <Dialog.Portal>
                <Dialog.Backdrop className="dialog-backdrop" />
                <Dialog.Viewport className="dialog-viewport">
                    <Dialog.Popup className="db-connection-dialog">
                        <Dialog.Title className="dialog-title">
                            {text(locale, "Database Connection Settings", "データベース接続設定")}
                        </Dialog.Title>
                        <div className="db-connection-content">
                            {history.length > 0 && (
                                <section className="db-history-section">
                                    <h2>{text(locale, "Recent connections", "最近の接続")}</h2>
                                    {history.map((item) => (
                                        <div key={String(item.index)} className="history-item">
                                            <div>
                                                <strong>{String(item.displayName)}</strong>
                                                <span>{String(item.host || item.database)}</span>
                                            </div>
                                            <Button
                                                variant="outline"
                                                size="sm"
                                                disabled={busy}
                                                onClick={() => onConnectSaved(Number(item.index))}
                                            >
                                                {text(locale, "Connect", "接続")}
                                            </Button>
                                        </div>
                                    ))}
                                </section>
                            )}
                            <div className="db-connect-form">
                                <h2>{text(locale, "New connection", "新しい接続")}</h2>
                                <Field
                                    label={text(locale, "Database type", "データベースの種類")}
                                    nativeLabel={false}
                                >
                                    <SelectBox
                                        value={type}
                                        onValueChange={onTypeChange}
                                        options={[
                                            { value: "QSQLITE", label: "SQLite" },
                                            { value: "QMYSQL", label: "MySQL" },
                                            { value: "QPSQL", label: "PostgreSQL" },
                                        ]}
                                    />
                                </Field>
                                {type === "QSQLITE" ? (
                                    <>
                                        <div className="selected-file">
                                            {selectedFileName ||
                                                text(locale, "No file selected", "ファイル未選択")}
                                        </div>
                                        <Button
                                            variant="outline"
                                            onClick={onSelectFile}
                                            disabled={busy}
                                        >
                                            {text(
                                                locale,
                                                "Choose SQLite file",
                                                "SQLiteファイルを選択",
                                            )}
                                        </Button>
                                    </>
                                ) : (
                                    <>
                                        <Field label={text(locale, "Host", "ホスト")}>
                                            <Input
                                                value={host}
                                                onChange={(event) =>
                                                    onHostChange(event.target.value)
                                                }
                                            />
                                        </Field>
                                        <Field label={text(locale, "Database", "データベース")}>
                                            <Input
                                                value={database}
                                                onChange={(event) =>
                                                    onDatabaseChange(event.target.value)
                                                }
                                            />
                                        </Field>
                                        <Field label={text(locale, "Username", "ユーザー名")}>
                                            <Input
                                                value={username}
                                                onChange={(event) =>
                                                    onUsernameChange(event.target.value)
                                                }
                                            />
                                        </Field>
                                        <Field label={text(locale, "Password", "パスワード")}>
                                            <Input
                                                type="password"
                                                value={password}
                                                onChange={(event) =>
                                                    onPasswordChange(event.target.value)
                                                }
                                            />
                                        </Field>
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
                        <div className="dialog-footer">
                            <Dialog.Close render={<Button variant="outline" />}>
                                {text(locale, "Close", "閉じる")}
                            </Dialog.Close>
                        </div>
                    </Dialog.Popup>
                </Dialog.Viewport>
            </Dialog.Portal>
        </Dialog.Root>
    );
}
