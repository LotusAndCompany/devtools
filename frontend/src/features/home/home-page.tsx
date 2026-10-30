import { Button } from "@/shared/components/ui/button";
import { Icon } from "@/shared/components/ui/icon";
import type { Locale, ToolId } from "@/shared/types";

type HomeTool = { id: ToolId; en: string; ja: string; icon: string };

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
                {tools.map((tool) => (
                    <Button
                        key={tool.id}
                        variant="ghost"
                        className="tool-card h-auto"
                        onClick={() => onOpen(tool.id)}
                    >
                        <Icon className="tool-card-icon">{tool.icon}</Icon>
                        <strong>{locale === "ja" ? tool.ja : tool.en}</strong>
                    </Button>
                ))}
            </div>
        </div>
    );
}
