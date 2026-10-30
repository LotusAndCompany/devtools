import { useEffect, useState, type MouseEvent } from "react";
import { Effect } from "effect";
import { Button } from "@/shared/components/ui/button";
import { runUiEffect } from "@/shared/effect";
import { Heading, OutputMessage, text, useOperation } from "@/shared/tool-page";
import type { ToolPageProps } from "@/shared/types";
import { ImageEditControls } from "./image-edit-controls";
import { ImagePreview } from "./image-preview";
import { ImageSplitControls } from "./image-split-controls";

export function ImagePage({
    locale,
    run,
    active,
    droppedImage,
    onDroppedImageConsumed,
}: ToolPageProps) {
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
                <ImagePreview
                    locale={locale}
                    image={image}
                    onChooseTransparentPoint={chooseTransparentPoint}
                />
                <div className="image-controls">
                    <ImageEditControls
                        locale={locale}
                        busy={busy}
                        hasImage={Boolean(image)}
                        targetWidth={targetWidth}
                        targetHeight={targetHeight}
                        horizontalScale={horizontalScale}
                        verticalScale={verticalScale}
                        keepAspect={keepAspect}
                        smooth={smooth}
                        colorMode={colorMode}
                        tolerance={tolerance}
                        transparency={transparency}
                        contiguousOnly={contiguousOnly}
                        setTargetWidth={setTargetWidth}
                        setTargetHeight={setTargetHeight}
                        setHorizontalScale={setHorizontalScale}
                        setVerticalScale={setVerticalScale}
                        setKeepAspect={setKeepAspect}
                        setSmooth={setSmooth}
                        setColorMode={setColorMode}
                        setTolerance={setTolerance}
                        setTransparency={setTransparency}
                        setContiguousOnly={setContiguousOnly}
                        onEdit={(action, extra) => runUiEffect(performEdit(action, extra))}
                    />
                    <ImageSplitControls
                        locale={locale}
                        busy={busy}
                        hasImage={Boolean(image)}
                        splitMode={splitMode}
                        horizontalDivisions={horizontalDivisions}
                        verticalDivisions={verticalDivisions}
                        cellWidth={cellWidth}
                        cellHeight={cellHeight}
                        ignoreRemainders={ignoreRemainders}
                        setSplitMode={setSplitMode}
                        setHorizontalDivisions={setHorizontalDivisions}
                        setVerticalDivisions={setVerticalDivisions}
                        setCellWidth={setCellWidth}
                        setCellHeight={setCellHeight}
                        setIgnoreRemainders={setIgnoreRemainders}
                        onSplit={() => runUiEffect(split())}
                    />
                </div>
            </div>
        </div>
    );
}
