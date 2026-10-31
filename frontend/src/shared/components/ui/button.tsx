import { Button as BaseButton } from "@base-ui/react/button";
import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

type ButtonVariant = "default" | "secondary" | "outline" | "ghost" | "destructive" | null;
type ButtonSize = "default" | "sm" | "icon" | null;

type ButtonProps = ComponentProps<typeof BaseButton> & {
    variant?: ButtonVariant;
    size?: ButtonSize;
};

function Button({ className, variant = "default", size = "default", ...props }: ButtonProps) {
    return (
        <BaseButton
            data-slot="button"
            data-variant={variant ?? undefined}
            data-size={size ?? undefined}
            className={cn("button", className)}
            {...props}
        />
    );
}

export { Button, type ButtonSize, type ButtonVariant };
