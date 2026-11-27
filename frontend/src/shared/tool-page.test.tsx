import { fireEvent, render, screen } from "@testing-library/react";
import { describe, expect, it, vi } from "vitest";
import { OutputMessage } from "@/shared/tool-page";

describe("OutputMessage", () => {
    it("shows operation errors inline instead of opening a modal", () => {
        const onDismiss = vi.fn<() => void>();
        render(<OutputMessage locale="ja" error="Cannot parse input text" onDismiss={onDismiss} />);

        expect(screen.getByRole("alert").textContent).toContain("処理に失敗しました");
        expect(screen.getByRole("alert").textContent).toContain("Cannot parse input text");
        expect(screen.queryByRole("alertdialog")).toBeNull();

        fireEvent.click(screen.getByRole("button", { name: "閉じる" }));
        expect(onDismiss).toHaveBeenCalledTimes(1);
    });
});
