import {
    useCallback,
    useEffect,
    useMemo,
    useRef,
    useState,
    type KeyboardEvent,
    type MouseEvent,
} from "react";
import { Effect } from "effect";
import { Button } from "@/components/ui/button";
import { AlertDialog } from "@/components/ui/alert-dialog";
import { Checkbox } from "@/components/ui/checkbox";
import { Dialog } from "@/components/ui/dialog";
import { Field } from "@/components/ui/field";
import { Icon } from "@/components/ui/icon";
import { Input } from "@/components/ui/input";
import { MessageDialog } from "@/components/ui/message-dialog";
import { SelectBox } from "@/components/ui/select";
import { Tabs } from "@/components/ui/tabs";
import { Textarea } from "@/components/ui/textarea";
import { NativeOperationError, OperationCancelledError, type OperationName } from "@/bridge";
import { forkUiEffect, runUiEffect } from "@/effect";

export type Locale = "en" | "ja";
export type OperationRunner = (
    operation: OperationName,
    payload: Record<string, unknown>,
) => Effect.Effect<Record<string, unknown>, NativeOperationError | OperationCancelledError>;
export type DroppedImage = {
    id: string;
    imageDataUrl: string;
    fileName: string;
    width: number;
    height: number;
};
type ToolPageProps = {
    id: string;
    locale: Locale;
    run: OperationRunner;
    active?: boolean;
    databaseNavigationVersion?: number;
    droppedImage?: DroppedImage | null;
    onDroppedImageConsumed?: (id: string) => void;
};
type RegexOptions = {
    global: boolean;
    caseInsensitive: boolean;
    multiline: boolean;
    dotMatchesEverything: boolean;
    useUnicodeProperties: boolean;
    extendedPatternSyntax: boolean;
};

const text = (locale: Locale, en: string, ja: string) => (locale === "ja" ? ja : en);

function useOperation(run: OperationRunner) {
    const [busy, setBusy] = useState(false);
    const [error, setError] = useState("");
    const execute = (operation: OperationName, payload: Record<string, unknown>) =>
        Effect.gen(function* () {
            yield* Effect.sync(() => {
                setBusy(true);
                setError("");
            });
            return yield* run(operation, payload);
        }).pipe(
            Effect.catch((reason) =>
                Effect.sync(() => {
                    if (reason instanceof OperationCancelledError) return null;
                    setError(reason instanceof Error ? reason.message : String(reason));
                    return null;
                }),
            ),
            Effect.ensuring(Effect.sync(() => setBusy(false))),
        );
    return { busy, error, execute, setError };
}

function Heading({ title }: { title: string }) {
    return (
        <header className="page-heading sr-only">
            <h1>{title}</h1>
        </header>
    );
}

function Area({
    label,
    value,
    onChange,
    placeholder,
    rows = 12,
    readOnly = false,
}: {
    label: string;
    value: string;
    onChange?: (value: string) => void;
    placeholder?: string;
    rows?: number;
    readOnly?: boolean;
}) {
    return (
        <Field label={label}>
            <Textarea
                className="form-control code-area"
                rows={rows}
                value={value}
                onChange={(event) => onChange?.(event.target.value)}
                placeholder={placeholder}
                readOnly={readOnly}
                spellCheck={false}
            />
        </Field>
    );
}

function OutputMessage({
    locale,
    error,
    message,
    onDismiss,
}: {
    locale: Locale;
    error: string;
    message?: string;
    onDismiss: () => void;
}) {
    return (
        <>
            {message && !error && (
                <p className="operation-message success" role="status">
                    {message}
                </p>
            )}
            <MessageDialog
                open={Boolean(error)}
                onOpenChange={(open) => {
                    if (!open) onDismiss();
                }}
                title={text(locale, "Error", "エラー")}
                description={error}
                closeLabel={text(locale, "OK", "閉じる")}
            />
        </>
    );
}

export function ToolPage({
    id,
    locale,
    run,
    active,
    databaseNavigationVersion,
    droppedImage,
    onDroppedImageConsumed,
}: ToolPageProps) {
    if (id === "image")
        return (
            <ImagePage
                locale={locale}
                run={run}
                active={active ?? false}
                droppedImage={droppedImage}
                onDroppedImageConsumed={onDroppedImageConsumed}
            />
        );
    if (id === "phrase") return <PhrasePage locale={locale} run={run} active={active ?? false} />;
    if (id === "http") return <HttpPage locale={locale} run={run} />;
    if (id === "data") return <DataPage locale={locale} run={run} />;
    if (id === "db")
        return (
            <DatabasePage
                locale={locale}
                run={run}
                active={active ?? false}
                databaseNavigationVersion={databaseNavigationVersion ?? 0}
            />
        );
    if (id === "timestamp") return <TimestampPage locale={locale} run={run} />;
    if (id === "diff") return <DiffPage locale={locale} run={run} />;
    if (id === "markdown") return <MarkdownPage locale={locale} run={run} />;
    if (id === "command") return <CommandPage locale={locale} run={run} />;
    if (id === "qr") return <QrPage locale={locale} run={run} />;
    if (id === "regex") return <RegexPage locale={locale} run={run} />;
    return (
        <div className="empty-state">
            {text(locale, "This tool is not available.", "このツールは利用できません。")}
        </div>
    );
}

function ImagePage({
    locale,
    run,
    active,
    droppedImage,
    onDroppedImageConsumed,
}: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [image, setImage] = useState("");
    const [fileName, setFileName] = useState("");
    const [width, setWidth] = useState(0);
    const [height, setHeight] = useState(0);
    const [targetWidth, setTargetWidth] = useState("800");
    const [targetHeight, setTargetHeight] = useState("600");
    const [horizontalScale, setHorizontalScale] = useState("100");
    const [verticalScale, setVerticalScale] = useState("100");
    const [keepAspect, setKeepAspect] = useState(false);
    const [smooth, setSmooth] = useState(true);
    const [transparency, setTransparency] = useState("1");
    const [tolerance, setTolerance] = useState("0.1");
    const [colorMode, setColorMode] = useState("rgb");
    const [contiguousOnly, setContiguousOnly] = useState(true);
    const [splitMode, setSplitMode] = useState("division");
    const [horizontalDivisions, setHorizontalDivisions] = useState("2");
    const [verticalDivisions, setVerticalDivisions] = useState("2");
    const [cellWidth, setCellWidth] = useState("128");
    const [cellHeight, setCellHeight] = useState("128");
    const [ignoreRemainders, setIgnoreRemainders] = useState(true);

    const updateImage = (result: Record<string, unknown>) => {
        setImage(String(result.imageDataUrl ?? ""));
        setWidth(Number(result.width ?? 0));
        setHeight(Number(result.height ?? 0));
        if (result.fileName) setFileName(String(result.fileName));
    };
    useEffect(() => {
        if (!active || !droppedImage) return;
        // oxlint-disable-next-line react/set-state-in-effect -- mirror a dropped-file event from the parent prop
        setImage(droppedImage.imageDataUrl);
        setFileName(droppedImage.fileName);
        setWidth(droppedImage.width);
        setHeight(droppedImage.height);
        onDroppedImageConsumed?.(droppedImage.id);
    }, [active, droppedImage, onDroppedImageConsumed]);
    const performEdit = (action: string, extra: Record<string, unknown> = {}) =>
        Effect.gen(function* () {
            const result = yield* execute("imageEdit", { action, ...extra });
            if (result) yield* Effect.sync(() => updateImage(result));
        });
    const open = () =>
        Effect.gen(function* () {
            const result = yield* execute("imageOpen", {});
            if (result) yield* Effect.sync(() => updateImage(result));
        });
    const save = () =>
        Effect.gen(function* () {
            const result = yield* execute("imageSave", {});
            if (result) yield* Effect.sync(() => setFileName(String(result.fileName ?? fileName)));
        });
    const split = () =>
        execute("imageSplit", {
            mode: splitMode === "division" ? "division" : "cell-size",
            horizontalDivisions: Number(horizontalDivisions),
            verticalDivisions: Number(verticalDivisions),
            cellWidth: Number(cellWidth),
            cellHeight: Number(cellHeight),
            ignoreRemainders,
        });
    const chooseTransparentPoint = (event: MouseEvent<HTMLImageElement>) => {
        const rect = event.currentTarget.getBoundingClientRect();
        const x = Math.max(
            0,
            Math.min(width - 1, Math.floor(((event.clientX - rect.left) * width) / rect.width)),
        );
        const y = Math.max(
            0,
            Math.min(height - 1, Math.floor(((event.clientY - rect.top) * height) / rect.height)),
        );
        runUiEffect(
            performEdit("transparent", {
                x,
                y,
                transparency: Number(transparency),
                tolerance: Number(tolerance),
                colorMode,
                contiguousOnly,
            }),
        );
    };

    return (
        <div className="tool-page">
            <Heading title={text(locale, "Image Editor", "画像編集")} />
            <div className="toolbar-row">
                <Button variant="outline" onClick={() => runUiEffect(open())} disabled={busy}>
                    {text(locale, "Open image", "画像を開く")}
                </Button>
                <Button
                    variant="outline"
                    onClick={() =>
                        runUiEffect(
                            execute("imageReset", {}).pipe(
                                Effect.tap((result) =>
                                    Effect.sync(() => {
                                        if (result) updateImage(result);
                                    }),
                                ),
                            ),
                        )
                    }
                    disabled={busy || !image}
                >
                    {text(locale, "Reset", "リセット")}
                </Button>
                <Button onClick={() => runUiEffect(save())} disabled={busy || !image}>
                    {text(locale, "Save image", "画像を保存")}
                </Button>
                <span className="muted-text">
                    {fileName}
                    {image ? ` · ${width} × ${height}` : ""}
                </span>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
            <div className="image-editor-layout">
                <div className="image-preview-panel">
                    {image ? (
                        <img
                            src={image}
                            alt={text(
                                locale,
                                "Editable image preview. Click to select a color for transparency.",
                                "編集プレビュー。クリックして透明化する色を選択。",
                            )}
                            onClick={chooseTransparentPoint}
                        />
                    ) : (
                        <div className="empty-state">
                            {text(
                                locale,
                                "Open an image to begin.",
                                "画像を開いて編集を始めてください。",
                            )}
                        </div>
                    )}
                </div>
                <div className="image-controls">
                    <section className="input-card">
                        <h2>{text(locale, "Resize", "サイズ変更")}</h2>
                        <div className="form-grid two">
                            <Field label={text(locale, "Width (px)", "幅 (px)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="1"
                                    max="32768"
                                    value={targetWidth}
                                    onChange={(event) => setTargetWidth(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Height (px)", "高さ (px)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="1"
                                    max="32768"
                                    value={targetHeight}
                                    onChange={(event) => setTargetHeight(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Horizontal scale (%)", "横倍率 (%)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="0.1"
                                    max="1000"
                                    step="0.1"
                                    value={horizontalScale}
                                    onChange={(event) => setHorizontalScale(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Vertical scale (%)", "縦倍率 (%)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="0.1"
                                    max="1000"
                                    step="0.1"
                                    value={verticalScale}
                                    onChange={(event) => setVerticalScale(event.target.value)}
                                />
                            </Field>
                        </div>
                        <label className="inline-check">
                            <Checkbox checked={keepAspect} onCheckedChange={setKeepAspect} />
                            {text(locale, "Keep aspect ratio", "縦横比を維持")}
                        </label>
                        <label className="inline-check">
                            <Checkbox checked={smooth} onCheckedChange={setSmooth} />
                            {text(locale, "Smooth scaling", "滑らかに拡大縮小")}
                        </label>
                        <div className="button-row">
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() =>
                                    runUiEffect(
                                        performEdit("resize-size", {
                                            width: Number(targetWidth),
                                            height: Number(targetHeight),
                                            keepAspectRatio: keepAspect,
                                            smoothScaling: smooth,
                                        }),
                                    )
                                }
                            >
                                {text(locale, "Apply size", "サイズを適用")}
                            </Button>
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() =>
                                    runUiEffect(
                                        performEdit("resize-scale", {
                                            horizontalPercent: Number(horizontalScale),
                                            verticalPercent: Number(verticalScale),
                                            keepAspectRatio: keepAspect,
                                            smoothScaling: smooth,
                                        }),
                                    )
                                }
                            >
                                {text(locale, "Apply scale", "倍率を適用")}
                            </Button>
                        </div>
                    </section>
                    <section className="input-card">
                        <h2>{text(locale, "Rotate and flip", "回転・反転")}</h2>
                        <div className="button-row">
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() => runUiEffect(performEdit("rotate-left"))}
                            >
                                ↶ {text(locale, "Rotate left", "左に回転")}
                            </Button>
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() => runUiEffect(performEdit("rotate-right"))}
                            >
                                ↷ {text(locale, "Rotate right", "右に回転")}
                            </Button>
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() => runUiEffect(performEdit("flip-horizontal"))}
                            >
                                {text(locale, "Flip horizontal", "左右反転")}
                            </Button>
                            <Button
                                variant="outline"
                                disabled={busy || !image}
                                onClick={() => runUiEffect(performEdit("flip-vertical"))}
                            >
                                {text(locale, "Flip vertical", "上下反転")}
                            </Button>
                        </div>
                    </section>
                    <section className="input-card">
                        <h2>{text(locale, "Make a color transparent", "色を透明化")}</h2>
                        <p className="muted-text">
                            {text(
                                locale,
                                "Click a pixel in the preview to apply these settings.",
                                "プレビュー上のピクセルをクリックすると設定を適用します。",
                            )}
                        </p>
                        <div className="form-grid two">
                            <Field label={text(locale, "Color mode", "色空間")}>
                                <SelectBox
                                    value={colorMode}
                                    onValueChange={setColorMode}
                                    options={[
                                        { value: "rgb", label: "RGB" },
                                        { value: "hsl", label: "HSL" },
                                        { value: "hsv", label: "HSV" },
                                    ]}
                                />
                            </Field>
                            <Field label={text(locale, "Tolerance (0–1)", "許容範囲 (0–1)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="0"
                                    max="1"
                                    step="0.05"
                                    value={tolerance}
                                    onChange={(event) => setTolerance(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Transparency (0–1)", "透明度 (0–1)")}>
                                <Input
                                    className="form-control"
                                    type="number"
                                    min="0"
                                    max="1"
                                    step="0.05"
                                    value={transparency}
                                    onChange={(event) => setTransparency(event.target.value)}
                                />
                            </Field>
                        </div>
                        <label className="inline-check">
                            <Checkbox
                                checked={contiguousOnly}
                                onCheckedChange={setContiguousOnly}
                            />
                            {text(locale, "Only connected pixels", "隣接する領域のみ")}
                        </label>
                    </section>
                    <section className="input-card">
                        <h2>{text(locale, "Split image", "画像を分割")}</h2>
                        <Field label={text(locale, "Split by", "分割方法")}>
                            <SelectBox
                                value={splitMode}
                                onValueChange={setSplitMode}
                                options={[
                                    {
                                        value: "division",
                                        label: text(locale, "Number of parts", "分割数"),
                                    },
                                    {
                                        value: "cell-size",
                                        label: text(locale, "Cell size", "セルサイズ"),
                                    },
                                ]}
                            />
                        </Field>
                        {splitMode === "division" ? (
                            <div className="form-grid two">
                                <Field label={text(locale, "Columns", "横の分割数")}>
                                    <Input
                                        className="form-control"
                                        type="number"
                                        min="1"
                                        value={horizontalDivisions}
                                        onChange={(event) =>
                                            setHorizontalDivisions(event.target.value)
                                        }
                                    />
                                </Field>
                                <Field label={text(locale, "Rows", "縦の分割数")}>
                                    <Input
                                        className="form-control"
                                        type="number"
                                        min="1"
                                        value={verticalDivisions}
                                        onChange={(event) =>
                                            setVerticalDivisions(event.target.value)
                                        }
                                    />
                                </Field>
                            </div>
                        ) : (
                            <div className="form-grid two">
                                <Field label={text(locale, "Cell width", "セル幅")}>
                                    <Input
                                        className="form-control"
                                        type="number"
                                        min="1"
                                        value={cellWidth}
                                        onChange={(event) => setCellWidth(event.target.value)}
                                    />
                                </Field>
                                <Field label={text(locale, "Cell height", "セル高さ")}>
                                    <Input
                                        className="form-control"
                                        type="number"
                                        min="1"
                                        value={cellHeight}
                                        onChange={(event) => setCellHeight(event.target.value)}
                                    />
                                </Field>
                            </div>
                        )}
                        <label className="inline-check">
                            <Checkbox
                                checked={ignoreRemainders}
                                onCheckedChange={setIgnoreRemainders}
                            />
                            {text(locale, "Ignore remainder pixels", "余りのピクセルを無視")}
                        </label>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(split())}
                            disabled={busy || !image}
                        >
                            {text(locale, "Choose folder and split", "保存先を選んで分割")}
                        </Button>
                    </section>
                </div>
            </div>
        </div>
    );
}

function PhrasePage({ locale, run, active }: Omit<ToolPageProps, "id">) {
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
                <aside className="input-card template-list">
                    <div className="toolbar-row">
                        <h2>{text(locale, "Templates", "定型文一覧")}</h2>
                        <Button variant="outline" onClick={() => requestAction({ type: "new" })}>
                            {text(locale, "New", "新規")} <kbd>⌘N</kbd>
                        </Button>
                    </div>
                    {templates.length === 0 && (
                        <p className="muted-text">
                            {text(locale, "No saved templates.", "保存された定型文はありません。")}
                        </p>
                    )}
                    {templates.map((item) => (
                        <Button
                            key={String(item.filename)}
                            variant="ghost"
                            className={`template-item ${filename === item.filename ? "active" : ""}`}
                            onClick={() => requestAction({ type: "select", item })}
                        >
                            <strong>{String(item.title)}</strong>
                            <span>{String(item.content).slice(0, 72)}</span>
                        </Button>
                    ))}
                </aside>
                <div className="input-card">
                    <Field label={text(locale, "Title", "タイトル")}>
                        <Input
                            className="form-control"
                            value={title}
                            onChange={(event) => setTitle(event.target.value)}
                        />
                    </Field>
                    <Area
                        label={text(locale, "Template text", "テンプレート本文")}
                        value={content}
                        onChange={setContent}
                        rows={16}
                    />
                    <div className="button-row">
                        <Button onClick={() => runUiEffect(save())} disabled={busy}>
                            {text(locale, "Save template", "定型文を保存")} <kbd>⌘S</kbd>
                        </Button>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(copy())}
                            disabled={!content}
                        >
                            {text(locale, "Copy text", "本文をコピー")} <kbd>⌘⇧C</kbd>
                        </Button>
                        <Button
                            variant="outline"
                            className="danger-button"
                            onClick={requestDelete}
                            disabled={busy || !filename}
                        >
                            {text(locale, "Delete", "削除")}
                        </Button>
                    </div>
                    <OutputMessage
                        locale={locale}
                        error={error}
                        message={message}
                        onDismiss={() => setError("")}
                    />
                </div>
            </div>
            <AlertDialog.Root
                open={unsavedDialogOpen}
                onOpenChange={(open) => {
                    setUnsavedDialogOpen(open);
                    if (!open) setPendingAction(null);
                }}
            >
                <AlertDialog.Portal>
                    <AlertDialog.Backdrop className="dialog-backdrop" />
                    <AlertDialog.Viewport className="dialog-viewport">
                        <AlertDialog.Popup className="confirm-dialog">
                            <AlertDialog.Title className="dialog-title">
                                {text(locale, "Unsaved Changes", "未保存の変更")}
                            </AlertDialog.Title>
                            <AlertDialog.Description className="dialog-description">
                                {text(
                                    locale,
                                    "Save your changes before continuing?",
                                    "変更を保存して続行しますか？",
                                )}
                            </AlertDialog.Description>
                            <div className="dialog-footer">
                                <AlertDialog.Close render={<Button variant="outline" />}>
                                    {text(locale, "Cancel", "キャンセル")}
                                </AlertDialog.Close>
                                <Button variant="outline" onClick={continueWithoutSaving}>
                                    {text(locale, "Discard", "破棄")}
                                </Button>
                                <Button onClick={saveAndContinue} disabled={busy}>
                                    {text(locale, "Save", "保存")}
                                </Button>
                            </div>
                        </AlertDialog.Popup>
                    </AlertDialog.Viewport>
                </AlertDialog.Portal>
            </AlertDialog.Root>
            <AlertDialog.Root open={deleteDialogOpen} onOpenChange={setDeleteDialogOpen}>
                <AlertDialog.Portal>
                    <AlertDialog.Backdrop className="dialog-backdrop" />
                    <AlertDialog.Viewport className="dialog-viewport">
                        <AlertDialog.Popup className="confirm-dialog">
                            <AlertDialog.Title className="dialog-title">
                                {text(locale, "Confirm Delete", "削除の確認")}
                            </AlertDialog.Title>
                            <AlertDialog.Description className="dialog-description">
                                {text(
                                    locale,
                                    `Are you sure you want to delete "${title}"?`,
                                    `「${title}」を削除しますか？`,
                                )}
                            </AlertDialog.Description>
                            <div className="dialog-footer">
                                <AlertDialog.Close render={<Button variant="outline" />}>
                                    {text(locale, "Cancel", "キャンセル")}
                                </AlertDialog.Close>
                                <Button className="danger-button" onClick={confirmDelete}>
                                    {text(locale, "Delete", "削除")}
                                </Button>
                            </div>
                        </AlertDialog.Popup>
                    </AlertDialog.Viewport>
                </AlertDialog.Portal>
            </AlertDialog.Root>
        </div>
    );
}

function HttpPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [method, setMethod] = useState("GET");
    const [url, setUrl] = useState("");
    const [headers, setHeaders] = useState("");
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [body, setBody] = useState("");
    const [parameters, setParameters] = useState<
        Array<{ key: string; value: string; description: string }>
    >(Array.from({ length: 10 }, () => ({ key: "", value: "", description: "" })));
    const [response, setResponse] = useState<Record<string, unknown> | null>(null);
    const [httpTab, setHttpTab] = useState("parameters");
    const send = () =>
        Effect.gen(function* () {
            const parsedHeaders: Record<string, string> = {};
            for (const line of headers.split(/\r?\n/)) {
                if (!line.trim()) continue;
                const separator = line.indexOf(":");
                if (separator <= 0) {
                    yield* Effect.sync(() =>
                        setError(
                            text(
                                locale,
                                "Each header must use the name: value format.",
                                "ヘッダーは name: value の形式で入力してください。",
                            ),
                        ),
                    );
                    return;
                }
                parsedHeaders[line.slice(0, separator).trim()] = line.slice(separator + 1).trim();
            }
            const result = yield* execute("sendHttpRequest", {
                method,
                url,
                headers: parsedHeaders,
                username,
                password,
                body,
                parameters: parameters.filter((parameter) => parameter.key.trim()),
            });
            if (result) yield* Effect.sync(() => setResponse(result));
        });
    return (
        <div className="tool-page">
            <Heading title={text(locale, "HTTP Request", "HTTPリクエスト")} />
            <section className="input-card">
                <div className="http-request-row">
                    <Field label={text(locale, "Method", "メソッド")}>
                        <SelectBox
                            value={method}
                            onValueChange={setMethod}
                            options={[
                                { value: "GET", label: "GET" },
                                { value: "POST", label: "POST" },
                                { value: "PUT", label: "PUT" },
                                { value: "DELETE", label: "DELETE" },
                            ]}
                        />
                    </Field>
                    <Field label="URL">
                        <Input
                            value={url}
                            onChange={(event) => setUrl(event.target.value)}
                            placeholder="https://api.example.com/v1/items"
                        />
                    </Field>
                    <Button onClick={() => runUiEffect(send())} disabled={busy}>
                        {busy ? text(locale, "Sending…", "送信中…") : text(locale, "Send", "送信")}
                    </Button>
                </div>
            </section>
            <div className="http-workspace">
                <Tabs.Root
                    className="input-card"
                    value={httpTab}
                    onValueChange={(value) => setHttpTab(String(value))}
                >
                    <Tabs.List className="feature-tabs-list">
                        <Tabs.Tab className="feature-tab" value="parameters">
                            {text(locale, "Parameters", "パラメーター")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="authentication">
                            {text(locale, "Authentication", "認証")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="headers">
                            {text(locale, "Headers", "ヘッダー")}
                        </Tabs.Tab>
                        <Tabs.Tab className="feature-tab" value="body">
                            {text(locale, "Body", "本文")}
                        </Tabs.Tab>
                    </Tabs.List>
                    <Tabs.Panel className="http-tab-panel" value="parameters">
                        <div className="http-parameter-grid">
                            <strong>{text(locale, "Key", "キー")}</strong>
                            <strong>{text(locale, "Value", "値")}</strong>
                            <strong>{text(locale, "Description", "説明")}</strong>
                            {parameters.map((parameter, index) => (
                                <div className="http-parameter-row" key={index}>
                                    <Input
                                        aria-label={`${text(locale, "Parameter key", "パラメーターキー")} ${index + 1}`}
                                        value={parameter.key}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? { ...item, key: event.target.value }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                    <Input
                                        aria-label={`${text(locale, "Parameter value", "パラメーター値")} ${index + 1}`}
                                        value={parameter.value}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? { ...item, value: event.target.value }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                    <Input
                                        aria-label={`${text(locale, "Parameter description", "パラメーター説明")} ${index + 1}`}
                                        value={parameter.description}
                                        onChange={(event) =>
                                            setParameters((current) =>
                                                current.map((item, itemIndex) =>
                                                    itemIndex === index
                                                        ? {
                                                              ...item,
                                                              description: event.target.value,
                                                          }
                                                        : item,
                                                ),
                                            )
                                        }
                                    />
                                </div>
                            ))}
                        </div>
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="authentication">
                        <div className="auth-card">
                            <Field label={text(locale, "Username", "ユーザー名")}>
                                <Input
                                    autoComplete="username"
                                    value={username}
                                    onChange={(event) => setUsername(event.target.value)}
                                />
                            </Field>
                            <Field label={text(locale, "Password", "パスワード")}>
                                <Input
                                    type="password"
                                    autoComplete="current-password"
                                    value={password}
                                    onChange={(event) => setPassword(event.target.value)}
                                />
                            </Field>
                        </div>
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="headers">
                        <Area
                            label={text(
                                locale,
                                "Headers (one name: value per line)",
                                "ヘッダー (1行に1つ name: value)",
                            )}
                            value={headers}
                            onChange={setHeaders}
                            rows={12}
                            placeholder="Accept: application/json"
                        />
                    </Tabs.Panel>
                    <Tabs.Panel className="http-tab-panel" value="body">
                        <Area
                            label={text(locale, "Request body", "リクエスト本文")}
                            value={body}
                            onChange={setBody}
                            rows={12}
                        />
                    </Tabs.Panel>
                </Tabs.Root>
                <section className="input-card http-response-panel">
                    <div className="toolbar-row">
                        <h2>{text(locale, "Response", "レスポンス")}</h2>
                        {response && (
                            <span className="muted-text">
                                {String(response.status ?? "")} {String(response.reason ?? "")} ·{" "}
                                {String(response.elapsedMs ?? "")} ms ·{" "}
                                {String(response.bytes ?? "")} B
                            </span>
                        )}
                    </div>
                    {response ? (
                        <Area
                            label={text(locale, "Response body", "レスポンス本文")}
                            value={String(response.body ?? "")}
                            readOnly
                            rows={16}
                        />
                    ) : (
                        <div className="empty-state">
                            {text(locale, "No response yet.", "レスポンスはありません。")}
                        </div>
                    )}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
            {response && (
                <section className="input-card response-card">
                    <div className="toolbar-row">
                        <h2>{text(locale, "Response", "レスポンス")}</h2>
                        <span className="muted-text">
                            {String(response.status ?? "")} {String(response.reason ?? "")} ·{" "}
                            {String(response.elapsedMs ?? "")} ms · {String(response.bytes ?? "")} B
                        </span>
                    </div>
                    <Area
                        label={text(locale, "Response body", "レスポンス本文")}
                        value={String(response.body ?? "")}
                        readOnly
                        rows={16}
                    />
                </section>
            )}
        </div>
    );
}

function DataPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [input, setInput] = useState("");
    const [output, setOutput] = useState("");
    const [format, setFormat] = useState("json");
    const [indentation, setIndentation] = useState("2");
    const convert = () =>
        Effect.gen(function* () {
            const result = yield* execute("convertData", { input, format, indentation });
            if (result) yield* Effect.sync(() => setOutput(String(result.output ?? "")));
        });
    const open = () =>
        Effect.gen(function* () {
            const result = yield* execute("openTextFile", { fileType: "data" });
            if (result) yield* Effect.sync(() => setInput(String(result.text ?? "")));
        });
    const save = () =>
        execute("saveTextFile", {
            text: output,
            suggestedName: "converted." + (format.startsWith("yaml") ? "yaml" : format),
            fileType: "data",
        });
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Data Conversion", "データ変換")} />
            <div className="toolbar-row">
                <Button variant="outline" onClick={() => runUiEffect(open())} disabled={busy}>
                    {text(locale, "Open file", "ファイルを開く")}
                </Button>
                <Field label={text(locale, "Output format", "出力形式")}>
                    <SelectBox
                        className="compact"
                        value={format}
                        onValueChange={setFormat}
                        options={[
                            { value: "json", label: "JSON" },
                            { value: "yaml-block", label: "YAML (block)" },
                            { value: "yaml-flow", label: "YAML (flow)" },
                            { value: "toml", label: "TOML" },
                        ]}
                    />
                </Field>
                <Field label={text(locale, "Indentation", "インデント")}>
                    <SelectBox
                        className="compact"
                        value={indentation}
                        onValueChange={setIndentation}
                        options={[
                            { value: "2", label: "2" },
                            { value: "4", label: "4" },
                            { value: "tab", label: "Tab" },
                            { value: "minified", label: "Minified" },
                        ]}
                    />
                </Field>
                <Button onClick={() => runUiEffect(convert())} disabled={busy}>
                    {text(locale, "Convert", "変換")}
                </Button>
            </div>
            <div className="two-column">
                <Area
                    label={text(locale, "Input", "入力")}
                    value={input}
                    onChange={setInput}
                    placeholder={'{"name":"DevTools"}'}
                />
                <Area
                    label={text(locale, "Output", "出力")}
                    value={output}
                    onChange={setOutput}
                    readOnly={false}
                />
            </div>
            <div className="action-row">
                <Button
                    variant="outline"
                    onClick={() => runUiEffect(save())}
                    disabled={!output || busy}
                >
                    {text(locale, "Save output", "出力を保存")}
                </Button>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

type TimestampValues = {
    seconds: string;
    milliseconds: string;
    local: string;
    utc: string;
    isoUtc: string;
    isoLocal: string;
};

const padDatePart = (value: number) => String(value).padStart(2, "0");

function toLocalInput(date: Date, utc = false) {
    const year = utc ? date.getUTCFullYear() : date.getFullYear();
    const month = padDatePart((utc ? date.getUTCMonth() : date.getMonth()) + 1);
    const day = padDatePart(utc ? date.getUTCDate() : date.getDate());
    const hour = padDatePart(utc ? date.getUTCHours() : date.getHours());
    const minute = padDatePart(utc ? date.getUTCMinutes() : date.getMinutes());
    return `${year}-${month}-${day}T${hour}:${minute}`;
}

function toLocalIso(date: Date) {
    const offset = -date.getTimezoneOffset();
    const sign = offset >= 0 ? "+" : "-";
    const absoluteOffset = Math.abs(offset);
    const seconds = padDatePart(date.getSeconds());
    const milliseconds = String(date.getMilliseconds()).padStart(3, "0");
    return `${toLocalInput(date)}:${seconds}.${milliseconds}${sign}${padDatePart(Math.floor(absoluteOffset / 60))}:${padDatePart(absoluteOffset % 60)}`;
}

function timestampValues(date: Date): TimestampValues {
    return {
        seconds: String(Math.floor(date.getTime() / 1000)),
        milliseconds: String(date.getTime()),
        local: toLocalInput(date),
        utc: toLocalInput(date, true),
        isoUtc: date.toISOString(),
        isoLocal: toLocalIso(date),
    };
}

function TimestampPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [values, setValues] = useState<TimestampValues>(() => timestampValues(new Date()));
    const convert = (key: keyof TimestampValues) =>
        Effect.gen(function* () {
            const source =
                key === "seconds" ? "seconds" : key === "milliseconds" ? "milliseconds" : "iso";
            const localDate = new Date(values.local);
            const value =
                key === "local"
                    ? Number.isNaN(localDate.getTime())
                        ? values.local
                        : localDate.toISOString()
                    : key === "utc"
                      ? `${values.utc}:00Z`
                      : key === "isoUtc"
                        ? values.isoUtc
                        : key === "isoLocal"
                          ? values.isoLocal
                          : values[key];
            const result = yield* execute("convertTimestamp", { value, source });
            if (!result) return;
            const instant = new Date(String(result.utc));
            yield* Effect.sync(() =>
                setValues({
                    seconds: String(result.seconds ?? ""),
                    milliseconds: String(result.milliseconds ?? ""),
                    local: toLocalInput(instant),
                    utc: toLocalInput(instant, true),
                    isoUtc: String(result.utc ?? ""),
                    isoLocal: String(result.local ?? ""),
                }),
            );
        });
    const copy = (value: string) => run("copyToClipboard", { text: value });
    const row = (
        key: keyof TimestampValues,
        label: string,
        type: "text" | "datetime-local" = "text",
    ) => (
        <div className="timestamp-row" key={key}>
            <Field label={label}>
                <Input
                    type={type}
                    value={values[key]}
                    onChange={(event) =>
                        setValues((current) => ({ ...current, [key]: event.target.value }))
                    }
                />
            </Field>
            <Button
                variant="outline"
                size="sm"
                disabled={busy}
                onClick={() => runUiEffect(convert(key))}
            >
                {text(locale, "Convert", "変換")}
            </Button>
            <Button variant="ghost" size="sm" onClick={() => runUiEffect(copy(values[key]))}>
                {text(locale, "Copy", "コピー")}
            </Button>
        </div>
    );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Timestamp Conversion", "タイムスタンプ変換")} />
            <div className="timestamp-toolbar">
                <Button variant="outline" onClick={() => setValues(timestampValues(new Date()))}>
                    {text(locale, "Now", "現在時刻")}
                </Button>
            </div>
            <div className="timestamp-groups">
                <section className="input-card timestamp-group">
                    <h2>{text(locale, "Unix Timestamp", "Unixタイムスタンプ")}</h2>
                    {row("seconds", text(locale, "Seconds", "秒"))}
                    {row("milliseconds", text(locale, "Milliseconds", "ミリ秒"))}
                </section>
                <section className="input-card timestamp-group">
                    <h2>{text(locale, "Date & Time", "日時")}</h2>
                    {row("local", text(locale, "Local", "ローカル"), "datetime-local")}
                    {row("utc", text(locale, "UTC", "UTC"), "datetime-local")}
                </section>
                <section className="input-card timestamp-group">
                    <h2>ISO 8601</h2>
                    {row("isoUtc", text(locale, "UTC", "UTC"))}
                    {row("isoLocal", text(locale, "Local", "ローカル"))}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

function DiffPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { error, setError } = useOperation(run);
    const [left, setLeft] = useState("");
    const [right, setRight] = useState("");
    const [rows, setRows] = useState<Array<Record<string, unknown>>>([]);
    const [mode, setMode] = useState("inline");
    useEffect(() => {
        const timer = window.setTimeout(() => {
            runUiEffect(
                run("compareText", { left, right }).pipe(
                    Effect.tap((result) =>
                        Effect.sync(() => {
                            setRows((result.rows as Array<Record<string, unknown>>) ?? []);
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
        }, 150);
        return () => window.clearTimeout(timer);
    }, [left, right, run, setError]);
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Text Diff", "テキスト差分")} />
            <div className="diff-layout">
                <div className="diff-editors">
                    <Area
                        label={text(locale, "Original", "変更前")}
                        value={left}
                        onChange={setLeft}
                    />
                    <Area
                        label={text(locale, "Modified", "変更後")}
                        value={right}
                        onChange={setRight}
                    />
                </div>
                <section className="input-card">
                    <div className="diff-result-heading">
                        <h2>{text(locale, "Diff", "差分")}</h2>
                        <SelectBox
                            className="compact"
                            value={mode}
                            onValueChange={setMode}
                            options={[
                                { value: "inline", label: text(locale, "Inline", "インライン") },
                                {
                                    value: "side-by-side",
                                    label: text(locale, "Side-by-side", "左右に並べる"),
                                },
                            ]}
                        />
                    </div>
                    {mode === "inline" ? (
                        <div className="diff-table diff-inline">
                            {rows.map((row, index) => {
                                const leftText = String(row.left ?? "");
                                const rightText = String(row.right ?? "");
                                if (row.leftChanged && row.rightChanged) {
                                    return (
                                        <div className="diff-row" key={index}>
                                            <code className="removed">− {leftText}</code>
                                            <code className="added">+ {rightText}</code>
                                        </div>
                                    );
                                }
                                const changed = Boolean(row.leftChanged || row.rightChanged);
                                return (
                                    <div className="diff-row" key={index}>
                                        <code className={changed ? "added" : ""}>
                                            {changed ? "+ " : "  "}
                                            {rightText || leftText}
                                        </code>
                                    </div>
                                );
                            })}
                        </div>
                    ) : (
                        <div
                            className="diff-table"
                            role="table"
                            aria-label={text(locale, "Diff result", "差分結果")}
                        >
                            {rows.map((row, index) => (
                                <div key={index} className="diff-row">
                                    <code className={row.leftChanged ? "removed" : ""}>
                                        {String(row.left ?? "")}
                                    </code>
                                    <code className={row.rightChanged ? "added" : ""}>
                                        {String(row.right ?? "")}
                                    </code>
                                </div>
                            ))}
                        </div>
                    )}
                </section>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

function MarkdownPage({ locale, run }: Omit<ToolPageProps, "id">) {
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
                            setPreview(String(result.html ?? ""));
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
            if (result) yield* Effect.sync(() => setMarkdown(String(result.text ?? "")));
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
    const attachPreviewScroll = useCallback(() => {
        previewScrollCleanup.current?.();
        previewScrollCleanup.current = null;
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
    }, [syncScroll]);
    useEffect(() => {
        attachPreviewScroll();
        return () => previewScrollCleanup.current?.();
    }, [attachPreviewScroll]);
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
                <div className="preview-panel">
                    <span className="field-label">{text(locale, "Preview", "プレビュー")}</span>
                    <iframe
                        ref={previewRef}
                        title={text(locale, "Markdown preview", "Markdownプレビュー")}
                        sandbox="allow-same-origin"
                        referrerPolicy="no-referrer"
                        srcDoc={previewDocument}
                        onLoad={attachPreviewScroll}
                    />
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

function CommandPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [category, setCategory] = useState("git");
    const [functions, setFunctions] = useState<Array<Record<string, unknown>>>([]);
    const [functionIndex, setFunctionIndex] = useState(0);
    const [optionIndex, setOptionIndex] = useState(0);
    const [value, setValue] = useState("");
    const [command, setCommand] = useState("");
    useEffect(() => {
        const loadFunctions = run("commandData", { action: "list", category }).pipe(
            Effect.tap((result) =>
                Effect.sync(() =>
                    setFunctions((result.functions as Array<Record<string, unknown>>) ?? []),
                ),
            ),
            Effect.catch(() => Effect.sync(() => setFunctions([]))),
        );
        return forkUiEffect(loadFunctions);
    }, [category, run]);
    const options = (functions[functionIndex]?.options as Array<Record<string, unknown>>) ?? [];
    const generate = () =>
        execute("commandData", {
            action: "generate",
            category,
            functionIndex,
            optionIndex,
            value,
        }).pipe(
            Effect.tap((result) => Effect.sync(() => setCommand(String(result?.command ?? "")))),
        );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Command Generation", "コマンド生成")} />
            <div className="input-card">
                <div className="form-grid">
                    <Field label={text(locale, "Category", "カテゴリ")}>
                        <SelectBox
                            value={category}
                            onValueChange={(nextCategory) => {
                                setCategory(nextCategory);
                                setFunctionIndex(0);
                                setOptionIndex(0);
                            }}
                            options={[
                                { value: "git", label: "Git" },
                                { value: "docker", label: "Docker" },
                                { value: "compose", label: "Docker Compose" },
                            ]}
                        />
                    </Field>
                    <Field label={text(locale, "Command", "コマンド")}>
                        <SelectBox
                            value={String(functionIndex)}
                            onValueChange={(nextIndex) => {
                                setFunctionIndex(Number(nextIndex));
                                setOptionIndex(0);
                            }}
                            options={functions.map((item, index) => ({
                                value: String(index),
                                label: String(item.name),
                            }))}
                        />
                    </Field>
                    <Field label={text(locale, "Option", "オプション")}>
                        <SelectBox
                            value={String(optionIndex)}
                            onValueChange={(nextIndex) => setOptionIndex(Number(nextIndex))}
                            options={options.map((item, index) => ({
                                value: String(index),
                                label: `${String(item.name)}${item.title ? ` — ${String(item.title)}` : ""}`,
                            }))}
                        />
                    </Field>
                    {Boolean(options[optionIndex]?.required || options[optionIndex]?.quotes) && (
                        <Field
                            label={String(
                                options[optionIndex]?.title || text(locale, "Value", "値"),
                            )}
                        >
                            <Input
                                className="form-control"
                                value={value}
                                onChange={(event) => setValue(event.target.value)}
                            />
                        </Field>
                    )}
                </div>
                <div className="action-row">
                    <Button
                        variant="outline"
                        onClick={() => {
                            setCategory("git");
                            setFunctionIndex(0);
                            setOptionIndex(0);
                            setValue("");
                        }}
                        disabled={busy}
                    >
                        {text(locale, "Reset", "リセット")}
                    </Button>
                    <Button
                        onClick={() => runUiEffect(generate())}
                        disabled={busy || !functions.length}
                    >
                        {text(locale, "Generate command", "コマンドを作成")}
                    </Button>
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
            <section className="input-card command-output">
                <div className="toolbar-row">
                    <h2>{text(locale, "Generated Result", "生成結果")}</h2>
                    <Button
                        variant="outline"
                        disabled={!command}
                        onClick={() => runUiEffect(run("copyToClipboard", { text: command }))}
                    >
                        {text(locale, "Copy", "コピー")}
                    </Button>
                    <Button variant="ghost" onClick={() => setCommand("")} disabled={!command}>
                        {text(locale, "Clear", "クリア")}
                    </Button>
                </div>
                <Textarea value={command} readOnly rows={5} />
            </section>
        </div>
    );
}

const qrFields: Record<string, Array<[string, string, boolean?]>> = {
    text: [["text_content", "Text", true]],
    url: [["url_content", "URL", true]],
    email: [["email_address", "Email", true]],
    phone: [["phone_number", "Phone number", true]],
    sms: [
        ["sms_phone", "Phone number", true],
        ["sms_message", "Message", true],
    ],
    wifi: [
        ["wifi_ssid", "SSID", true],
        ["wifi_password", "Password"],
    ],
    contact: [
        ["contact_name", "Name", true],
        ["contact_phone", "Phone"],
        ["contact_email", "Email"],
    ],
    calendar: [
        ["cal_summary", "Event title", true],
        ["cal_start", "Start (ISO-8601)", true],
        ["cal_end", "End (ISO-8601)", true],
    ],
    geo: [
        ["geo_lat", "Latitude", true],
        ["geo_lng", "Longitude", true],
    ],
};

function QrPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [type, setType] = useState("text");
    const [parameters, setParameters] = useState<Record<string, string>>({ text_content: "" });
    const [content, setContent] = useState("");
    const [image, setImage] = useState("");
    const [message, setMessage] = useState("");
    const fields = qrFields[type] ?? [];
    const generate = () =>
        Effect.gen(function* () {
            yield* Effect.sync(() => setMessage(""));
            const result = yield* execute("generateQr", { type, parameters });
            if (result) {
                yield* Effect.sync(() => {
                    setContent(String(result.content ?? ""));
                    setImage(String(result.imageDataUrl ?? ""));
                });
            }
        });
    const clear = () => {
        setParameters(
            type === "text"
                ? { text_content: "" }
                : type === "wifi"
                  ? { wifi_security: "WPA", wifi_hidden: "false" }
                  : {},
        );
        setContent("");
        setImage("");
        setMessage("");
    };
    const copy = () =>
        execute("copyToClipboard", { imageDataUrl: image }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (result)
                        setMessage(text(locale, "QR image copied.", "QR画像をコピーしました。"));
                }),
            ),
        );
    const save = () =>
        execute("saveQrImage", { type, parameters }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (result)
                        setMessage(
                            text(
                                locale,
                                `Saved ${String(result.fileName ?? "QR image")}.`,
                                `${String(result.fileName ?? "QR画像")}を保存しました。`,
                            ),
                        );
                }),
            ),
        );
    return (
        <div className="tool-page">
            <Heading title={text(locale, "QR Code Generation", "QRコード生成")} />
            <div className="qr-layout">
                <div className="input-card">
                    <Field label={text(locale, "QR type", "QRの種類")}>
                        <SelectBox
                            value={type}
                            onValueChange={(value) => {
                                setType(value);
                                setParameters(
                                    value === "text"
                                        ? { text_content: "" }
                                        : value === "wifi"
                                          ? { wifi_security: "WPA", wifi_hidden: "false" }
                                          : {},
                                );
                                setContent("");
                                setImage("");
                                setMessage("");
                            }}
                            options={[
                                { value: "text", label: "Text" },
                                { value: "url", label: "URL" },
                                { value: "email", label: "Email" },
                                { value: "phone", label: "Phone" },
                                { value: "sms", label: "SMS" },
                                { value: "wifi", label: "Wi-Fi" },
                                { value: "contact", label: "Contact" },
                                { value: "calendar", label: "Calendar" },
                                { value: "geo", label: "Location" },
                            ]}
                        />
                    </Field>
                    {fields.map(([key, label, required]) => (
                        <Field key={key} label={`${label}${required ? " *" : ""}`}>
                            <Input
                                className="form-control"
                                type={
                                    key === "wifi_password"
                                        ? "password"
                                        : key.startsWith("cal_")
                                          ? "datetime-local"
                                          : "text"
                                }
                                value={parameters[key] ?? ""}
                                onChange={(event) =>
                                    setParameters((current) => ({
                                        ...current,
                                        [key]: event.target.value,
                                    }))
                                }
                            />
                        </Field>
                    ))}
                    {type === "wifi" && (
                        <>
                            <Field label={text(locale, "Security", "セキュリティ")}>
                                <SelectBox
                                    value={parameters.wifi_security ?? "WPA"}
                                    onValueChange={(wifiSecurity) =>
                                        setParameters((current) => ({
                                            ...current,
                                            wifi_security: wifiSecurity,
                                        }))
                                    }
                                    options={[
                                        { value: "WPA", label: "WPA" },
                                        { value: "WEP", label: "WEP" },
                                        { value: "nopass", label: text(locale, "None", "なし") },
                                    ]}
                                />
                            </Field>
                            <label className="inline-check">
                                <Checkbox
                                    checked={parameters.wifi_hidden === "true"}
                                    onCheckedChange={(checked) =>
                                        setParameters((current) => ({
                                            ...current,
                                            wifi_hidden: String(checked),
                                        }))
                                    }
                                />
                                {text(locale, "Hidden network", "非公開ネットワーク")}
                            </label>
                        </>
                    )}
                    <div className="button-row">
                        <Button onClick={() => runUiEffect(generate())} disabled={busy}>
                            {text(locale, "Generate", "生成")}
                        </Button>
                        <Button variant="outline" onClick={clear} disabled={busy}>
                            {text(locale, "Clear", "クリア")}
                        </Button>
                    </div>
                </div>
                <div className="qr-preview input-card">
                    {image ? (
                        <img
                            src={image}
                            alt={text(locale, "Generated QR code", "生成したQRコード")}
                        />
                    ) : (
                        <span>{text(locale, "QR code preview", "QRコードのプレビュー")}</span>
                    )}
                    {content && (
                        <>
                            <code>{content}</code>
                            <div className="button-row">
                                <Button
                                    variant="outline"
                                    onClick={() => runUiEffect(copy())}
                                    disabled={busy}
                                >
                                    {text(locale, "Copy image", "画像をコピー")}
                                </Button>
                                <Button
                                    variant="outline"
                                    onClick={() => runUiEffect(save())}
                                    disabled={busy}
                                >
                                    {text(locale, "Save image", "画像を保存")}
                                </Button>
                            </div>
                        </>
                    )}
                </div>
            </div>
            <OutputMessage
                locale={locale}
                error={error}
                message={message}
                onDismiss={() => setError("")}
            />
        </div>
    );
}

function DatabasePage({
    locale,
    run,
    active,
    databaseNavigationVersion,
}: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [type, setType] = useState("QSQLITE");
    const [host, setHost] = useState("localhost");
    const [database, setDatabase] = useState("");
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [history, setHistory] = useState<Array<Record<string, unknown>>>([]);
    const [selectedFileName, setSelectedFileName] = useState("");
    const [connected, setConnected] = useState(false);
    const [displayName, setDisplayName] = useState("");
    const [tables, setTables] = useState<string[]>([]);
    const [connectionDialogOpen, setConnectionDialogOpen] = useState(false);
    const [queryTabs, setQueryTabs] = useState([
        {
            id: "query-1",
            title: "SQL",
            sql: "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;",
            columns: [] as string[],
            rows: [] as Array<Array<unknown>>,
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
                    setHistory((result.history as Array<Record<string, unknown>>) ?? []);
                    setSelectedFileName(String(result.selectedFileName ?? ""));
                    if (result.connected) setConnected(true);
                }),
            ),
        );
    useEffect(() => {
        if (!active) return;
        let mounted = true;
        const initialize = Effect.gen(function* () {
            const result = yield* run("databaseHistory", {});
            if (!mounted) return;
            const selected = String(result.selectedFileName ?? "");
            yield* Effect.sync(() => {
                setHistory((result.history as Array<Record<string, unknown>>) ?? []);
                setSelectedFileName(selected);
                if (result.connected) setConnected(true);
            });
            if (selected && !result.connected) {
                const connectedResult = yield* run("databaseConnect", { type: "QSQLITE" });
                if (mounted && connectedResult.connected) {
                    yield* Effect.sync(() => {
                        setConnected(true);
                        setDisplayName(
                            String(connectedResult.displayName ?? `SQLite: ${selected}`),
                        );
                        setTables((connectedResult.tables as string[]) ?? []);
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
                    setSelectedFileName(String(result.fileName ?? ""));
                    setType("QSQLITE");
                }),
            ),
        );
    const connect = (historyIndex?: number) =>
        Effect.gen(function* () {
            const payload =
                historyIndex === undefined
                    ? { type, host, database, username, password }
                    : { historyIndex, password };
            const result = yield* execute("databaseConnect", payload);
            if (!result) return;
            yield* Effect.sync(() => {
                setConnected(true);
                setDisplayName(
                    String(
                        result.displayName ??
                            (type === "QSQLITE"
                                ? `SQLite: ${selectedFileName}`
                                : `${database}@${host}`),
                    ),
                );
                setTables((result.tables as string[]) ?? []);
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
                        columns: (result.columns as string[]) ?? [],
                        rows: (result.rows as Array<Array<unknown>>) ?? [],
                        truncated: Boolean(result.truncated),
                    }));
                }),
            ),
        );
    const loadTables = () =>
        execute("databaseTables", {}).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (result) setTables((result.tables as string[]) ?? []);
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
                <aside className="input-card db-sidebar">
                    <h2>{text(locale, "Tables", "テーブル")}</h2>
                    <div className="db-table-list">
                        {tables.map((table) => (
                            <Button
                                key={table}
                                variant="ghost"
                                size="sm"
                                className="db-table-item"
                                onClick={() =>
                                    updateActiveQuery((current) => ({
                                        ...current,
                                        sql: `SELECT * FROM "${table.replaceAll('"', '""')}" LIMIT 1000;`,
                                    }))
                                }
                            >
                                <Icon>table_chart</Icon>
                                {table}
                            </Button>
                        ))}
                        {tables.length === 0 && (
                            <span className="muted-text">
                                {connected
                                    ? text(locale, "No tables found.", "テーブルがありません。")
                                    : text(
                                          locale,
                                          "Connect to a database to view tables.",
                                          "接続するとテーブルを表示します。",
                                      )}
                            </span>
                        )}
                    </div>
                </aside>
                <div className="db-workspace">
                    <Tabs.Root
                        className="db-query-tabs input-card"
                        value={activeQueryId}
                        onValueChange={(value) => setActiveQueryId(String(value))}
                    >
                        <Tabs.List className="feature-tabs-list">
                            {queryTabs.map((queryTab) => (
                                <Tabs.Tab
                                    key={queryTab.id}
                                    className="feature-tab db-query-tab"
                                    value={queryTab.id}
                                >
                                    <span>{queryTab.title}</span>
                                    {queryTabs.length > 1 && (
                                        <span
                                            className="db-query-tab-close"
                                            role="button"
                                            tabIndex={0}
                                            aria-label={text(
                                                locale,
                                                "Close query",
                                                "クエリを閉じる",
                                            )}
                                            onClick={(event) => {
                                                event.stopPropagation();
                                                closeQueryTab(queryTab.id);
                                            }}
                                            onKeyDown={(event) => {
                                                if (event.key === "Enter" || event.key === " ") {
                                                    event.preventDefault();
                                                    event.stopPropagation();
                                                    closeQueryTab(queryTab.id);
                                                }
                                            }}
                                        >
                                            ×
                                        </span>
                                    )}
                                </Tabs.Tab>
                            ))}
                        </Tabs.List>
                        {queryTabs.map((queryTab) => (
                            <Tabs.Panel
                                key={queryTab.id}
                                className="db-query-panel"
                                value={queryTab.id}
                            >
                                <Area
                                    label="SQL"
                                    value={queryTab.sql}
                                    onChange={(sql) =>
                                        setQueryTabs((current) =>
                                            current.map((item) =>
                                                item.id === queryTab.id ? { ...item, sql } : item,
                                            ),
                                        )
                                    }
                                    rows={9}
                                />
                                <div className="action-row">
                                    <Button
                                        onClick={() => runUiEffect(query())}
                                        disabled={!connected || busy}
                                    >
                                        {text(locale, "Execute", "実行")}
                                    </Button>
                                </div>
                                {queryTab.columns.length > 0 && (
                                    <div className="table-wrap">
                                        <table>
                                            <thead>
                                                <tr>
                                                    {queryTab.columns.map((column) => (
                                                        <th key={column}>{column}</th>
                                                    ))}
                                                </tr>
                                            </thead>
                                            <tbody>
                                                {queryTab.rows.map((row, rowIndex) => (
                                                    <tr key={rowIndex}>
                                                        {row.map((value, columnIndex) => (
                                                            <td key={columnIndex}>
                                                                {value === null ? (
                                                                    <em>NULL</em>
                                                                ) : (
                                                                    String(value)
                                                                )}
                                                            </td>
                                                        ))}
                                                    </tr>
                                                ))}
                                            </tbody>
                                        </table>
                                        {queryTab.truncated && (
                                            <p className="muted-text">
                                                {text(
                                                    locale,
                                                    "Showing the first 1,000 rows.",
                                                    "先頭の1,000行を表示しています。",
                                                )}
                                            </p>
                                        )}
                                    </div>
                                )}
                            </Tabs.Panel>
                        ))}
                    </Tabs.Root>
                </div>
            </div>
            <Dialog.Root open={connectionDialogOpen} onOpenChange={setConnectionDialogOpen}>
                <Dialog.Portal>
                    <Dialog.Backdrop className="dialog-backdrop" />
                    <Dialog.Viewport className="dialog-viewport">
                        <Dialog.Popup className="db-connection-dialog">
                            <Dialog.Title className="dialog-title">
                                {text(
                                    locale,
                                    "Database Connection Settings",
                                    "データベース接続設定",
                                )}
                            </Dialog.Title>
                            <div className="db-connection-content">
                                {history.length > 0 && (
                                    <section className="db-history-section">
                                        <h2>{text(locale, "Recent connections", "最近の接続")}</h2>
                                        {history.map((item) => (
                                            <div key={String(item.index)} className="history-item">
                                                <div>
                                                    <strong>{String(item.displayName)}</strong>
                                                    <span>
                                                        {String(item.host || item.database)}
                                                    </span>
                                                </div>
                                                <Button
                                                    variant="outline"
                                                    size="sm"
                                                    disabled={busy}
                                                    onClick={() =>
                                                        runUiEffect(connect(Number(item.index)))
                                                    }
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
                                    >
                                        <SelectBox
                                            value={type}
                                            onValueChange={setType}
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
                                                    text(
                                                        locale,
                                                        "No file selected",
                                                        "ファイル未選択",
                                                    )}
                                            </div>
                                            <Button
                                                variant="outline"
                                                onClick={() => runUiEffect(selectFile())}
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
                                                        setHost(event.target.value)
                                                    }
                                                />
                                            </Field>
                                            <Field label={text(locale, "Database", "データベース")}>
                                                <Input
                                                    value={database}
                                                    onChange={(event) =>
                                                        setDatabase(event.target.value)
                                                    }
                                                />
                                            </Field>
                                            <Field label={text(locale, "Username", "ユーザー名")}>
                                                <Input
                                                    value={username}
                                                    onChange={(event) =>
                                                        setUsername(event.target.value)
                                                    }
                                                />
                                            </Field>
                                            <Field label={text(locale, "Password", "パスワード")}>
                                                <Input
                                                    type="password"
                                                    value={password}
                                                    onChange={(event) =>
                                                        setPassword(event.target.value)
                                                    }
                                                />
                                            </Field>
                                        </>
                                    )}
                                    <div className="button-row">
                                        <Button
                                            onClick={() => runUiEffect(connect())}
                                            disabled={
                                                busy ||
                                                (type === "QSQLITE"
                                                    ? !selectedFileName
                                                    : !host || !database || !username)
                                            }
                                        >
                                            {text(locale, "Connect", "接続")}
                                        </Button>
                                        <Button
                                            variant="outline"
                                            onClick={() => runUiEffect(disconnect())}
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
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}

function RegexPage({ locale, run }: Omit<ToolPageProps, "id">) {
    const { busy, error, execute, setError } = useOperation(run);
    const [pattern, setPattern] = useState("");
    const [input, setInput] = useState("");
    const [replacement, setReplacement] = useState("");
    const [options, setOptions] = useState<RegexOptions>({
        global: true,
        caseInsensitive: false,
        multiline: false,
        dotMatchesEverything: false,
        useUnicodeProperties: false,
        extendedPatternSyntax: false,
    });
    const [matches, setMatches] = useState<Array<Record<string, unknown>>>([]);
    const [replacedText, setReplacedText] = useState("");
    const [quickReference, setQuickReference] = useState(true);
    const [settingsLoaded, setSettingsLoaded] = useState(false);
    const evaluate = () =>
        execute("evaluateRegex", {
            pattern,
            text: input,
            replacement,
            ...options,
        }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    if (!result) return;
                    setMatches((result.matches as Array<Record<string, unknown>>) ?? []);
                    setReplacedText(String(result.replacedText ?? ""));
                }),
            ),
        );
    useEffect(() => {
        const loadSettings = run("regexSettings", { action: "load" }).pipe(
            Effect.tap((result) =>
                Effect.sync(() => {
                    setPattern(String(result.pattern ?? ""));
                    setInput(String(result.testText ?? ""));
                    setReplacement(String(result.replacePattern ?? ""));
                    setOptions((current) => ({
                        ...current,
                        ...(result.options as Partial<RegexOptions>),
                    }));
                }),
            ),
            Effect.catch(() => Effect.void),
            Effect.ensuring(Effect.sync(() => setSettingsLoaded(true))),
        );
        return forkUiEffect(loadSettings);
    }, [run]);
    useEffect(() => {
        if (!settingsLoaded) return;
        const timer = window.setTimeout(() => {
            runUiEffect(
                run("regexSettings", {
                    action: "save",
                    pattern,
                    testText: input,
                    replacePattern: replacement,
                    options,
                }).pipe(Effect.catch(() => Effect.void)),
            );
        }, 400);
        return () => window.clearTimeout(timer);
    }, [pattern, input, replacement, options, settingsLoaded, run]);
    const setOption = (key: keyof RegexOptions) =>
        setOptions((current) => ({ ...current, [key]: !current[key] }));
    const copyMatches = () => {
        const plain = matches
            .map(
                (match, index) =>
                    `${index + 1}: ${String(match.value ?? "")} @ ${String(match.offset ?? 0)}`,
            )
            .join("\n");
        return run("copyToClipboard", { text: plain });
    };
    const presets = [
        { label: "Email", pattern: "[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}" },
        { label: "URL", pattern: "https?://[^\\s]+" },
        { label: "IPv4", pattern: "\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b" },
        { label: "ISO date", pattern: "\\b\\d{4}-\\d{2}-\\d{2}\\b" },
        { label: "Phone (E.164)", pattern: "\\+[1-9]\\d{1,14}" },
        {
            label: "UUID v4",
            pattern: "\\b[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}\\b",
        },
    ];
    const flags: Array<[keyof RegexOptions, string, string]> = [
        ["global", "Global", "すべて検索"],
        ["caseInsensitive", "Case insensitive", "大文字小文字を無視"],
        ["multiline", "Multiline", "複数行"],
        ["dotMatchesEverything", "Dot matches newline", "ドットを改行にも一致"],
        ["useUnicodeProperties", "Unicode properties", "Unicodeプロパティ"],
        ["extendedPatternSyntax", "Extended syntax", "拡張構文"],
    ];
    const insert = (value: string) => setPattern((current) => current + value);
    return (
        <div className="tool-page">
            <Heading title={text(locale, "Regex Tester", "正規表現テスター")} />
            <div className="regex-layout">
                <div className="input-card">
                    <div className="toolbar-row">
                        <Field label={text(locale, "Preset", "プリセット")}>
                            <SelectBox
                                className="compact"
                                defaultValue=""
                                onValueChange={(value) => {
                                    const preset = presets.find((item) => item.label === value);
                                    if (preset) setPattern(preset.pattern);
                                }}
                                options={[
                                    {
                                        value: "",
                                        label: text(
                                            locale,
                                            "Select preset pattern…",
                                            "プリセットを選択…",
                                        ),
                                    },
                                    ...presets.map((preset) => ({
                                        value: preset.label,
                                        label: preset.label,
                                    })),
                                ]}
                            />
                        </Field>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(run("copyToClipboard", { text: pattern }))}
                        >
                            {text(locale, "Copy pattern", "パターンをコピー")}
                        </Button>
                    </div>
                    <Field label={text(locale, "Pattern", "パターン")}>
                        <Input
                            className="form-control code-input"
                            value={pattern}
                            onChange={(event) => setPattern(event.target.value)}
                            placeholder="(\\w+)"
                        />
                    </Field>
                    <div className="flag-grid">
                        {flags.map(([key, en, ja]) => (
                            <label key={key} className="inline-check">
                                <Checkbox
                                    checked={options[key]}
                                    onCheckedChange={() => setOption(key)}
                                />
                                {text(locale, en, ja)}
                            </label>
                        ))}
                    </div>
                    <Area
                        label={text(locale, "Test text", "テスト文字列")}
                        value={input}
                        onChange={setInput}
                        rows={14}
                    />
                    <div className="action-row">
                        <Button onClick={() => runUiEffect(evaluate())} disabled={busy}>
                            {text(locale, "Test pattern", "テスト")}
                        </Button>
                        <Button
                            variant="outline"
                            onClick={() => runUiEffect(copyMatches())}
                            disabled={!matches.length}
                        >
                            {text(locale, "Copy matches", "マッチをコピー")}
                        </Button>
                    </div>
                </div>
                <div className="regex-results">
                    <section className="input-card">
                        <div className="toolbar-row">
                            <h2>{text(locale, "Matches", "マッチ")}</h2>
                            <span className="muted-text">{matches.length}</span>
                        </div>
                        {matches.length === 0 ? (
                            <p className="muted-text">
                                {text(locale, "No matches yet.", "マッチ結果はありません。")}
                            </p>
                        ) : (
                            <div className="match-list">
                                {matches.map((match, index) => (
                                    <div key={index} className="match-item">
                                        <strong>{String(match.value)}</strong>
                                        <span>
                                            {text(locale, "Offset", "位置")} {String(match.offset)}{" "}
                                            · {text(locale, "Length", "長さ")}{" "}
                                            {String(match.length)}
                                        </span>
                                        {((match.groups as Array<Record<string, unknown>>) ?? [])
                                            .slice(1)
                                            .map((group) => (
                                                <code key={String(group.index)}>
                                                    {String(group.name || group.index)}:{" "}
                                                    {String(group.value)}
                                                </code>
                                            ))}
                                    </div>
                                ))}
                            </div>
                        )}
                    </section>
                    <section className="input-card">
                        <h2>{text(locale, "Replacement", "置換")}</h2>
                        <Field label={text(locale, "Replacement pattern", "置換文字列")}>
                            <Input
                                className="form-control code-input"
                                value={replacement}
                                onChange={(event) => setReplacement(event.target.value)}
                                placeholder="$1"
                            />
                        </Field>
                        <Area
                            label={text(locale, "Result", "結果")}
                            value={replacedText}
                            readOnly
                            rows={8}
                        />
                        <Button
                            variant="outline"
                            onClick={() =>
                                runUiEffect(run("copyToClipboard", { text: replacedText }))
                            }
                            disabled={!replacedText}
                        >
                            {text(locale, "Copy result", "結果をコピー")}
                        </Button>
                    </section>
                    <section className="input-card">
                        <div className="toolbar-row">
                            <h2>{text(locale, "Quick reference", "クイックリファレンス")}</h2>
                            <Button
                                variant="ghost"
                                onClick={() => setQuickReference((value) => !value)}
                            >
                                {quickReference ? "−" : "+"}
                            </Button>
                        </div>
                        {quickReference && (
                            <div className="quick-reference">
                                {[
                                    ["\\d", text(locale, "Digit", "数字")],
                                    ["\\w", text(locale, "Word character", "単語文字")],
                                    ["\\s", text(locale, "Whitespace", "空白")],
                                    [".", text(locale, "Any character", "任意の文字")],
                                    ["^", text(locale, "Start of line", "行頭")],
                                    ["$", text(locale, "End of line", "行末")],
                                    [
                                        "(?:…)",
                                        text(locale, "Non-capturing group", "非キャプチャグループ"),
                                    ],
                                    [
                                        "(?<name>…)",
                                        text(locale, "Named capture", "名前付きキャプチャ"),
                                    ],
                                ].map(([token, description]) => (
                                    <Button
                                        key={token}
                                        variant="ghost"
                                        className="quick-reference-item h-auto"
                                        onClick={() => insert(token)}
                                    >
                                        <code>{token}</code>
                                        <span>{description}</span>
                                    </Button>
                                ))}
                            </div>
                        )}
                    </section>
                </div>
            </div>
            <OutputMessage locale={locale} error={error} onDismiss={() => setError("")} />
        </div>
    );
}
