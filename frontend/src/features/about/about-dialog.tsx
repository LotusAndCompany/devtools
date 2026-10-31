import { useEffect, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Alert, AlertDescription } from "@/shared/components/ui/alert";
import {
    Dialog,
    DialogClose,
    DialogContent,
    DialogDescription,
    DialogFooter,
    DialogHeader,
    DialogTitle,
} from "@/shared/components/ui/dialog";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/shared/components/ui/tabs";
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
        <Dialog open={open} onOpenChange={onOpenChange}>
            <DialogContent
                className="max-h-[min(760px,calc(100vh-40px))] overflow-y-auto sm:max-w-[648px]"
                showCloseButton={false}
            >
                <DialogHeader>
                    <DialogTitle>
                        {locale === "ja" ? "DevToolsについて" : "About DevTools"}
                    </DialogTitle>
                    <DialogDescription className="sr-only">
                        {locale === "ja"
                            ? "DevToolsのアプリケーション情報とライセンス"
                            : "DevTools application information and license"}
                    </DialogDescription>
                </DialogHeader>
                <Tabs defaultValue="about" className="about-tabs">
                    <TabsList variant="line">
                        <TabsTrigger value="about">
                            {locale === "ja" ? "概要" : "About"}
                        </TabsTrigger>
                        <TabsTrigger value="license">
                            {locale === "ja" ? "ライセンス" : "License"}
                        </TabsTrigger>
                    </TabsList>
                    <TabsContent value="about" className="about-panel">
                        <div className="about-brand">
                            <img src={devToolsLogoUrl} alt="" />
                            <strong>DevTools</strong>
                        </div>
                        {error && (
                            <Alert variant="destructive" className="operation-message error">
                                <AlertDescription>{error}</AlertDescription>
                            </Alert>
                        )}
                        <dl className="about-metadata">
                            {metadata.map(([label, value]) => (
                                <div key={label}>
                                    <dt>{label}</dt>
                                    <dd>{value}</dd>
                                </div>
                            ))}
                        </dl>
                        <div className="about-actions">
                            {message && (
                                <Alert role="status" className="about-status">
                                    <AlertDescription>{message}</AlertDescription>
                                </Alert>
                            )}
                            <Button variant="outline" onClick={copy} disabled={!info}>
                                {locale === "ja" ? "コピー" : "Copy"}
                            </Button>
                        </div>
                    </TabsContent>
                    <TabsContent value="license" className="about-panel">
                        <Textarea
                            className="license-text"
                            value={licenseTextOrFallback(info?.licenseText, locale)}
                            readOnly
                            spellCheck={false}
                        />
                    </TabsContent>
                </Tabs>
                <DialogFooter>
                    <DialogClose render={<Button variant="outline" />}>
                        {locale === "ja" ? "OK" : "OK"}
                    </DialogClose>
                </DialogFooter>
            </DialogContent>
        </Dialog>
    );
}
