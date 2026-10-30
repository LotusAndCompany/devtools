import { useCallback, useEffect, useRef, useState } from "react";
import { Effect } from "effect";
import {
  BRIDGE_CONTRACT_VERSION,
  NativeOperationError,
  type ApplicationSettings,
  type ImageTakeDroppedResult,
  type OperationInput,
  type OperationName,
  type OperationRequest,
  type OperationRunner,
  type BridgeEvent,
} from "@/shared/bridge/types";
import { connectNativeBridge, runNativeOperation, type NativeBridge } from "@/platform/qt/bridge";
import { forkUiEffect, runUiEffect } from "@/shared/effect";
import { toolForPage, type PageId } from "./tool-registry";
import type { DroppedImage, Locale } from "@/shared/types";

type UseNativeBridgeOptions = {
  locale: Locale;
  activePage: PageId;
  onNavigate: (route: string) => void;
  onDroppedImage: (result: ImageTakeDroppedResult) => void;
  onStartupSettings: (settings: Partial<ApplicationSettings>) => void;
};

export function useNativeBridge({
  locale,
  activePage,
  onNavigate,
  onDroppedImage,
  onStartupSettings,
}: UseNativeBridgeOptions) {
  const [bridge, setBridge] = useState<NativeBridge | null>(null);
  const [pendingCount, setPendingCount] = useState(0);
  const pending = useRef(new Map<string, () => void>());
  const localeRef = useRef(locale);

  useEffect(() => {
    localeRef.current = locale;
  }, [locale]);

  const runWithBridge = useCallback(
    <Name extends OperationName>(
      native: NativeBridge,
      operation: Name,
      payload: OperationInput<Name>,
    ) => {
      const requestId = crypto.randomUUID().replaceAll("-", "");
      const request: OperationRequest<Name> = {
        contractVersion: BRIDGE_CONTRACT_VERSION,
        requestId,
        operation,
        payload,
      };
      return runNativeOperation(
        native,
        request,
        localeRef.current === "ja" ? "処理をキャンセルしました。" : "Operation cancelled.",
        (id, isPending, cancel) => {
          if (isPending) pending.current.set(id, cancel);
          else pending.current.delete(id);
          setPendingCount(pending.current.size);
        },
      );
    },
    [],
  );

  useEffect(() => {
    let mounted = true;
    let unsubscribe: (() => void) | undefined;
    let native: NativeBridge | null = null;
    const onEvent = (event: BridgeEvent) => {
      if (event.type === "imageDropAvailable") {
        if (native) {
          runUiEffect(
            runWithBridge(native, "imageTakeDropped", {}).pipe(
              Effect.tap((result) => Effect.sync(() => onDroppedImage(result))),
              Effect.catch(() => Effect.void),
            ),
          );
        }
        return;
      }
      if (event.type === "navigate") onNavigate(event.route);
    };
    const setup = Effect.gen(function* () {
      const connected = yield* connectNativeBridge();
      if (!mounted) return;
      native = connected;
      unsubscribe = connected.subscribe(onEvent);
      yield* Effect.sync(() => setBridge(connected));

      const [droppedImageResult, settings] = yield* Effect.all(
        [
          runWithBridge(connected, "imageTakeDropped", {}).pipe(
            Effect.catch(() => Effect.succeed<ImageTakeDroppedResult>({ found: false })),
          ),
          runWithBridge(connected, "getSettings", {}).pipe(
            Effect.catch(() => Effect.succeed<Partial<ApplicationSettings>>({})),
          ),
        ],
        { concurrency: "unbounded" },
      );
      if (mounted && droppedImageResult.found) {
        yield* Effect.sync(() => onDroppedImage(droppedImageResult));
      }
      if (mounted) yield* Effect.sync(() => onStartupSettings(settings));
    }).pipe(Effect.catch(() => Effect.void));
    const interruptSetup = forkUiEffect(setup);
    return () => {
      mounted = false;
      unsubscribe?.();
      interruptSetup();
    };
    // oxlint-disable-next-line react/exhaustive-effect-dependencies -- runWithBridge is stable
  }, [onDroppedImage, onNavigate, onStartupSettings, runWithBridge]);

  const run: OperationRunner = useCallback(
    <Name extends OperationName>(operation: Name, payload: OperationInput<Name>) => {
      if (!bridge) {
        return Effect.fail(
          new NativeOperationError(
            locale === "ja"
              ? "ネイティブ連携に接続していません。"
              : "Native services are not connected.",
            "NOT_CONNECTED",
          ),
        );
      }
      return runWithBridge(bridge, operation, payload);
    },
    [bridge, locale, runWithBridge],
  );

  useEffect(() => {
    const nativeId = toolForPage(activePage)?.nativeId;
    if (!bridge || !nativeId) return;
    runUiEffect(
      runWithBridge(bridge, "saveLastUsedTool", { toolId: nativeId }).pipe(
        Effect.catch(() => Effect.void),
      ),
    );
    // oxlint-disable-next-line react/exhaustive-effect-dependencies -- runWithBridge is stable
  }, [activePage, bridge, runWithBridge]);

  const cancelAll = useCallback(() => {
    for (const cancel of pending.current.values()) cancel();
  }, []);

  return { run, pendingCount, cancelAll };
}

export function useDroppedImage(
  openPage: (page: PageId) => void,
): [DroppedImage | null, (result: ImageTakeDroppedResult) => void, (id: string) => void] {
  const [droppedImage, setDroppedImage] = useState<DroppedImage | null>(null);
  const accept = useCallback(
    (result: ImageTakeDroppedResult) => {
      if (!result.found || !result.imageDataUrl) return;
      openPage("image");
      setDroppedImage({
        id: crypto.randomUUID(),
        imageDataUrl: result.imageDataUrl,
        fileName: result.fileName,
        width: result.width,
        height: result.height,
      });
    },
    [openPage],
  );
  const clear = useCallback((id: string) => {
    setDroppedImage((current) => (current?.id === id ? null : current));
  }, []);
  return [droppedImage, accept, clear];
}
