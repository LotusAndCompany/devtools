import { Switch as BaseSwitch } from "@base-ui/react/switch";
import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

type SwitchProps = Omit<ComponentProps<typeof BaseSwitch.Root>, "onCheckedChange"> & {
    onCheckedChange?: (checked: boolean) => void;
};

function Switch({ className, onCheckedChange, ...props }: SwitchProps) {
    return (
        <BaseSwitch.Root
            data-slot="switch"
            className={cn("switch-root", className)}
            onCheckedChange={(checked) => onCheckedChange?.(checked)}
            {...props}
        >
            <BaseSwitch.Thumb className="switch-thumb" />
        </BaseSwitch.Root>
    );
}

export { Switch };
