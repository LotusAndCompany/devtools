import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import type { ImageEditInput } from "@/shared/bridge/types";
import { createOperationRunner } from "@/test/operation-runner";
import { ImagePage } from "./image-page";

describe("ImagePage", () => {
    it("applies an edit to a dropped image", async () => {
        const edits: ImageEditInput[] = [];
        const run = createOperationRunner({
            imageEdit: (input) => {
                edits.push(input);
                return { imageDataUrl: "data:image/png;base64,edited", width: 24, height: 32 };
            },
        });

        render(
            <ImagePage
                locale="en"
                run={run}
                active
                droppedImage={{
                    id: "drop-1",
                    imageDataUrl: "data:image/png;base64,original",
                    fileName: "sample.png",
                    width: 32,
                    height: 24,
                }}
                onDroppedImageConsumed={() => {}}
            />,
        );
        await screen.findByText(/sample\.png · 32 × 24/);
        fireEvent.click(screen.getByRole("button", { name: /Rotate left/ }));

        await waitFor(() => expect(edits).toEqual([{ action: "rotate-left" }]));
        expect(screen.getByText(/sample\.png · 24 × 32/)).toBeTruthy();
    });
});
