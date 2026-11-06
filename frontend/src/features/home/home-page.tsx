import { Button } from "@/shared/components/ui/button";
import type { LucideIcon } from "lucide-react";
import type { Locale, ToolId } from "@/shared/types";

type HomeTool = {
    id: ToolId;
    en: string;
    ja: string;
    detailEn: string;
    detailJa: string;
    icon: LucideIcon;
};

export function WelcomePage({
    locale,
    onOpen,
    tools,
}: {
    locale: Locale;
    onOpen: (page: ToolId) => void;
    tools: HomeTool[];
}) {
    return (
        <div className="welcome-page">
            <div className="tool-card-grid">
                {tools.map((tool) => {
                    const ToolIcon = tool.icon;
                    return (
                        <Button
                            key={tool.id}
                            variant="ghost"
                            className="tool-card h-auto"
                            onClick={() => onOpen(tool.id)}
                        >
                            <ToolIcon className="tool-card-icon" aria-hidden="true" />
                            <strong>{locale === "ja" ? tool.ja : tool.en}</strong>
                            <span>{locale === "ja" ? tool.detailJa : tool.detailEn}</span>
                        </Button>
                    );
                })}
            </div>
        </div>
    );
}
