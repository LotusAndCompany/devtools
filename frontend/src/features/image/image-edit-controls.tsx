import type { Dispatch, SetStateAction } from "react";
import { Button } from "@/shared/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { FormField } from "@/shared/components/form-field";
import { Field, FieldGroup, FieldLabel } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectControl } from "@/shared/components/select-control";
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
        <>
            <Card role="region" className="input-card">
                <CardHeader>
                    <CardTitle>{text(locale, "Resize", "サイズ変更")}</CardTitle>
                </CardHeader>
                <CardContent className="flex flex-col gap-3">
                    <FieldGroup className="form-grid two">
                        <FormField label={text(locale, "Width (px)", "幅 (px)")}>
                            <Input
                                type="number"
                                min="1"
                                max="32768"
                                value={targetWidth}
                                onChange={(event) => setTargetWidth(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Height (px)", "高さ (px)")}>
                            <Input
                                type="number"
                                min="1"
                                max="32768"
                                value={targetHeight}
                                onChange={(event) => setTargetHeight(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Horizontal scale (%)", "横倍率 (%)")}>
                            <Input
                                type="number"
                                min="0.1"
                                max="1000"
                                step="0.1"
                                value={horizontalScale}
                                onChange={(event) => setHorizontalScale(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Vertical scale (%)", "縦倍率 (%)")}>
                            <Input
                                type="number"
                                min="0.1"
                                max="1000"
                                step="0.1"
                                value={verticalScale}
                                onChange={(event) => setVerticalScale(event.target.value)}
                            />
                        </FormField>
                    </FieldGroup>
                    <Field orientation="horizontal">
                        <Checkbox
                            id="image-keep-aspect"
                            checked={keepAspect}
                            onCheckedChange={setKeepAspect}
                        />
                        <FieldLabel htmlFor="image-keep-aspect" className="font-normal">
                            {text(locale, "Keep aspect ratio", "縦横比を維持")}
                        </FieldLabel>
                    </Field>
                    <Field orientation="horizontal">
                        <Checkbox
                            id="image-smooth-scale"
                            checked={smooth}
                            onCheckedChange={setSmooth}
                        />
                        <FieldLabel htmlFor="image-smooth-scale" className="font-normal">
                            {text(locale, "Smooth scaling", "滑らかに拡大縮小")}
                        </FieldLabel>
                    </Field>
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
                </CardContent>
            </Card>
            <Card role="region" className="input-card">
                <CardHeader>
                    <CardTitle>{text(locale, "Rotate and flip", "回転・反転")}</CardTitle>
                </CardHeader>
                <CardContent>
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
                </CardContent>
            </Card>
            <Card role="region" className="input-card">
                <CardHeader>
                    <CardTitle>{text(locale, "Make a color transparent", "色を透明化")}</CardTitle>
                </CardHeader>
                <CardContent className="flex flex-col gap-3">
                    <p className="muted-text">
                        {text(
                            locale,
                            "Click a pixel in the preview to apply these settings.",
                            "プレビュー上のピクセルをクリックすると設定を適用します。",
                        )}
                    </p>
                    <FieldGroup className="form-grid two">
                        <FormField label={text(locale, "Color mode", "色空間")} nativeLabel={false}>
                            <SelectControl
                                value={colorMode}
                                onValueChange={setColorMode}
                                options={[
                                    { value: "rgb", label: "RGB" },
                                    { value: "hsl", label: "HSL" },
                                    { value: "hsv", label: "HSV" },
                                ]}
                            />
                        </FormField>
                        <FormField label={text(locale, "Tolerance (0–1)", "許容範囲 (0–1)")}>
                            <Input
                                type="number"
                                min="0"
                                max="1"
                                step="0.05"
                                value={tolerance}
                                onChange={(event) => setTolerance(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Transparency (0–1)", "透明度 (0–1)")}>
                            <Input
                                type="number"
                                min="0"
                                max="1"
                                step="0.05"
                                value={transparency}
                                onChange={(event) => setTransparency(event.target.value)}
                            />
                        </FormField>
                    </FieldGroup>
                    <Field orientation="horizontal">
                        <Checkbox
                            id="image-contiguous-only"
                            checked={contiguousOnly}
                            onCheckedChange={setContiguousOnly}
                        />
                        <FieldLabel htmlFor="image-contiguous-only" className="font-normal">
                            {text(locale, "Only connected pixels", "隣接する領域のみ")}
                        </FieldLabel>
                    </Field>
                </CardContent>
            </Card>
        </>
    );
}
