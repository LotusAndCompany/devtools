import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Dialog } from "@/shared/components/ui/dialog";
import { Tabs } from "@/shared/components/ui/tabs";
import { Textarea } from "@/shared/components/ui/textarea";
import { type AboutInfo, type OperationRunner } from "@/shared/bridge/types";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import { devToolsLogoUrl } from "@/shared/lib/resources";
import { type Locale } from "@/shared/types";
import { licenseTextOrFallback } from "./license";

export function AboutDialog({
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
                <Dialog.Backdrop />
                <Dialog.Viewport>
                    <Dialog.Popup data-size="wide">
                        <Dialog.Title>
                            {locale === "ja" ? "DevToolsについて" : "About DevTools"}
                        </Dialog.Title>
                        <Dialog.Description className="sr-only">
                            {locale === "ja"
                                ? "DevToolsのアプリケーション情報とライセンス"
                                : "DevTools application information and license"}
                        </Dialog.Description>
                        <Tabs.Root defaultValue="about" className="about-tabs">
                            <Tabs.List>
                                <Tabs.Tab value="about">
                                    {locale === "ja" ? "概要" : "About"}
                                </Tabs.Tab>
                                <Tabs.Tab value="license">
                                    {locale === "ja" ? "ライセンス" : "License"}
                                </Tabs.Tab>
                            </Tabs.List>
                            <Tabs.Panel value="about" className="about-panel">
                                <div className="about-brand">
                                    <img src={devToolsLogoUrl} alt="" />
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
                        <div data-slot="dialog-footer">
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
