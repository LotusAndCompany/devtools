import { Field as BaseField } from "@base-ui/react/field";
import {
    Children,
    cloneElement,
    isValidElement,
    useId,
    type ComponentProps,
    type ReactElement,
    type ReactNode,
} from "react";
import { cn } from "@/shared/lib/utils";

function Field({
    label,
    children,
    className,
    nativeLabel = true,
    description,
}: {
    label: string;
    children: ReactNode;
    className?: string;
    nativeLabel?: boolean;
    description?: ReactNode;
}) {
    const id = useId();
    const child = Children.only(children);
    const controlId =
        isValidElement(child) && typeof (child.props as { id?: unknown }).id === "string"
            ? (child.props as { id: string }).id
            : id;
    const control = isValidElement(child)
        ? cloneElement(child as ReactElement<{ id?: string }>, {
              id: controlId,
          })
        : child;

    return (
        <BaseField.Root data-slot="field" className={cn("field", className)}>
            <BaseField.Label
                data-slot="field-label"
                className="field-label"
                htmlFor={nativeLabel ? controlId : undefined}
                nativeLabel={nativeLabel}
                render={nativeLabel ? undefined : <div />}
            >
                {label}
            </BaseField.Label>
            {control}
            {description && (
                <BaseField.Description data-slot="field-description" className="field-description">
                    {description}
                </BaseField.Description>
            )}
        </BaseField.Root>
    );
}

function FieldGroup({ className, ...props }: ComponentProps<"div">) {
    return <div data-slot="field-group" className={cn("field-group", className)} {...props} />;
}

export { Field, FieldGroup };
