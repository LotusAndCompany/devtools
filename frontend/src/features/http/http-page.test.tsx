import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { HttpResponse, OperationInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { HttpPage } from "./http-page";

describe("HttpPage", () => {
    it("sends the entered URL and shows the native response", async () => {
        const requests: OperationInput<"sendHttpRequest">[] = [];
        const response: HttpResponse = {
            status: 201,
            reason: "Created",
            elapsedMs: 8,
            errorCode: 0,
            error: "",
            body: "created item",
            bytes: 12,
        };
        const run = createOperationRunner({
            sendHttpRequest: (input) => {
                requests.push(input);
                return response;
            },
        });

        render(<HttpPage locale="en" run={run} />);
        fireEvent.change(screen.getByLabelText("URL"), {
            target: { value: "https://example.test/items" },
        });
        fireEvent.click(screen.getByRole("button", { name: "Send" }));

        await waitFor(() => expect(screen.getAllByDisplayValue("created item")).toHaveLength(2));
        expect(requests).toHaveLength(1);
        expect(requests[0]).toMatchObject({
            method: "GET",
            url: "https://example.test/items",
            parameters: [],
        });
    });
});
