import type { ComponentProps } from "react";
import { cn } from "@/lib/utils";

function Textarea({ className, ...props }: ComponentProps<"textarea">) {
    return (
        <textarea
            data-slot="textarea"
            className={cn("form-control code-area", className)}
            {...props}
        />
    );
}

export { Textarea };
