import { Select as BaseSelect } from "@base-ui/react/select";
import { cn } from "@/shared/lib/utils";

type SelectBoxOption<Value extends string = string> = { value: Value; label: string };

type SelectBoxProps<Value extends string = string> = {
    id?: string;
    "aria-label"?: string;
    className?: string;
    disabled?: boolean;
    options: readonly SelectBoxOption<Value>[];
    value?: Value;
    defaultValue?: Value;
    placeholder?: string;
    onValueChange?: (value: Value) => void;
};

function SelectBox<Value extends string>({
    id,
    "aria-label": ariaLabel,
    className,
    disabled,
    options,
    value,
    defaultValue,
    placeholder,
    onValueChange,
}: SelectBoxProps<Value>) {
    return (
        <BaseSelect.Root
            items={options}
            value={value}
            defaultValue={defaultValue}
            disabled={disabled}
            onValueChange={(nextValue) => {
                if (
                    typeof nextValue === "string" &&
                    options.some((option) => option.value === nextValue)
                ) {
                    onValueChange?.(nextValue as Value);
                }
            }}
        >
            <BaseSelect.Trigger
                id={id}
                aria-label={ariaLabel}
                className={cn("select-trigger", className)}
            >
                <BaseSelect.Value placeholder={placeholder} />
                <BaseSelect.Icon className="material-symbols select-chevron" aria-hidden="true">
                    keyboard_arrow_down
                </BaseSelect.Icon>
            </BaseSelect.Trigger>
            <BaseSelect.Portal>
                <BaseSelect.Positioner
                    className="select-positioner"
                    sideOffset={4}
                    alignItemWithTrigger={false}
                >
                    <BaseSelect.Popup className="select-popup">
                        <BaseSelect.List className="select-list">
                            <BaseSelect.Group>
                                {options.map((option) => (
                                    <BaseSelect.Item
                                        key={option.value}
                                        value={option.value}
                                        className="select-item"
                                    >
                                        <BaseSelect.ItemIndicator
                                            className="select-item-indicator"
                                            aria-hidden="true"
                                        >
                                            ✓
                                        </BaseSelect.ItemIndicator>
                                        <BaseSelect.ItemText>{option.label}</BaseSelect.ItemText>
                                    </BaseSelect.Item>
                                ))}
                            </BaseSelect.Group>
                        </BaseSelect.List>
                    </BaseSelect.Popup>
                </BaseSelect.Positioner>
            </BaseSelect.Portal>
        </BaseSelect.Root>
    );
}

export { SelectBox, type SelectBoxOption };
