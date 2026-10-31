import type { ComponentProps } from "react";
import { Dialog as BaseDialog } from "@base-ui/react/dialog";

function DialogBackdrop(props: ComponentProps<typeof BaseDialog.Backdrop>) {
    return <BaseDialog.Backdrop {...props} data-slot="dialog-backdrop" />;
}

function DialogViewport(props: ComponentProps<typeof BaseDialog.Viewport>) {
    return <BaseDialog.Viewport {...props} data-slot="dialog-viewport" />;
}

function DialogPopup(props: ComponentProps<typeof BaseDialog.Popup>) {
    return <BaseDialog.Popup {...props} data-slot="dialog-popup" />;
}

function DialogTitle(props: ComponentProps<typeof BaseDialog.Title>) {
    return <BaseDialog.Title {...props} data-slot="dialog-title" />;
}

function DialogDescription(props: ComponentProps<typeof BaseDialog.Description>) {
    return <BaseDialog.Description {...props} data-slot="dialog-description" />;
}

const Dialog = {
    Root: BaseDialog.Root,
    Trigger: BaseDialog.Trigger,
    Portal: BaseDialog.Portal,
    Backdrop: DialogBackdrop,
    Viewport: DialogViewport,
    Popup: DialogPopup,
    Title: DialogTitle,
    Description: DialogDescription,
    Close: BaseDialog.Close,
};

export { Dialog };
