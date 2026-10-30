import { Button } from "@/shared/components/ui/button";
import { AlertDialog } from "@/shared/components/ui/alert-dialog";

type MessageDialogProps = {
    open: boolean;
    onOpenChange: (open: boolean) => void;
    title: string;
    description: string;
    closeLabel: string;
};

function MessageDialog({ open, onOpenChange, title, description, closeLabel }: MessageDialogProps) {
    return (
        <AlertDialog.Root open={open} onOpenChange={onOpenChange}>
            <AlertDialog.Portal>
                <AlertDialog.Backdrop className="dialog-backdrop" />
                <AlertDialog.Viewport className="dialog-viewport">
                    <AlertDialog.Popup className="message-dialog">
                        <AlertDialog.Title className="dialog-title">{title}</AlertDialog.Title>
                        <AlertDialog.Description className="dialog-description">
                            {description}
                        </AlertDialog.Description>
                        <div className="dialog-footer">
                            <AlertDialog.Close render={<Button />}>{closeLabel}</AlertDialog.Close>
                        </div>
                    </AlertDialog.Popup>
                </AlertDialog.Viewport>
            </AlertDialog.Portal>
        </AlertDialog.Root>
    );
}

export { MessageDialog };
