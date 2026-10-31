import { Checkbox as BaseCheckbox } from "@base-ui/react/checkbox";
import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

type CheckboxProps = Omit<ComponentProps<typeof BaseCheckbox.Root>, "onCheckedChange"> & {
    onCheckedChange?: (checked: boolean) => void;
};

function Checkbox({ className, onCheckedChange, ...props }: CheckboxProps) {
    return (
        <BaseCheckbox.Root
            data-slot="checkbox"
            className={cn("checkbox-root", className)}
            onCheckedChange={(checked) => onCheckedChange?.(checked)}
            {...props}
        >
            <BaseCheckbox.Indicator data-slot="checkbox-indicator" className="checkbox-indicator">
                ✓
            </BaseCheckbox.Indicator>
        </BaseCheckbox.Root>
    );
}

export { Checkbox };
