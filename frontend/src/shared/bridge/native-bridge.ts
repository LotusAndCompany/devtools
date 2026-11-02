import type { BridgeEvent, OperationRequest } from "@/shared/bridge/types";

export type NativeBridge = {
  readonly contractVersion: string;
  startOperation: (request: OperationRequest) => void;
  cancelOperation: (requestId: string) => void;
  subscribe: (handler: (event: BridgeEvent) => void) => () => void;
};
