import { useEffect, useState, type KeyboardEvent } from "react";
import { Effect } from "effect";
import { PhraseConfirmationDialogs } from "./phrase-confirmation-dialogs";
import { PhraseEditor } from "./phrase-editor";
import { PhraseList } from "./phrase-list";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import { Heading, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";

export function PhrasePage({ locale, run, active }: ToolPageProps) {
    type PhraseDraft = { title: string; content: string };
    type PendingAction = { type: "new" } | { type: "select"; item: Record<string, unknown> };
    const { busy, error, execute, setError } = useOperation(run);
    const [templates, setTemplates] = useState<Array<Record<string, unknown>>>([]);
    const [filename, setFilename] = useState("");
    const [title, setTitle] = useState("");
    const [content, setContent] = useState("");
    const [savedDraft, setSavedDraft] = useState<PhraseDraft | null>(null);
    const [message, setMessage] = useState("");
    const [pendingAction, setPendingAction] = useState<PendingAction | null>(null);
    const [unsavedDialogOpen, setUnsavedDialogOpen] = useState(false);
    const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
    const hasUnsavedChanges = savedDraft
        ? title !== savedDraft.title || content !== savedDraft.content
        : Boolean(title || content);
    const refresh = () =>
        Effect.gen(function* () {
            const result = yield* execute("phraseData", { action: "list" });
            if (result) {
                yield* Effect.sync(() =>
                    setTemplates((result.templates as Array<Record<string, unknown>>) ?? []),
                );
            }
        });
    useEffect(() => {
        return forkUiEffect(
            run("phraseData", { action: "list" }).pipe(
                Effect.tap((result) =>
                    Effect.sync(() =>
                        setTemplates((result.templates as Array<Record<string, unknown>>) ?? []),
                    ),
                ),
                Effect.catch(() => Effect.void),
            ),
        );
    }, [run]);
    const applyAction = (action: PendingAction) => {
        if (action.type === "new") {
            setFilename("");
            setTitle("");
            setContent("");
            setSavedDraft(null);
        } else {
            const nextFilename = String(action.item.filename ?? "");
            const nextDraft = {
                title: String(action.item.title ?? ""),
                content: String(action.item.content ?? ""),
            };
            setFilename(nextFilename);
            setTitle(nextDraft.title);
            setContent(nextDraft.content);
            setSavedDraft(nextDraft);
        }
        setMessage("");
    };
    const requestAction = (action: PendingAction) => {
        if (action.type === "select" && String(action.item.filename ?? "") === filename) return;
        if (hasUnsavedChanges) {
            setPendingAction(action);
            setUnsavedDialogOpen(true);
            return;
        }
        applyAction(action);
    };
    const save = () =>
        Effect.gen(function* () {
            const result = yield* execute("phraseData", {
                action: "save",
                filename,
                title,
                content,
            });
            if (!result) return false;
            const nextDraft = { title, content };
            yield* Effect.sync(() => {
                setFilename(String(result.filename));
                setSavedDraft(nextDraft);
                setMessage(text(locale, "Template saved.", "定型文を保存しました。"));
            });
            yield* refresh();
            return true;
        });
    const remove = () =>
        Effect.gen(function* () {
            if (!filename) return;
            const result = yield* execute("phraseData", { action: "delete", filename });
            if (result) {
                yield* Effect.sync(() => {
                    setFilename("");
                    setTitle("");
                    setContent("");
                    setSavedDraft(null);
                    setMessage(text(locale, "Template deleted.", "定型文を削除しました。"));
                });
                yield* refresh();
            }
        });
    const copy = () =>
        execute("copyToClipboard", { text: content }).pipe(
            Effect.tap((result) =>
                Effect.sync(() =>
                    setMessage(
                        result?.copied
                            ? text(
                                  locale,
                                  "Copied to clipboard.",
                                  "クリップボードにコピーしました。",
                              )
                            : "",
                    ),
                ),
            ),
        );
    const continueWithoutSaving = () => {
        if (pendingAction) applyAction(pendingAction);
        setPendingAction(null);
        setUnsavedDialogOpen(false);
    };
    const saveAndContinue = () => {
        const action = pendingAction;
        setPendingAction(null);
        setUnsavedDialogOpen(false);
        runUiEffect(
            save().pipe(
                Effect.tap((saved) => Effect.sync(() => saved && action && applyAction(action))),
            ),
        );
    };
    const requestDelete = () => {
        if (filename) setDeleteDialogOpen(true);
    };
    const confirmDelete = () => {
        setDeleteDialogOpen(false);
        runUiEffect(remove());
    };
    const onKeyDown = (event: KeyboardEvent<HTMLDivElement>) => {
        if (!active || !(event.metaKey || event.ctrlKey)) return;
        const key = event.key.toLowerCase();
        if (key === "s") {
            event.preventDefault();
            if (!busy) runUiEffect(save());
        } else if (key === "n") {
            event.preventDefault();
            requestAction({ type: "new" });
        } else if (key === "c" && event.shiftKey) {
            event.preventDefault();
            if (content) runUiEffect(copy());
        }
    };
    const onDeleteKey = (event: KeyboardEvent<HTMLDivElement>) => {
        const target = event.target;
        const isEditor =
            target instanceof HTMLElement &&
            (target.isContentEditable || ["INPUT", "TEXTAREA", "SELECT"].includes(target.tagName));
        if (!active || isEditor || !["Backspace", "Delete"].includes(event.key) || !filename)
            return;
        event.preventDefault();
        if (!busy) requestDelete();
    };
    return (
        <div
            className="tool-page"
            onKeyDown={(event) => {
                onKeyDown(event);
                onDeleteKey(event);
            }}
        >
            <Heading title={text(locale, "Phrase Generation", "定型文生成")} />
            <div className="phrase-layout">
                <PhraseList
                    locale={locale}
                    templates={templates}
                    filename={filename}
                    onNew={() => requestAction({ type: "new" })}
                    onSelect={(item) => requestAction({ type: "select", item })}
                />
                <PhraseEditor
                    locale={locale}
                    title={title}
                    content={content}
                    busy={busy}
                    filename={filename}
                    error={error}
                    message={message}
                    onTitleChange={setTitle}
                    onContentChange={setContent}
                    onSave={() => runUiEffect(save())}
                    onCopy={() => runUiEffect(copy())}
                    onDelete={requestDelete}
                    onDismissError={() => setError("")}
                />
            </div>
            <PhraseConfirmationDialogs
                locale={locale}
                title={title}
                busy={busy}
                unsavedOpen={unsavedDialogOpen}
                deleteOpen={deleteDialogOpen}
                onUnsavedOpenChange={(open) => {
                    setUnsavedDialogOpen(open);
                    if (!open) setPendingAction(null);
                }}
                onDeleteOpenChange={setDeleteDialogOpen}
                onDiscard={continueWithoutSaving}
                onSaveAndContinue={saveAndContinue}
                onConfirmDelete={confirmDelete}
            />
        </div>
    );
}
