import { Button as BaseButton } from "@base-ui/react/button";
import { cva, type VariantProps } from "class-variance-authority";
import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

const buttonVariants = cva(
    "inline-flex shrink-0 items-center justify-center gap-2 rounded-md text-xs font-medium transition-colors focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring disabled:pointer-events-none",
    {
        variants: {
            variant: {
                default:
                    "bg-primary text-primary-foreground hover:bg-primary-hover active:bg-primary-pressed disabled:bg-primary-disabled disabled:text-primary-foreground-disabled",
                secondary:
                    "bg-secondary text-secondary-foreground hover:bg-secondary-hover active:bg-secondary-pressed disabled:bg-secondary-disabled disabled:text-secondary-foreground-disabled",
                outline:
                    "border border-border bg-background hover:bg-muted active:bg-surface-main-3 disabled:border-border-disabled disabled:text-secondary-foreground-disabled",
                ghost: "hover:bg-muted active:bg-surface-main-3 disabled:text-secondary-foreground-disabled",
            },
            size: {
                default: "h-8 px-3",
                sm: "h-7 px-2.5 text-[11px]",
            },
        },
        defaultVariants: { variant: "default", size: "default" },
    },
);

type ButtonProps = ComponentProps<typeof BaseButton> & VariantProps<typeof buttonVariants>;

function Button({ className, variant, size, ...props }: ButtonProps) {
    return (
        <BaseButton
            data-slot="button"
            className={cn(buttonVariants({ variant, size }), className)}
            {...props}
        />
    );
}

export { Button, buttonVariants };
