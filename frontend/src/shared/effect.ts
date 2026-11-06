import { Effect, Fiber } from "effect";

export function runUiEffect<A, E>(effect: Effect.Effect<A, E>): void {
  void Effect.runPromise(effect).catch(() => undefined);
}

export function forkUiEffect<A, E>(effect: Effect.Effect<A, E>): () => void {
  const fiber = Effect.runFork(effect);
  return () => {
    Effect.runFork(Fiber.interrupt(fiber));
  };
}
