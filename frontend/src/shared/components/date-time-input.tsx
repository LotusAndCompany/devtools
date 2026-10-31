import { useState } from "react";
import { enUS, ja } from "date-fns/locale";
import { CalendarDays } from "lucide-react";
import { Button } from "@/shared/components/ui/button";
import { Calendar } from "@/shared/components/ui/calendar";
import { Input } from "@/shared/components/ui/input";
import {
    Popover,
    PopoverContent,
    PopoverTitle,
    PopoverTrigger,
} from "@/shared/components/ui/popover";
import type { Locale } from "@/shared/types";

type DateParts = {
    year: number;
    month: number;
    day: number;
    time: string;
};

function parseDateTime(value: string): DateParts | null {
    const match = /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2})$/.exec(value);
    if (!match) return null;

    const [, yearText, monthText, dayText, hourText, minuteText] = match;
    const year = Number(yearText);
    const month = Number(monthText) - 1;
    const day = Number(dayText);
    const date = new Date(year, month, day);
    if (
        date.getFullYear() !== year ||
        date.getMonth() !== month ||
        date.getDate() !== day ||
        Number(hourText) > 23 ||
        Number(minuteText) > 59
    ) {
        return null;
    }
    return { year, month, day, time: `${hourText}:${minuteText}` };
}

function dateValue(date: Date, time: string) {
    const year = String(date.getFullYear()).padStart(4, "0");
    const month = String(date.getMonth() + 1).padStart(2, "0");
    const day = String(date.getDate()).padStart(2, "0");
    return `${year}-${month}-${day}T${time}`;
}

function monthStart(date: Date) {
    return new Date(date.getFullYear(), date.getMonth(), 1);
}

interface DateTimeInputProps {
    locale: Locale;
    value: string;
    onValueChange: (value: string) => void;
    id?: string;
    placeholder?: string;
    "aria-label"?: string;
    "aria-labelledby"?: string;
    "aria-describedby"?: string;
}

function DateTimeInput({
    locale,
    value,
    onValueChange,
    id,
    placeholder,
    "aria-label": ariaLabel,
    "aria-labelledby": labelledBy,
    "aria-describedby": describedBy,
}: DateTimeInputProps) {
    const selected = parseDateTime(value);
    const selectedDate = selected
        ? new Date(selected.year, selected.month, selected.day)
        : undefined;
    const [open, setOpen] = useState(false);
    const [visibleMonth, setVisibleMonth] = useState(() => monthStart(selectedDate ?? new Date()));

    return (
        <div data-slot="date-time-input" className="date-time-input">
            <Input
                id={id}
                value={value}
                aria-label={ariaLabel}
                aria-labelledby={labelledBy}
                aria-describedby={describedBy}
                placeholder={placeholder ?? "YYYY-MM-DDTHH:mm"}
                onChange={(event) => onValueChange(event.target.value)}
                autoComplete="off"
                spellCheck={false}
            />
            <Popover
                open={open}
                onOpenChange={(nextOpen) => {
                    if (nextOpen) {
                        const current = parseDateTime(value);
                        setVisibleMonth(
                            monthStart(
                                current
                                    ? new Date(current.year, current.month, current.day)
                                    : new Date(),
                            ),
                        );
                    }
                    setOpen(nextOpen);
                }}
            >
                <PopoverTrigger
                    aria-label={locale === "ja" ? "カレンダーを開く" : "Open calendar"}
                    title={locale === "ja" ? "カレンダーを開く" : "Open calendar"}
                    render={
                        <Button
                            variant="outline"
                            size="icon-sm"
                            type="button"
                            className="date-time-trigger"
                        >
                            <CalendarDays aria-hidden="true" />
                        </Button>
                    }
                />
                <PopoverContent
                    data-calendar-popup=""
                    className="date-time-popup w-[var(--calendar-popup-width)] gap-0 p-0"
                    side="bottom"
                    align="end"
                    sideOffset={6}
                >
                    <PopoverTitle className="sr-only">
                        {locale === "ja" ? "日付を選択" : "Select date"}
                    </PopoverTitle>
                    <Calendar
                        mode="single"
                        aria-label={locale === "ja" ? "カレンダー" : "Calendar"}
                        className="date-time-calendar"
                        locale={locale === "ja" ? ja : enUS}
                        month={visibleMonth}
                        onMonthChange={setVisibleMonth}
                        selected={selectedDate}
                        onSelect={(date) => {
                            if (!date) return;
                            onValueChange(dateValue(date, selected?.time ?? "00:00"));
                            setOpen(false);
                        }}
                    />
                </PopoverContent>
            </Popover>
        </div>
    );
}

export { DateTimeInput };
