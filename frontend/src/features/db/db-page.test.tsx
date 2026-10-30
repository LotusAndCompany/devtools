import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { OperationInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { DatabasePage } from "./db-page";

describe("DatabasePage", () => {
    it("executes the edited SQL and renders the returned rows", async () => {
        const statements: OperationInput<"databaseQuery">[] = [];
        const run = createOperationRunner({
            databaseHistory: () => ({ history: [], selectedFileName: "", connected: true }),
            databaseQuery: (input) => {
                statements.push(input);
                return {
                    columns: ["answer"],
                    rows: [["42"]],
                    truncated: false,
                    affectedRows: 0,
                };
            },
        });

        render(<DatabasePage locale="en" run={run} active />);
        const sql = screen.getByRole("tabpanel").querySelector("textarea")!;
        fireEvent.change(sql, { target: { value: "SELECT 42 AS answer" } });
        await waitFor(() =>
            expect(screen.getByRole("button", { name: "Execute" }).hasAttribute("disabled")).toBe(
                false,
            ),
        );
        fireEvent.click(screen.getByRole("button", { name: "Execute" }));

        expect(await screen.findByText("42")).toBeTruthy();
        expect(statements).toEqual([{ sql: "SELECT 42 AS answer" }]);
    });
});
