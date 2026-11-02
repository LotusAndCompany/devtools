import { Effect } from "effect";
import {
  NativeOperationError,
  OperationCancelledError,
  type OperationName,
  type OperationOutput,
  type OperationRequest,
} from "@/shared/bridge/types";
import type { NativeBridge } from "@/shared/bridge/native-bridge";

export function runNativeOperation<Name extends OperationName>(
  native: NativeBridge,
  request: OperationRequest<Name>,
  cancellationMessage: string,
  onPendingChange?: (requestId: string, isPending: boolean, cancel: () => void) => void,
): Effect.Effect<OperationOutput<Name>, NativeOperationError | OperationCancelledError> {
  return Effect.callback<OperationOutput<Name>, NativeOperationError | OperationCancelledError>(
    (resume) => {
      let settled = false;
      let unsubscribe: (() => void) | undefined;
      const cancel = () => {
        try {
          native.cancelOperation(request.requestId);
        } catch {
          // The native operation may already have completed while the UI is closing.
        }
      };
      const cleanup = () => {
        unsubscribe?.();
        unsubscribe = undefined;
        onPendingChange?.(request.requestId, false, cancel);
      };
      const finish = (
        effect: Effect.Effect<
          OperationOutput<Name>,
          NativeOperationError | OperationCancelledError
        >,
      ) => {
        if (settled) return;
        settled = true;
        cleanup();
        resume(effect);
      };

      onPendingChange?.(request.requestId, true, cancel);
      try {
        unsubscribe = native.subscribe((event) => {
          if (event.type === "completed" && event.requestId === request.requestId) {
            // Transport results have no runtime schema metadata; this is the typed boundary.
            finish(Effect.succeed(event.result as OperationOutput<Name>));
          } else if (event.type === "failed" && event.requestId === request.requestId) {
            finish(
              Effect.fail(
                new NativeOperationError(event.error.message || event.error.code, event.error.code),
              ),
            );
          } else if (event.type === "cancelled" && event.requestId === request.requestId) {
            finish(Effect.fail(new OperationCancelledError(cancellationMessage)));
          }
        });
        native.startOperation(request as OperationRequest);
      } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        finish(Effect.fail(new NativeOperationError(message, "START_FAILED")));
      }

      return Effect.sync(() => {
        if (settled) return;
        settled = true;
        cleanup();
        cancel();
      });
    },
  );
}
