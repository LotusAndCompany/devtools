import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { OperationInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { QrPage } from "./qr-page";

describe("QrPage", () => {
    it("saves the input used to create the displayed preview", async () => {
        const savedInputs: OperationInput<"saveQrImage">[] = [];
        const run = createOperationRunner({
            generateQr: ({ parameters }) => ({
                content: parameters.text_content,
                imageDataUrl: "data:image/png;base64,preview",
            }),
            saveQrImage: (input) => {
                savedInputs.push(input);
                return { fileName: "qr.png" };
            },
        });

        render(<QrPage locale="en" run={run} />);
        const input = screen.getByRole("textbox");
        fireEvent.change(input, { target: { value: "preview input" } });
        fireEvent.click(screen.getByRole("button", { name: "Generate" }));
        await screen.findByAltText("Generated QR code");

        fireEvent.change(input, { target: { value: "new input" } });
        fireEvent.click(screen.getByRole("button", { name: "Save image" }));

        await waitFor(() =>
            expect(savedInputs).toEqual([
                { type: "text", parameters: { text_content: "preview input" } },
            ]),
        );
    });
});
