import { Popover as BasePopover } from "@base-ui/react/popover";
import { useState } from "react";
import { Button } from "@/shared/components/ui/button";
import { Input } from "@/shared/components/ui/input";
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

function calendarDays(month: Date) {
    const firstDay = month.getDay();
    const mondayOffset = (firstDay + 6) % 7;
    const start = new Date(month.getFullYear(), month.getMonth(), 1 - mondayOffset);
    return Array.from({ length: 42 }, (_, index) => {
        const date = new Date(start);
        date.setDate(start.getDate() + index);
        return date;
    });
}

function sameDate(left: Date, right: Date) {
    return (
        left.getFullYear() === right.getFullYear() &&
        left.getMonth() === right.getMonth() &&
        left.getDate() === right.getDate()
    );
}

interface DateTimeInputProps {
    locale: Locale;
    value: string;
    onValueChange: (value: string) => void;
    id?: string;
    placeholder?: string;
    "aria-label"?: string;
}

function DateTimeInput({
    locale,
    value,
    onValueChange,
    id,
    placeholder,
    "aria-label": ariaLabel,
}: DateTimeInputProps) {
    const selected = parseDateTime(value);
    const [open, setOpen] = useState(false);
    const [visibleMonth, setVisibleMonth] = useState(() =>
        monthStart(selected ? new Date(selected.year, selected.month, selected.day) : new Date()),
    );
    const language = locale === "ja" ? "ja-JP" : "en-US";
    const weekdays =
        locale === "ja"
            ? ["月", "火", "水", "木", "金", "土", "日"]
            : ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
    const days = calendarDays(visibleMonth);
    const [today] = useState(() => new Date());

    return (
        <div data-slot="date-time-input" className="date-time-input">
            <Input
                id={id}
                value={value}
                aria-label={ariaLabel}
                placeholder={placeholder ?? "YYYY-MM-DDTHH:mm"}
                onChange={(event) => onValueChange(event.target.value)}
                autoComplete="off"
                spellCheck={false}
            />
            <BasePopover.Root
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
                <BasePopover.Trigger
                    aria-label={locale === "ja" ? "カレンダーを開く" : "Open calendar"}
                    title={locale === "ja" ? "カレンダーを開く" : "Open calendar"}
                    render={
                        <Button
                            variant="outline"
                            size="sm"
                            type="button"
                            className="date-time-trigger"
                        >
                            <span className="material-symbols" aria-hidden="true">
                                calendar_month
                            </span>
                        </Button>
                    }
                />
                <BasePopover.Portal>
                    <BasePopover.Positioner
                        data-slot="calendar-positioner"
                        className="date-time-positioner"
                        side="bottom"
                        align="end"
                        sideOffset={6}
                    >
                        <BasePopover.Popup data-slot="calendar-popup" className="date-time-popup">
                            <div data-slot="calendar-header" className="date-time-calendar-header">
                                <BasePopover.Title
                                    data-slot="calendar-month"
                                    className="date-time-month"
                                >
                                    {new Intl.DateTimeFormat(language, {
                                        year: "numeric",
                                        month: "long",
                                    }).format(visibleMonth)}
                                </BasePopover.Title>
                                <div data-slot="calendar-nav" className="date-time-calendar-nav">
                                    <Button
                                        variant="ghost"
                                        size="sm"
                                        type="button"
                                        aria-label={locale === "ja" ? "前の月" : "Previous month"}
                                        onClick={() =>
                                            setVisibleMonth(
                                                (current) =>
                                                    new Date(
                                                        current.getFullYear(),
                                                        current.getMonth() - 1,
                                                        1,
                                                    ),
                                            )
                                        }
                                    >
                                        <span className="material-symbols" aria-hidden="true">
                                            chevron_left
                                        </span>
                                    </Button>
                                    <Button
                                        variant="ghost"
                                        size="sm"
                                        type="button"
                                        aria-label={locale === "ja" ? "次の月" : "Next month"}
                                        onClick={() =>
                                            setVisibleMonth(
                                                (current) =>
                                                    new Date(
                                                        current.getFullYear(),
                                                        current.getMonth() + 1,
                                                        1,
                                                    ),
                                            )
                                        }
                                    >
                                        <span className="material-symbols" aria-hidden="true">
                                            chevron_right
                                        </span>
                                    </Button>
                                </div>
                            </div>
                            <div
                                data-slot="calendar-grid"
                                className="date-time-calendar-grid"
                                role="grid"
                            >
                                {weekdays.map((weekday, index) => (
                                    <span
                                        key={`${weekday}-${index}`}
                                        data-slot="calendar-weekday"
                                        className="date-time-weekday"
                                        role="columnheader"
                                    >
                                        {weekday}
                                    </span>
                                ))}
                                {days.map((date) => {
                                    const selectedDate =
                                        selected &&
                                        date.getFullYear() === selected.year &&
                                        date.getMonth() === selected.month &&
                                        date.getDate() === selected.day;
                                    const inMonth = date.getMonth() === visibleMonth.getMonth();
                                    return (
                                        <button
                                            key={date.toISOString()}
                                            type="button"
                                            data-slot="calendar-day"
                                            data-outside-month={!inMonth ? "" : undefined}
                                            data-selected={selectedDate ? "" : undefined}
                                            role="gridcell"
                                            aria-pressed={Boolean(selectedDate)}
                                            aria-current={
                                                sameDate(date, today) ? "date" : undefined
                                            }
                                            aria-label={new Intl.DateTimeFormat(language, {
                                                dateStyle: "full",
                                            }).format(date)}
                                            onClick={() => {
                                                onValueChange(
                                                    dateValue(date, selected?.time ?? "00:00"),
                                                );
                                                setOpen(false);
                                            }}
                                        >
                                            {date.getDate()}
                                        </button>
                                    );
                                })}
                            </div>
                        </BasePopover.Popup>
                    </BasePopover.Positioner>
                </BasePopover.Portal>
            </BasePopover.Root>
        </div>
    );
}

export { DateTimeInput };
