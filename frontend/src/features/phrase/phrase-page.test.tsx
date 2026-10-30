import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { PhraseTemplate } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { PhrasePage } from "./phrase-page";

describe("PhrasePage", () => {
    it("saves a template and refreshes the list", async () => {
        let saved: PhraseTemplate | null = null;
        const run = createOperationRunner({
            phraseData: (input) => {
                if (input.action === "list") {
                    return { templates: saved ? [saved] : [] };
                }
                if (input.action === "save") {
                    saved = {
                        filename: "greeting.txt",
                        title: input.title,
                        content: input.content,
                    };
                    return { filename: saved.filename };
                }
                return { deleted: true };
            },
        });

        render(<PhrasePage locale="en" run={run} active />);
        fireEvent.change(screen.getByLabelText("Title"), {
            target: { value: "Greeting" },
        });
        fireEvent.change(screen.getByLabelText("Template text"), {
            target: { value: "Hello there" },
        });
        fireEvent.click(screen.getByRole("button", { name: /Save template/ }));

        expect(await screen.findByText("Template saved.")).toBeTruthy();
        await waitFor(() => expect(screen.getByText("Greeting")).toBeTruthy());
        expect(saved).toEqual({
            filename: "greeting.txt",
            title: "Greeting",
            content: "Hello there",
        });
    });
});
