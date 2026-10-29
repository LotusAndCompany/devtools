import { Select as BaseSelect } from "@base-ui/react/select";
import { cn } from "@/lib/utils";

type SelectBoxOption = { value: string; label: string };

type SelectBoxProps = {
    id?: string;
    "aria-label"?: string;
    className?: string;
    disabled?: boolean;
    options: readonly SelectBoxOption[];
    value?: string;
    defaultValue?: string;
    placeholder?: string;
    onValueChange?: (value: string) => void;
};

function SelectBox({
    id,
    "aria-label": ariaLabel,
    className,
    disabled,
    options,
    value,
    defaultValue,
    placeholder,
    onValueChange,
}: SelectBoxProps) {
    return (
        <BaseSelect.Root
            items={options}
            value={value}
            defaultValue={defaultValue}
            disabled={disabled}
            onValueChange={(nextValue) => {
                if (typeof nextValue === "string") onValueChange?.(nextValue);
            }}
        >
            <BaseSelect.Trigger
                id={id}
                aria-label={ariaLabel}
                className={cn("select-trigger", className)}
            >
                <BaseSelect.Value placeholder={placeholder} />
                <BaseSelect.Icon className="select-chevron" aria-hidden="true" />
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
