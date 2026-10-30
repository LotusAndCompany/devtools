import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Dialog } from "@/shared/components/ui/dialog";
import { Field } from "@/shared/components/ui/field";
import { SelectBox } from "@/shared/components/ui/select";
import { Switch } from "@/shared/components/ui/switch";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import { type Locale } from "@/shared/types";
import type { ApplicationSettings, OperationRunner } from "@/shared/bridge/types";

type AppSettings = Omit<ApplicationSettings, "lastUsedTool">;
const defaultSettings = (locale: Locale): AppSettings => ({
    language: locale === "ja" ? "ja_JP" : "en",
    theme: "system",
    showSidebarOnStartup: true,
    showLastToolOnStartup: false,
    alwaysOnTop: false,
    rememberWindowSize: true,
    rememberWindowPosition: true,
});

export function SettingsDialog({
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
                Effect.sync(() => setSettings((current) => ({ ...current, ...result }))),
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
