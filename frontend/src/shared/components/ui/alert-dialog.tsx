import type { ComponentProps } from "react";
import { AlertDialog as BaseAlertDialog } from "@base-ui/react/alert-dialog";

function AlertDialogBackdrop(props: ComponentProps<typeof BaseAlertDialog.Backdrop>) {
    return <BaseAlertDialog.Backdrop {...props} data-slot="alert-dialog-backdrop" />;
}

function AlertDialogViewport(props: ComponentProps<typeof BaseAlertDialog.Viewport>) {
    return <BaseAlertDialog.Viewport {...props} data-slot="alert-dialog-viewport" />;
}

function AlertDialogPopup(props: ComponentProps<typeof BaseAlertDialog.Popup>) {
    return <BaseAlertDialog.Popup {...props} data-slot="alert-dialog-popup" />;
}

function AlertDialogTitle(props: ComponentProps<typeof BaseAlertDialog.Title>) {
    return <BaseAlertDialog.Title {...props} data-slot="alert-dialog-title" />;
}

function AlertDialogDescription(props: ComponentProps<typeof BaseAlertDialog.Description>) {
    return <BaseAlertDialog.Description {...props} data-slot="alert-dialog-description" />;
}

const AlertDialog = {
    Root: BaseAlertDialog.Root,
    Trigger: BaseAlertDialog.Trigger,
    Portal: BaseAlertDialog.Portal,
    Backdrop: AlertDialogBackdrop,
    Viewport: AlertDialogViewport,
    Popup: AlertDialogPopup,
    Title: AlertDialogTitle,
    Description: AlertDialogDescription,
    Close: BaseAlertDialog.Close,
};

export { AlertDialog };
