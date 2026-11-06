import { Effect } from "effect";
import {
  NativeOperationError,
  type OperationInput,
  type OperationName,
  type OperationOutput,
  type OperationRunner,
} from "@/shared/bridge/types";

type OperationHandlers = {
  [Name in OperationName]?: (input: OperationInput<Name>) => OperationOutput<Name>;
};

export function createOperationRunner(handlers: OperationHandlers): OperationRunner {
  return <Name extends OperationName>(operation: Name, input: OperationInput<Name>) => {
    const handler = handlers[operation] as
      | ((payload: OperationInput<Name>) => OperationOutput<Name>)
      | undefined;
    if (!handler) {
      return Effect.fail(new NativeOperationError(`Unexpected operation: ${operation}`, "TEST"));
    }
    return Effect.succeed(handler(input));
  };
}
