import {
    Children,
    cloneElement,
    isValidElement,
    useId,
    type ReactElement,
    type ReactNode,
} from "react";
import { Field, FieldDescription, FieldLabel, FieldTitle } from "@/shared/components/ui/field";

type FormFieldProps = {
    label: ReactNode;
    children: ReactNode;
    className?: string;
    nativeLabel?: boolean;
    description?: ReactNode;
};

type LabeledControlProps = {
    id?: string;
    "aria-labelledby"?: string;
    "aria-describedby"?: string;
};

function FormField({
    label,
    children,
    className,
    nativeLabel = true,
    description,
}: FormFieldProps) {
    const id = useId();
    const child = Children.only(children);
    const childProps = isValidElement(child) ? (child.props as LabeledControlProps) : undefined;
    const childId = typeof childProps?.id === "string" ? childProps.id : id;
    const labelId = `${id}-label`;
    const descriptionId = `${id}-description`;
    const labelledBy = nativeLabel
        ? childProps?.["aria-labelledby"]
        : [childProps?.["aria-labelledby"], labelId].filter(Boolean).join(" ");
    const describedBy = [childProps?.["aria-describedby"], description ? descriptionId : undefined]
        .filter(Boolean)
        .join(" ");
    const control = isValidElement(child)
        ? cloneElement(child as ReactElement<LabeledControlProps>, {
              id: childId,
              "aria-labelledby": labelledBy || undefined,
              "aria-describedby": describedBy || undefined,
          })
        : child;

    return (
        <Field className={className}>
            {nativeLabel ? (
                <FieldLabel id={labelId} htmlFor={childId}>
                    {label}
                </FieldLabel>
            ) : (
                <FieldTitle id={labelId}>{label}</FieldTitle>
            )}
            {control}
            {description && <FieldDescription id={descriptionId}>{description}</FieldDescription>}
        </Field>
    );
}

export { FormField, type FormFieldProps };
