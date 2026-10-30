import { WelcomePage } from "@/features/home";
import { toolForPage, titleFor, tools, type PageId } from "./tool-registry";
import type { Locale, ToolPageProps } from "@/shared/types";

type ToolStageProps = {
    activePage: PageId;
    locale: Locale;
    run: ToolPageProps["run"];
    databaseNavigationVersion: number;
    droppedImage: ToolPageProps["droppedImage"];
    onDroppedImageConsumed: ToolPageProps["onDroppedImageConsumed"];
    onNavigate: (page: PageId) => void;
};

export function ToolStage({
    activePage,
    locale,
    run,
    databaseNavigationVersion,
    droppedImage,
    onDroppedImageConsumed,
    onNavigate,
}: ToolStageProps) {
    const activeTool = toolForPage(activePage);
    const ActiveScreen = activeTool?.screen;
    const DatabaseScreen = toolForPage("db")!.screen;

    return (
        <div className="page-stack">
            {activePage !== "db" && (
                <section
                    className="page-host active-page"
                    data-page={activePage}
                    aria-label={titleFor(activePage, locale)}
                >
                    {activePage === "welcome" ? (
                        <WelcomePage locale={locale} onOpen={onNavigate} tools={tools} />
                    ) : ActiveScreen ? (
                        <ActiveScreen
                            key={activePage}
                            locale={locale}
                            run={run}
                            active
                            droppedImage={droppedImage}
                            onDroppedImageConsumed={onDroppedImageConsumed}
                        />
                    ) : null}
                </section>
            )}
            <section
                className="page-host database-cache"
                data-page="db"
                aria-label={titleFor("db", locale)}
                hidden={activePage !== "db"}
            >
                <DatabaseScreen
                    locale={locale}
                    run={run}
                    active={activePage === "db"}
                    databaseNavigationVersion={databaseNavigationVersion}
                />
            </section>
        </div>
    );
}
