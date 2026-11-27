import { fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { OperationInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { TimestampPage } from "./timestamp-page";

describe("TimestampPage", () => {
    it("preserves seconds and milliseconds when converting the UTC field", async () => {
        const conversions: OperationInput<"convertTimestamp">[] = [];
        const run = createOperationRunner({
            convertTimestamp: (input) => {
                conversions.push(input);
                return {
                    seconds: "1793709296",
                    milliseconds: "1793709296789",
                    utc: "2026-11-03T12:34:56.789Z",
                    local: "2026-11-03T12:34:56.789+00:00",
                };
            },
        });

        render(<TimestampPage locale="en" run={run} />);
        const utcInput = screen.getAllByPlaceholderText("YYYY-MM-DDTHH:mm:ss.SSS")[1];
        fireEvent.change(utcInput, {
            target: { value: "2026-11-03T12:34:56.789" },
        });
        fireEvent.click(
            within(utcInput.closest(".timestamp-row")!).getByRole("button", { name: "Convert" }),
        );

        await waitFor(() =>
            expect(conversions).toEqual([{ value: "2026-11-03T12:34:56.789Z", source: "iso" }]),
        );
    });
});
