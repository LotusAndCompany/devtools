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
                data-slot="select-trigger"
                id={id}
                aria-label={ariaLabel}
                className={cn("select-trigger", className)}
            >
                <BaseSelect.Value placeholder={placeholder} />
                <BaseSelect.Icon
                    data-slot="select-icon"
                    className="select-chevron"
                    aria-hidden="true"
                >
                    <svg viewBox="0 0 16 16" fill="none" aria-hidden="true">
                        <path d="m4.5 6 3.5 3.5L11.5 6" />
                    </svg>
                </BaseSelect.Icon>
            </BaseSelect.Trigger>
            <BaseSelect.Portal>
                <BaseSelect.Positioner
                    data-slot="select-positioner"
                    className="select-positioner"
                    sideOffset={4}
                    alignItemWithTrigger={false}
                >
                    <BaseSelect.Popup data-slot="select-popup" className="select-popup">
                        <BaseSelect.List data-slot="select-list" className="select-list">
                            {options.map((option) => (
                                <BaseSelect.Item
                                    data-slot="select-item"
                                    key={option.value}
                                    value={option.value}
                                    className="select-item"
                                >
                                    <BaseSelect.ItemIndicator
                                        data-slot="select-item-indicator"
                                        className="select-item-indicator"
                                        aria-hidden="true"
                                    >
                                        <svg viewBox="0 0 16 16" fill="none">
                                            <path d="m3 8 3.25 3.25L13 4.5" />
                                        </svg>
                                    </BaseSelect.ItemIndicator>
                                    <BaseSelect.ItemText>{option.label}</BaseSelect.ItemText>
                                </BaseSelect.Item>
                            ))}
                        </BaseSelect.List>
                    </BaseSelect.Popup>
                </BaseSelect.Positioner>
            </BaseSelect.Portal>
        </BaseSelect.Root>
    );
}

export { SelectBox, type SelectBoxOption };
