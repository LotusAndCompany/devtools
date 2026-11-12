import type { BridgeEvent, OperationRequest } from "@/shared/bridge/types";

export class BridgeConnectionError extends Error {
    readonly _tag = "BridgeConnectionError";

    constructor(message: string) {
        super(message);
        this.name = this._tag;
    }
}

export type NativeBridge = {
    readonly contractVersion: string;
    startOperation: (request: OperationRequest) => void;
    cancelOperation: (requestId: string) => void;
    subscribe: (handler: (event: BridgeEvent) => void) => () => void;
};
