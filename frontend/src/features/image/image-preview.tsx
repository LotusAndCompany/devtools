import type { MouseEvent } from "react";
import { Card, CardContent } from "@/shared/components/ui/card";
import { Empty, EmptyDescription, EmptyHeader } from "@/shared/components/ui/empty";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface ImagePreviewProps {
    locale: Locale;
    image: string;
    onChooseTransparentPoint: (event: MouseEvent<HTMLImageElement>) => void;
}

export function ImagePreview({ locale, image, onChooseTransparentPoint }: ImagePreviewProps) {
    return (
        <Card className="image-preview-panel input-card p-0">
            <CardContent className="image-preview-content p-0">
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
                    <Empty className="empty-state">
                        <EmptyHeader>
                            <EmptyDescription>
                                {text(
                                    locale,
                                    "Open an image to begin.",
                                    "画像を開いて編集を始めてください。",
                                )}
                            </EmptyDescription>
                        </EmptyHeader>
                    </Empty>
                )}
            </CardContent>
        </Card>
    );
}
