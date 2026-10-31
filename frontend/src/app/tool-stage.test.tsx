import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it, vi } from "vitest";
import { createOperationRunner } from "@/test/operation-runner";
import { ToolStage } from "./tool-stage";
import type { PageId } from "./tool-registry";

describe("ToolStage", () => {
    it("keeps registry-marked screens mounted while switching pages", async () => {
        const run = createOperationRunner({
            databaseHistory: () => ({
                history: [],
                selectedFileName: "",
                connected: true,
                connectedType: "QSQLITE",
            }),
        });
        const props = {
            activePage: "db" as const,
            locale: "en" as const,
            run,
            databaseNavigationVersion: 0,
            droppedImage: null,
            onDroppedImageConsumed: vi.fn<(id: string) => void>(),
            onNavigate: vi.fn<(page: PageId) => void>(),
        };
        const view = render(<ToolStage {...props} />);
        await screen.findByRole("tabpanel");
        const sql = screen.getByRole("tabpanel").querySelector("textarea")!;
        fireEvent.change(sql, { target: { value: "SELECT 1" } });

        view.rerender(<ToolStage {...props} activePage="welcome" />);
        expect(view.container.querySelector('[data-page="db"]')?.hasAttribute("hidden")).toBe(true);
        view.rerender(<ToolStage {...props} />);

        await waitFor(() =>
            expect(screen.getByRole("tabpanel").querySelector("textarea")?.value).toBe("SELECT 1"),
        );
    });
});
