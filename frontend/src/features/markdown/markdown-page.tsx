import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { Field } from "@/shared/components/ui/field";
import { Textarea } from "@/shared/components/ui/textarea";
import { runUiEffect } from "@/shared/effect";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function MarkdownPage({ locale, run }: ToolPageProps) {
    const { busy, error, execute, setError } = useOperation(run);
    const [markdown, setMarkdown] = useState("# Markdown\n\nWrite here to preview.");
    const [preview, setPreview] = useState("");
    const [syncScroll, setSyncScroll] = useState(true);
    const editorRef = useRef<HTMLTextAreaElement>(null);
    const previewRef = useRef<HTMLIFrameElement>(null);
    const previewScrollCleanup = useRef<(() => void) | null>(null);
    useEffect(() => {
        const timer = window.setTimeout(() => {
            runUiEffect(
                run("renderMarkdown", { markdown }).pipe(
                    Effect.tap((result) =>
                        Effect.sync(() => {
                            setPreview(result.html);
                            setError("");
                        }),
                    ),
                    Effect.catch((reason) =>
                        Effect.sync(() =>
                            setError(reason instanceof Error ? reason.message : String(reason)),
                        ),
                    ),
                ),
            );
        }, 180);
        return () => window.clearTimeout(timer);
    }, [markdown, run, setError]);
    const open = () =>
        Effect.gen(function* () {
            const result = yield* execute("openTextFile", { fileType: "markdown" });
            if (result) yield* Effect.sync(() => setMarkdown(result.text));
        });
    const save = (html = false) =>
        execute("saveTextFile", {
            text: html ? preview : markdown,
            fileType: html ? "html" : "markdown",
            suggestedName: html ? "preview.html" : "document.md",
        });
    const previewDocument = useMemo(() => {
        if (!preview) return "";
        const policy = `<meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src data:; style-src 'unsafe-inline'; font-src data:; base-uri 'none'; form-action 'none'">`;
        return preview.replace("<head>", `<head>${policy}`);
    }, [preview]);
    const syncEditorToPreview = useCallback(() => {
        if (!syncScroll) return;
        const editor = editorRef.current;
        const scroller = previewRef.current?.contentDocument?.scrollingElement;
        if (!editor || !scroller) return;
        const editorRange = editor.scrollHeight - editor.clientHeight;
        const previewRange = scroller.scrollHeight - scroller.clientHeight;
        if (editorRange > 0 && previewRange > 0) {
            scroller.scrollTop = (editor.scrollTop / editorRange) * previewRange;
        }
    }, [syncScroll]);
    const applyPreviewTheme = useCallback(() => {
        const iframeDocument = previewRef.current?.contentDocument;
        if (!iframeDocument?.head) return;

        const rootStyles = getComputedStyle(document.documentElement);
        const color = (name: string) => rootStyles.getPropertyValue(name).trim();
        const style =
            iframeDocument.getElementById("devtools-preview-theme") ??
            iframeDocument.createElement("style");
        style.id = "devtools-preview-theme";
        style.textContent = `
            :root { color-scheme: ${rootStyles.colorScheme}; }
            body {
                margin: 8px;
                background: ${color("--card")};
                color: ${color("--foreground")};
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
            }
            a { color: ${color("--primary-hover")}; }
            pre, code { background: ${color("--muted")}; }
            blockquote, hr { border-color: ${color("--border")}; }
        `;
        if (!style.parentElement) iframeDocument.head.appendChild(style);
    }, []);
    const attachPreviewScroll = useCallback(() => {
        previewScrollCleanup.current?.();
        previewScrollCleanup.current = null;
        applyPreviewTheme();
        if (!syncScroll) return;
        const iframeDocument = previewRef.current?.contentDocument;
        const editor = editorRef.current;
        const scroller = iframeDocument?.scrollingElement;
        if (!iframeDocument || !editor || !scroller) return;
        const onScroll = () => {
            const editorRange = editor.scrollHeight - editor.clientHeight;
            const previewRange = scroller.scrollHeight - scroller.clientHeight;
            if (editorRange > 0 && previewRange > 0) {
                editor.scrollTop = (scroller.scrollTop / previewRange) * editorRange;
            }
        };
        iframeDocument.addEventListener("scroll", onScroll, { passive: true });
        previewScrollCleanup.current = () => iframeDocument.removeEventListener("scroll", onScroll);
    }, [applyPreviewTheme, syncScroll]);
    useEffect(() => {
        attachPreviewScroll();
        return () => previewScrollCleanup.current?.();
    }, [attachPreviewScroll]);
    useEffect(() => {
        const observer = new MutationObserver(applyPreviewTheme);
        observer.observe(document.documentElement, {
            attributes: true,
            attributeFilter: ["data-theme"],
        });
        const colorScheme = window.matchMedia("(prefers-color-scheme: dark)");
        colorScheme.addEventListener("change", applyPreviewTheme);
        applyPreviewTheme();
        return () => {
            observer.disconnect();
            colorScheme.removeEventListener("change", applyPreviewTheme);
        };
    }, [applyPreviewTheme]);
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Markdown Preview", "Markdownプレビュー")} />
            <div className="toolbar-row">
                <Button variant="outline" onClick={() => runUiEffect(open())} disabled={busy}>
                    {text(locale, "Open", "開く")}
                </Button>
                <Button variant="outline" onClick={() => runUiEffect(save())} disabled={busy}>
                    {text(locale, "Save", "保存")}
                </Button>
                <Button
                    variant="outline"
                    onClick={() => runUiEffect(save(true))}
                    disabled={!preview || busy}
                >
                    {text(locale, "Export HTML", "HTMLを出力")}
                </Button>
                <label className="markdown-sync">
                    <Checkbox checked={syncScroll} onCheckedChange={setSyncScroll} />
                    {text(locale, "Sync scroll", "スクロールを同期")}
                </label>
            </div>
            <div className="two-column markdown-columns">
                <section className="input-card markdown-editor-card">
                    <Field label={text(locale, "Editor", "エディター")}>
                        <Textarea
                            ref={editorRef}
                            className="code-area markdown-editor"
                            value={markdown}
                            onChange={(event) => setMarkdown(event.target.value)}
                            onScroll={syncEditorToPreview}
                            rows={20}
                            spellCheck={false}
                        />
                    </Field>
                </section>
                <section className="input-card preview-panel">
                    <span className="field-label">{text(locale, "Preview", "プレビュー")}</span>
                    <iframe
                        ref={previewRef}
                        title={text(locale, "Markdown preview", "Markdownプレビュー")}
                        sandbox="allow-same-origin"
                        referrerPolicy="no-referrer"
                        srcDoc={previewDocument}
                        onLoad={attachPreviewScroll}
                    />
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
