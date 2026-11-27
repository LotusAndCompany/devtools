import {
    Select,
    SelectContent,
    SelectGroup,
    SelectItem,
    SelectTrigger,
    SelectValue,
} from "@/shared/components/ui/select";
import { cn } from "@/shared/lib/utils";

type SelectControlOption<Value extends string = string> = {
    value: Value;
    label: string;
};

type SelectControlProps<Value extends string = string> = {
    id?: string;
    "aria-label"?: string;
    "aria-labelledby"?: string;
    "aria-describedby"?: string;
    className?: string;
    disabled?: boolean;
    options: readonly SelectControlOption<Value>[];
    value?: Value;
    defaultValue?: Value;
    placeholder?: string;
    onValueChange?: (value: Value) => void;
};

function SelectControl<Value extends string>({
    id,
    "aria-label": ariaLabel,
    "aria-labelledby": labelledBy,
    "aria-describedby": describedBy,
    className,
    disabled,
    options,
    value,
    defaultValue,
    placeholder,
    onValueChange,
}: SelectControlProps<Value>) {
    return (
        <Select
            items={options.map(({ value: optionValue, label }) => ({ value: optionValue, label }))}
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
            <SelectTrigger
                id={id}
                aria-label={ariaLabel}
                aria-labelledby={labelledBy}
                aria-describedby={describedBy}
                className={cn("w-full", className)}
            >
                <SelectValue placeholder={placeholder} />
            </SelectTrigger>
            <SelectContent alignItemWithTrigger={false}>
                <SelectGroup>
                    {options.map((option) => (
                        <SelectItem key={option.value} value={option.value}>
                            {option.label}
                        </SelectItem>
                    ))}
                </SelectGroup>
            </SelectContent>
        </Select>
    );
}

export { SelectControl, type SelectControlOption };
