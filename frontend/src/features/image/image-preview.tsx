import type { MouseEvent } from "react";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface ImagePreviewProps {
    locale: Locale;
    image: string;
    onChooseTransparentPoint: (event: MouseEvent<HTMLImageElement>) => void;
}

export function ImagePreview({ locale, image, onChooseTransparentPoint }: ImagePreviewProps) {
    return (
        <div className="image-preview-panel">
            {image ? (
                <img
                    src={image}
                    alt={text(
                        locale,
                        "Editable image preview. Click to select a color for transparency.",
                        "編集プレビュー。クリックして透明化する色を選択。",
                    )}
                    onClick={onChooseTransparentPoint}
                />
            ) : (
                <div className="empty-state">
                    {text(locale, "Open an image to begin.", "画像を開いて編集を始めてください。")}
                </div>
            )}
        </div>
    );
}
