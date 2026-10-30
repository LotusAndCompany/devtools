import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

function Input({ className, type = "text", ...props }: ComponentProps<"input">) {
    return (
        <input data-slot="input" type={type} className={cn("form-control", className)} {...props} />
    );
}

export { Input };
