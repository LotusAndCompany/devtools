"use client";

import * as React from "react";
import { mergeProps } from "@base-ui/react/merge-props";
import { useRender } from "@base-ui/react/use-render";
import { cva, type VariantProps } from "class-variance-authority";
import { cn } from "cn";

function ItemGroup({ className, ...props }: React.ComponentProps<"div">) {
    return (
        <div
            data-slot="item-group"
            className={cn("flex w-full flex-col gap-2", className)}
            {...props}
        />
    );
}

const itemVariants = cva(
    "group/item flex w-full items-center gap-3 rounded-lg border text-left text-sm outline-none transition-colors focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:pointer-events-none disabled:opacity-50",
    {
        variants: {
            variant: {
                default: "border-transparent bg-transparent",
                outline: "border-border bg-transparent",
                muted: "border-transparent bg-muted",
            },
            size: {
                default: "min-h-14 p-4",
                sm: "min-h-12 gap-2.5 p-3",
                xs: "min-h-10 gap-2 p-2.5 text-xs",
            },
        },
        defaultVariants: {
            variant: "default",
            size: "default",
        },
    },
);

function Item({
    className,
    variant = "default",
    size = "default",
    render,
    ...props
}: useRender.ComponentProps<"div"> &
    React.ComponentProps<"div"> &
    VariantProps<typeof itemVariants>) {
    return useRender({
        defaultTagName: "div",
        props: mergeProps<"div">(
            {
                className: cn(itemVariants({ variant, size }), className),
            },
            props,
        ),
        render,
        state: {
            slot: "item",
            variant,
            size,
        },
    });
}

function ItemContent({ className, ...props }: React.ComponentProps<"div">) {
    return (
        <div
            data-slot="item-content"
            className={cn("flex min-w-0 flex-1 flex-col gap-1", className)}
            {...props}
        />
    );
}

function ItemActions({ className, ...props }: React.ComponentProps<"div">) {
    return (
        <div
            data-slot="item-actions"
            className={cn("flex shrink-0 items-center gap-2", className)}
            {...props}
        />
    );
}

function ItemTitle({ className, ...props }: React.ComponentProps<"div">) {
    return (
        <div
            data-slot="item-title"
            className={cn("min-w-0 truncate font-medium leading-snug", className)}
            {...props}
        />
    );
}

function ItemDescription({ className, ...props }: React.ComponentProps<"div">) {
    return (
        <div
            data-slot="item-description"
            className={cn("line-clamp-2 text-sm/relaxed text-muted-foreground", className)}
            {...props}
        />
    );
}

export { Item, ItemActions, ItemContent, ItemDescription, ItemGroup, ItemTitle };
