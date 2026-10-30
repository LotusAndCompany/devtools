import { AlertDialog } from "@/shared/components/ui/alert-dialog";
import { Button } from "@/shared/components/ui/button";
import { text } from "@/shared/tool-page";
import type { Locale } from "@/shared/types";

interface PhraseConfirmationDialogsProps {
    locale: Locale;
    title: string;
    busy: boolean;
    unsavedOpen: boolean;
    deleteOpen: boolean;
    onUnsavedOpenChange: (open: boolean) => void;
    onDeleteOpenChange: (open: boolean) => void;
    onDiscard: () => void;
    onSaveAndContinue: () => void;
    onConfirmDelete: () => void;
}

export function PhraseConfirmationDialogs({
    locale,
    title,
    busy,
    unsavedOpen,
    deleteOpen,
    onUnsavedOpenChange,
    onDeleteOpenChange,
    onDiscard,
    onSaveAndContinue,
    onConfirmDelete,
}: PhraseConfirmationDialogsProps) {
    return (
        <>
            <AlertDialog.Root open={unsavedOpen} onOpenChange={onUnsavedOpenChange}>
                <AlertDialog.Portal>
                    <AlertDialog.Backdrop className="dialog-backdrop" />
                    <AlertDialog.Viewport className="dialog-viewport">
                        <AlertDialog.Popup className="confirm-dialog">
                            <AlertDialog.Title className="dialog-title">
                                {text(locale, "Unsaved Changes", "未保存の変更")}
                            </AlertDialog.Title>
                            <AlertDialog.Description className="dialog-description">
                                {text(
                                    locale,
                                    "Save your changes before continuing?",
                                    "変更を保存して続行しますか？",
                                )}
                            </AlertDialog.Description>
                            <div className="dialog-footer">
                                <AlertDialog.Close render={<Button variant="outline" />}>
                                    {text(locale, "Cancel", "キャンセル")}
                                </AlertDialog.Close>
                                <Button variant="outline" onClick={onDiscard}>
                                    {text(locale, "Discard", "破棄")}
                                </Button>
                                <Button onClick={onSaveAndContinue} disabled={busy}>
                                    {text(locale, "Save", "保存")}
                                </Button>
                            </div>
                        </AlertDialog.Popup>
                    </AlertDialog.Viewport>
                </AlertDialog.Portal>
            </AlertDialog.Root>
            <AlertDialog.Root open={deleteOpen} onOpenChange={onDeleteOpenChange}>
                <AlertDialog.Portal>
                    <AlertDialog.Backdrop className="dialog-backdrop" />
                    <AlertDialog.Viewport className="dialog-viewport">
                        <AlertDialog.Popup className="confirm-dialog">
                            <AlertDialog.Title className="dialog-title">
                                {text(locale, "Confirm Delete", "削除の確認")}
                            </AlertDialog.Title>
                            <AlertDialog.Description className="dialog-description">
                                {text(
                                    locale,
                                    `Are you sure you want to delete "${title}"?`,
                                    `「${title}」を削除しますか？`,
                                )}
                            </AlertDialog.Description>
                            <div className="dialog-footer">
                                <AlertDialog.Close render={<Button variant="outline" />}>
                                    {text(locale, "Cancel", "キャンセル")}
                                </AlertDialog.Close>
                                <Button
                                    size="sm"
                                    className="danger-button"
                                    onClick={onConfirmDelete}
                                >
                                    {text(locale, "Delete", "削除")}
                                </Button>
                            </div>
                        </AlertDialog.Popup>
                    </AlertDialog.Viewport>
                </AlertDialog.Portal>
            </AlertDialog.Root>
        </>
    );
}
