import { Field as BaseField } from "@base-ui/react/field";
import {
    Children,
    cloneElement,
    isValidElement,
    useId,
    type ReactElement,
    type ReactNode,
} from "react";
import { cn } from "@/lib/utils";

function Field({
    label,
    children,
    className,
}: {
    label: string;
    children: ReactNode;
    className?: string;
}) {
    const id = useId();
    const child = Children.only(children);
    const control = isValidElement(child)
        ? cloneElement(child as ReactElement<{ id?: string }>, {
              id: (child.props as { id?: string }).id ?? id,
          })
        : child;

    return (
        <BaseField.Root className={cn("field", className)}>
            <BaseField.Label className="field-label" htmlFor={id}>
                {label}
            </BaseField.Label>
            {control}
        </BaseField.Root>
    );
}

export { Field };
