import { fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { OperationInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { DatabasePage } from "./db-page";

describe("DatabasePage", () => {
    it("executes the edited SQL and renders the returned rows", async () => {
        const statements: OperationInput<"databaseQuery">[] = [];
        const run = createOperationRunner({
            databaseHistory: () => ({
                history: [],
                selectedFileName: "",
                connected: true,
                connectedType: "sqlite",
            }),
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

    it("quotes table names with the active MySQL driver's identifier quote", async () => {
        const run = createOperationRunner({
            databaseHistory: () => ({
                history: [],
                selectedFileName: "",
                connected: true,
                connectedType: "mysql",
            }),
            databaseTables: () => ({ tables: ["user`profiles"] }),
        });

        render(<DatabasePage locale="en" run={run} active />);
        fireEvent.click(screen.getByRole("button", { name: "Refresh" }));
        fireEvent.click(await screen.findByRole("button", { name: "user`profiles" }));

        expect(
            (screen.getByRole("tabpanel").querySelector("textarea") as HTMLTextAreaElement).value,
        ).toBe("SELECT * FROM `user``profiles` LIMIT 1000;");
    });

    it("asks for a saved MySQL connection password before connecting", async () => {
        const connections: OperationInput<"databaseConnect">[] = [];
        const run = createOperationRunner({
            databaseHistory: () => ({
                history: [
                    {
                        index: 0,
                        type: "mysql",
                        host: "db.example.test",
                        database: "app",
                        username: "dev",
                        displayName: "Development database",
                    },
                ],
                selectedFileName: "",
                connected: false,
                connectedType: "",
            }),
            databaseConnect: (input) => {
                connections.push(input);
                return {
                    connected: true,
                    displayName: "Development database",
                    tables: [],
                };
            },
        });

        render(<DatabasePage locale="en" run={run} active />);
        fireEvent.click(screen.getByRole("button", { name: "Connection settings" }));
        fireEvent.click(screen.getAllByRole("button", { name: "Connect" })[0]);
        fireEvent.change(await screen.findByLabelText("Password"), {
            target: { value: "secret" },
        });
        const passwordForm = screen
            .getByLabelText("Password")
            .closest(".db-connect-form") as HTMLElement;
        fireEvent.click(within(passwordForm).getByRole("button", { name: "Connect" }));

        await waitFor(() => expect(connections).toEqual([{ historyIndex: 0, password: "secret" }]));
    });
});
