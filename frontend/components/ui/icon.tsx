import type { ComponentProps } from "react";
import { cn } from "@/lib/utils";

function Icon({ className, children, ...props }: ComponentProps<"span">) {
    return (
        <span aria-hidden="true" className={cn("material-symbols", className)} {...props}>
            {children}
        </span>
    );
}

export { Icon };
