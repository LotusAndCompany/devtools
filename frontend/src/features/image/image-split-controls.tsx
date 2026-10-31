import type { Dispatch, SetStateAction } from "react";
import { Button } from "@/shared/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/shared/components/ui/card";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { FormField } from "@/shared/components/form-field";
import { Field, FieldGroup, FieldLabel } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectControl } from "@/shared/components/select-control";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface ImageSplitControlsProps {
    locale: Locale;
    busy: boolean;
    hasImage: boolean;
    splitMode: "division" | "cell-size";
    horizontalDivisions: string;
    verticalDivisions: string;
    cellWidth: string;
    cellHeight: string;
    ignoreRemainders: boolean;
    setSplitMode: Dispatch<SetStateAction<"division" | "cell-size">>;
    setHorizontalDivisions: Dispatch<SetStateAction<string>>;
    setVerticalDivisions: Dispatch<SetStateAction<string>>;
    setCellWidth: Dispatch<SetStateAction<string>>;
    setCellHeight: Dispatch<SetStateAction<string>>;
    setIgnoreRemainders: Dispatch<SetStateAction<boolean>>;
    onSplit: () => void;
}

export function ImageSplitControls({
    locale,
    busy,
    hasImage,
    splitMode,
    horizontalDivisions,
    verticalDivisions,
    cellWidth,
    cellHeight,
    ignoreRemainders,
    setSplitMode,
    setHorizontalDivisions,
    setVerticalDivisions,
    setCellWidth,
    setCellHeight,
    setIgnoreRemainders,
    onSplit,
}: ImageSplitControlsProps) {
    return (
        <Card role="region" className="input-card">
            <CardHeader>
                <CardTitle>{text(locale, "Split image", "画像を分割")}</CardTitle>
            </CardHeader>
            <CardContent className="flex flex-col gap-3">
                <FormField label={text(locale, "Split by", "分割方法")} nativeLabel={false}>
                    <SelectControl
                        value={splitMode}
                        onValueChange={setSplitMode}
                        options={[
                            { value: "division", label: text(locale, "Number of parts", "分割数") },
                            { value: "cell-size", label: text(locale, "Cell size", "セルサイズ") },
                        ]}
                    />
                </FormField>
                {splitMode === "division" ? (
                    <FieldGroup className="form-grid two">
                        <FormField label={text(locale, "Columns", "横の分割数")}>
                            <Input
                                type="number"
                                min="1"
                                value={horizontalDivisions}
                                onChange={(event) => setHorizontalDivisions(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Rows", "縦の分割数")}>
                            <Input
                                type="number"
                                min="1"
                                value={verticalDivisions}
                                onChange={(event) => setVerticalDivisions(event.target.value)}
                            />
                        </FormField>
                    </FieldGroup>
                ) : (
                    <FieldGroup className="form-grid two">
                        <FormField label={text(locale, "Cell width", "セル幅")}>
                            <Input
                                type="number"
                                min="1"
                                value={cellWidth}
                                onChange={(event) => setCellWidth(event.target.value)}
                            />
                        </FormField>
                        <FormField label={text(locale, "Cell height", "セル高さ")}>
                            <Input
                                type="number"
                                min="1"
                                value={cellHeight}
                                onChange={(event) => setCellHeight(event.target.value)}
                            />
                        </FormField>
                    </FieldGroup>
                )}
                <Field orientation="horizontal">
                    <Checkbox
                        id="image-ignore-remainder"
                        checked={ignoreRemainders}
                        onCheckedChange={setIgnoreRemainders}
                    />
                    <FieldLabel htmlFor="image-ignore-remainder" className="font-normal">
                        {text(locale, "Ignore remainder pixels", "余りのピクセルを無視")}
                    </FieldLabel>
                </Field>
                <Button variant="outline" onClick={onSplit} disabled={busy || !hasImage}>
                    {text(locale, "Choose folder and split", "保存先を選んで分割")}
                </Button>
            </CardContent>
        </Card>
    );
}
