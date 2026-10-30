import type { Dispatch, SetStateAction } from "react";
import { Button } from "@/shared/components/ui/button";
import { Checkbox } from "@/shared/components/ui/checkbox";
import { Field } from "@/shared/components/ui/field";
import { Input } from "@/shared/components/ui/input";
import { SelectBox } from "@/shared/components/ui/select";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface ImageSplitControlsProps {
    locale: Locale;
    busy: boolean;
    hasImage: boolean;
    splitMode: string;
    horizontalDivisions: string;
    verticalDivisions: string;
    cellWidth: string;
    cellHeight: string;
    ignoreRemainders: boolean;
    setSplitMode: Dispatch<SetStateAction<string>>;
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
        <section className="input-card">
            <h2>{text(locale, "Split image", "画像を分割")}</h2>
            <Field label={text(locale, "Split by", "分割方法")}>
                <SelectBox
                    value={splitMode}
                    onValueChange={setSplitMode}
                    options={[
                        { value: "division", label: text(locale, "Number of parts", "分割数") },
                        { value: "cell-size", label: text(locale, "Cell size", "セルサイズ") },
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
                            onChange={(event) => setHorizontalDivisions(event.target.value)}
                        />
                    </Field>
                    <Field label={text(locale, "Rows", "縦の分割数")}>
                        <Input
                            className="form-control"
                            type="number"
                            min="1"
                            value={verticalDivisions}
                            onChange={(event) => setVerticalDivisions(event.target.value)}
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
                <Checkbox checked={ignoreRemainders} onCheckedChange={setIgnoreRemainders} />
                {text(locale, "Ignore remainder pixels", "余りのピクセルを無視")}
            </label>
            <Button variant="outline" onClick={onSplit} disabled={busy || !hasImage}>
                {text(locale, "Choose folder and split", "保存先を選んで分割")}
            </Button>
        </section>
    );
}
