import type { ReactNode } from "react";
import { TriangleAlertIcon, XIcon } from "lucide-react";
import { Alert, AlertAction, AlertDescription, AlertTitle } from "@/shared/components/ui/alert";
import { Button } from "@/shared/components/ui/button";

type FeedbackAlertProps = {
    title: string;
    description: ReactNode;
    onDismiss?: () => void;
    dismissLabel?: string;
};

export function FeedbackAlert({
    title,
    description,
    onDismiss,
    dismissLabel = "Dismiss",
}: FeedbackAlertProps) {
    return (
        <Alert className="feedback-message" role="alert">
            <TriangleAlertIcon aria-hidden="true" />
            <AlertTitle>{title}</AlertTitle>
            <AlertDescription>{description}</AlertDescription>
            {onDismiss && (
                <AlertAction>
                    <Button
                        aria-label={dismissLabel}
                        onClick={onDismiss}
                        size="icon-sm"
                        variant="ghost"
                    >
                        <XIcon aria-hidden="true" data-icon="inline-start" />
                    </Button>
                </AlertAction>
            )}
        </Alert>
    );
}
