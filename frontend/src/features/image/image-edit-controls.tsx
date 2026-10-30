import type { Dispatch, SetStateAction } from "react";
import { Button } from "@/shared/components/ui/button";
import { Card } from "@/shared/components/ui/card";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { Field, FieldGroup } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { text } from "@/shared/tool-page";
import type { ImageEditInput } from "@/shared/bridge/types";
import type { Locale } from "@/shared/types";

interface ImageEditControlsProps {
    locale: Locale;
    busy: boolean;
    hasImage: boolean;
    targetWidth: string;
    targetHeight: string;
    horizontalScale: string;
    verticalScale: string;
    keepAspect: boolean;
    smooth: boolean;
    colorMode: "rgb" | "hsl" | "hsv";
    tolerance: string;
    transparency: string;
    contiguousOnly: boolean;
    setTargetWidth: Dispatch<SetStateAction<string>>;
    setTargetHeight: Dispatch<SetStateAction<string>>;
    setHorizontalScale: Dispatch<SetStateAction<string>>;
    setVerticalScale: Dispatch<SetStateAction<string>>;
    setKeepAspect: Dispatch<SetStateAction<boolean>>;
    setSmooth: Dispatch<SetStateAction<boolean>>;
    setColorMode: Dispatch<SetStateAction<"rgb" | "hsl" | "hsv">>;
    setTolerance: Dispatch<SetStateAction<string>>;
    setTransparency: Dispatch<SetStateAction<string>>;
    setContiguousOnly: Dispatch<SetStateAction<boolean>>;
    onEdit: (input: ImageEditInput) => void;
}

export function ImageEditControls(props: ImageEditControlsProps) {
    const {
        locale,
        busy,
        hasImage,
        targetWidth,
        targetHeight,
        horizontalScale,
        verticalScale,
        keepAspect,
        smooth,
        colorMode,
        tolerance,
        transparency,
        contiguousOnly,
        setTargetWidth,
        setTargetHeight,
        setHorizontalScale,
        setVerticalScale,
        setKeepAspect,
        setSmooth,
        setColorMode,
        setTolerance,
        setTransparency,
        setContiguousOnly,
        onEdit,
    } = props;

    return (
        <div className="image-edit-controls">
            <Card as="section" className="input-card">
                <h2>{text(locale, "Resize", "サイズ変更")}</h2>
                <FieldGroup className="form-grid two">
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
                </FieldGroup>
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
                        disabled={busy || !hasImage}
                        onClick={() =>
                            onEdit({
                                action: "resize-size",
                                width: Number(targetWidth),
                                height: Number(targetHeight),
                                keepAspectRatio: keepAspect,
                                smoothScaling: smooth,
                            })
                        }
                    >
                        {text(locale, "Apply size", "サイズを適用")}
                    </Button>
                    <Button
                        variant="outline"
                        disabled={busy || !hasImage}
                        onClick={() =>
                            onEdit({
                                action: "resize-scale",
                                horizontalPercent: Number(horizontalScale),
                                verticalPercent: Number(verticalScale),
                                keepAspectRatio: keepAspect,
                                smoothScaling: smooth,
                            })
                        }
                    >
                        {text(locale, "Apply scale", "倍率を適用")}
                    </Button>
                </div>
            </Card>
            <Card as="section" className="input-card">
                <h2>{text(locale, "Rotate and flip", "回転・反転")}</h2>
                <div className="button-row">
                    <Button
                        variant="outline"
                        disabled={busy || !hasImage}
                        onClick={() => onEdit({ action: "rotate-left" })}
                    >
                        ↶ {text(locale, "Rotate left", "左に回転")}
                    </Button>
                    <Button
                        variant="outline"
                        disabled={busy || !hasImage}
                        onClick={() => onEdit({ action: "rotate-right" })}
                    >
                        ↷ {text(locale, "Rotate right", "右に回転")}
                    </Button>
                    <Button
                        variant="outline"
                        disabled={busy || !hasImage}
                        onClick={() => onEdit({ action: "flip-horizontal" })}
                    >
                        {text(locale, "Flip horizontal", "左右反転")}
                    </Button>
                    <Button
                        variant="outline"
                        disabled={busy || !hasImage}
                        onClick={() => onEdit({ action: "flip-vertical" })}
                    >
                        {text(locale, "Flip vertical", "上下反転")}
                    </Button>
                </div>
            </Card>
            <Card as="section" className="input-card">
                <h2>{text(locale, "Make a color transparent", "色を透明化")}</h2>
                <p className="muted-text">
                    {text(
                        locale,
                        "Click a pixel in the preview to apply these settings.",
                        "プレビュー上のピクセルをクリックすると設定を適用します。",
                    )}
                </p>
                <FieldGroup className="form-grid two">
                    <Field label={text(locale, "Color mode", "色空間")} nativeLabel={false}>
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
                </FieldGroup>
                <label className="inline-check">
                    <Checkbox checked={contiguousOnly} onCheckedChange={setContiguousOnly} />
                    {text(locale, "Only connected pixels", "隣接する領域のみ")}
                </label>
            </Card>
        </div>
    );
}
