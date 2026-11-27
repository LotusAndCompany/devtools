import { useCallback, useEffect, useState, type MouseEvent } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { FeedbackAlert } from "@/shared/components/feedback-alert";
import { toast } from "@/shared/components/ui/toast";
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
    openExternalUrl,
}: {
    open: boolean;
    onOpenChange: (open: boolean) => void;
    locale: Locale;
    run: OperationRunner;
    openExternalUrl: (url: string) => Promise<void>;
}) {
    const [info, setInfo] = useState<AboutInfo | null>(null);
    const [error, setError] = useState("");
    const [licenseHtml, setLicenseHtml] = useState("");

    useEffect(() => {
        if (!open) return;
        const load = run("getAboutInfo", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    setInfo(result as AboutInfo);
                    setError("");
                }),
            ),
            Effect.flatMap((result) =>
                run("renderMarkdown", { markdown: result.licenseText }).pipe(
                    Effect.tap((rendered) =>
                        Effect.sync(() => {
                            setLicenseHtml(
                                new DOMParser().parseFromString(rendered.html, "text/html").body
                                    .innerHTML,
                            );
                        }),
                    ),
                ),
            ),
            Effect.catch((reason) =>
                Effect.sync(() => {
                    setError(reason instanceof Error ? reason.message : String(reason));
                }),
            ),
        );
        return forkUiEffect(load);
    }, [open, run]);

    const openLicenseLink = useCallback(
        (event: MouseEvent<HTMLDivElement>) => {
            const href = (event.target as Element | null)
                ?.closest?.("a[href]")
                ?.getAttribute("href");
            if (!href || href.startsWith("#")) return;

            event.preventDefault();

            let url: URL;
            try {
                url = new URL(href, window.location.href);
            } catch {
                return;
            }
            if (url.protocol !== "https:") return;

            setError("");
            void openExternalUrl(url.href).catch((reason) => {
                setError(reason instanceof Error ? reason.message : String(reason));
            });
        },
        [openExternalUrl],
    );

    const copy = () => {
        if (!info) return;
        const summary = [
            `DevTools ${info.appVersion}`,
            `Revision: ${info.revision}`,
            `Build Environment: ${info.buildEnvironment}`,
            `Runtime: ${info.runtime}`,
            info.copyright,
        ].join("\n");
        setError("");
        runUiEffect(
            run("copyToClipboard", { text: summary }).pipe(
                Effect.tap(() =>
                    Effect.sync(() =>
                        toast.add({
                            title: locale === "ja" ? "コピーしました。" : "Copied.",
                            type: "success",
                        }),
                    ),
                ),
                Effect.catch((reason) =>
                    Effect.sync(() =>
                        setError(reason instanceof Error ? reason.message : String(reason)),
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
              [locale === "ja" ? "実行基盤" : "Runtime", info.runtime],
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
                <Tabs defaultValue="about">
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
                            <FeedbackAlert
                                title={
                                    locale === "ja" ? "アプリ情報エラー" : "About information error"
                                }
                                description={error}
                            />
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
                            <Button variant="outline" onClick={copy} disabled={!info}>
                                {locale === "ja" ? "コピー" : "Copy"}
                            </Button>
                        </div>
                    </TabsContent>
                    <TabsContent value="license" className="about-panel">
                        {error && (
                            <FeedbackAlert
                                title={
                                    locale === "ja"
                                        ? "ライセンス表示エラー"
                                        : "License display error"
                                }
                                description={error}
                            />
                        )}
                        <div
                            className="license-scroll-area"
                            role="region"
                            aria-label={locale === "ja" ? "ライセンス情報" : "License information"}
                            tabIndex={0}
                        >
                            {licenseHtml ? (
                                <div
                                    className="markdown-preview-content"
                                    onClick={openLicenseLink}
                                    dangerouslySetInnerHTML={{ __html: licenseHtml }}
                                />
                            ) : (
                                <p className="license-status" role="status">
                                    {error
                                        ? licenseTextOrFallback("", locale)
                                        : locale === "ja"
                                          ? "ライセンスを読み込み中…"
                                          : "Loading license…"}
                                </p>
                            )}
                        </div>
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
