import {
    AlertDialog,
    AlertDialogAction,
    AlertDialogCancel,
    AlertDialogContent,
    AlertDialogDescription,
    AlertDialogFooter,
    AlertDialogHeader,
    AlertDialogTitle,
} from "@/shared/components/ui/alert-dialog";
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
            <AlertDialog open={unsavedOpen} onOpenChange={onUnsavedOpenChange}>
                <AlertDialogContent>
                    <AlertDialogHeader>
                        <AlertDialogTitle>
                            {text(locale, "Unsaved Changes", "未保存の変更")}
                        </AlertDialogTitle>
                        <AlertDialogDescription>
                            {text(
                                locale,
                                "Save your changes before continuing?",
                                "変更を保存して続行しますか？",
                            )}
                        </AlertDialogDescription>
                    </AlertDialogHeader>
                    <AlertDialogFooter>
                        <AlertDialogCancel>
                            {text(locale, "Cancel", "キャンセル")}
                        </AlertDialogCancel>
                        <Button variant="outline" onClick={onDiscard}>
                            {text(locale, "Discard", "破棄")}
                        </Button>
                        <AlertDialogAction onClick={onSaveAndContinue} disabled={busy}>
                            {text(locale, "Save", "保存")}
                        </AlertDialogAction>
                    </AlertDialogFooter>
                </AlertDialogContent>
            </AlertDialog>
            <AlertDialog open={deleteOpen} onOpenChange={onDeleteOpenChange}>
                <AlertDialogContent>
                    <AlertDialogHeader>
                        <AlertDialogTitle>
                            {text(locale, "Confirm Delete", "削除の確認")}
                        </AlertDialogTitle>
                        <AlertDialogDescription>
                            {text(
                                locale,
                                `Are you sure you want to delete "${title}"?`,
                                `「${title}」を削除しますか？`,
                            )}
                        </AlertDialogDescription>
                    </AlertDialogHeader>
                    <AlertDialogFooter>
                        <AlertDialogCancel>
                            {text(locale, "Cancel", "キャンセル")}
                        </AlertDialogCancel>
                        <AlertDialogAction variant="destructive" onClick={onConfirmDelete}>
                            {text(locale, "Delete", "削除")}
                        </AlertDialogAction>
                    </AlertDialogFooter>
                </AlertDialogContent>
            </AlertDialog>
        </>
    );
}
