import type { ComponentProps } from "react";
import { cn } from "@/shared/lib/utils";

type CardElement = "aside" | "article" | "div" | "section";
type CardProps = ComponentProps<"div"> & { as?: CardElement };

function Card({ as = "div", className, ...props }: CardProps) {
    const sharedProps = { "data-slot": "card", className: cn("ui-card", className), ...props };
    switch (as) {
        case "aside":
            return <aside {...sharedProps} />;
        case "article":
            return <article {...sharedProps} />;
        case "section":
            return <section {...sharedProps} />;
        default:
            return <div {...sharedProps} />;
    }
}

function CardHeader({ className, ...props }: ComponentProps<"div">) {
    return <div data-slot="card-header" className={cn("card-header", className)} {...props} />;
}

function CardTitle({ className, ...props }: ComponentProps<"h2">) {
    return <h2 data-slot="card-title" className={cn("card-title", className)} {...props} />;
}

function CardDescription({ className, ...props }: ComponentProps<"p">) {
    return (
        <p data-slot="card-description" className={cn("card-description", className)} {...props} />
    );
}

function CardContent({ className, ...props }: ComponentProps<"div">) {
    return <div data-slot="card-content" className={cn("card-content", className)} {...props} />;
}

function CardFooter({ className, ...props }: ComponentProps<"div">) {
    return <div data-slot="card-footer" className={cn("card-footer", className)} {...props} />;
}

export { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle, type CardProps };
