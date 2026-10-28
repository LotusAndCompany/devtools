import { useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import {
  BRIDGE_CONTRACT_VERSION,
  connectNativeBridge,
  type BridgeEvent,
  type NativeBridge,
  type OperationName,
  type OperationRequest,
} from "@/bridge";

const MAX_INPUT_BYTES = 1024 * 1024;
const isJapanese = navigator.language.toLowerCase().startsWith("ja");

const copy = {
  en: {
    eyebrow: "NATIVE INTEGRATION",
    title: "React, backed by C++",
    intro: "A real Qt WebChannel bridge formats JSON and opens or saves local text files.",
    connected: "Native bridge connected",
    connecting: "Connecting to native services…",
    disconnected: "Open this screen inside DevTools to use native services.",
    contract: "Contract",
    input: "Input JSON",
    output: "Formatted output",
    inputHint: "UTF-8 input is limited to 1 MiB per operation.",
    outputHint: "Output is returned as plain text. No user content is rendered as HTML.",
    placeholder: '{\n  "name": "DevTools",\n  "native": true\n}',
    outputPlaceholder: "Formatted JSON will appear here",
    indentation: "Indent",
    format: "Format JSON",
    open: "Open file",
    save: "Save output",
    cancel: "Cancel operation",
    ready: "Ready for a native operation",
    working: "Working with native C++…",
    complete: "Operation completed",
    cancelled: "Operation cancelled",
    invalidInput: "Check the JSON and try again.",
    sizeLimit: "This operation exceeds the 1 MiB input or 4 MiB output limit.",
    invalidEncoding: "The selected file is not valid UTF-8.",
    permission: "DevTools could not access that file.",
    bridgeError: "The native operation failed.",
    bytes: "bytes",
    formatError: "Enter JSON to format.",
    indent2: "2 spaces",
    indent4: "4 spaces",
    indentTab: "Tab",
    indentMinified: "Minified",
  },
  ja: {
    eyebrow: "ネイティブ連携",
    title: "ReactからC++を実行",
    intro: "Qt WebChannel経由でJSONを整形し、ローカルファイルを開いたり保存したりします。",
    connected: "ネイティブブリッジに接続済み",
    connecting: "ネイティブサービスに接続中…",
    disconnected: "ネイティブ機能を使うにはDevTools内で開いてください。",
    contract: "契約バージョン",
    input: "入力JSON",
    output: "整形結果",
    inputHint: "入力はUTF-8で1操作あたり1 MiBまでです。",
    outputHint: "結果はテキストとして表示します。入力内容をHTMLとして描画しません。",
    placeholder: '{\n  "name": "DevTools",\n  "native": true\n}',
    outputPlaceholder: "整形したJSONをここに表示します",
    indentation: "インデント",
    format: "JSONを整形",
    open: "ファイルを開く",
    save: "結果を保存",
    cancel: "処理をキャンセル",
    ready: "ネイティブ処理を実行できます",
    working: "C++で処理中…",
    complete: "処理が完了しました",
    cancelled: "処理をキャンセルしました",
    invalidInput: "JSONを確認して、もう一度実行してください。",
    sizeLimit: "入力1 MiBまたは出力4 MiBの上限を超えています。",
    invalidEncoding: "選択したファイルは有効なUTF-8形式ではありません。",
    permission: "DevToolsからファイルにアクセスできませんでした。",
    bridgeError: "ネイティブ処理に失敗しました。",
    bytes: "バイト",
    formatError: "整形するJSONを入力してください。",
    indent2: "2スペース",
    indent4: "4スペース",
    indentTab: "タブ",
    indentMinified: "最小出力",
  },
}[isJapanese ? "ja" : "en"];

type Status = { phase: "idle" | "running" | "done" | "failed" | "cancelled"; percent: number; text: string };

function errorMessage(code: string, fallback: string) {
  if (code === "SIZE_LIMIT_EXCEEDED") return copy.sizeLimit;
  if (code === "INVALID_ENCODING") return copy.invalidEncoding;
  if (code === "PERMISSION_DENIED") return copy.permission;
  if (code === "INVALID_INPUT") return fallback || copy.invalidInput;
  return fallback || copy.bridgeError;
}

function makeRequestId() {
  return crypto.randomUUID().replaceAll("-", "");
}

function App() {
  const [bridge, setBridge] = useState<NativeBridge | null>(null);
  const [connectionError, setConnectionError] = useState("");
  const [input, setInput] = useState(copy.placeholder);
  const [output, setOutput] = useState("");
  const [indentation, setIndentation] = useState("2");
  const [status, setStatus] = useState<Status>({ phase: "idle", percent: 0, text: copy.ready });
  const [error, setError] = useState("");
  const activeRequestId = useRef("");
  const operations = useRef(new Map<string, OperationName>());
  const inputBytes = new TextEncoder().encode(input).length;

  useEffect(() => {
    document.documentElement.lang = isJapanese ? "ja" : "en";
    let mounted = true;
    let unsubscribe: (() => void) | undefined;

    void connectNativeBridge()
      .then((connectedBridge) => {
        if (!mounted) return;
        unsubscribe = connectedBridge.subscribe((event: BridgeEvent) => handleBridgeEvent(event));
        setBridge(connectedBridge);
        setConnectionError("");
      })
      .catch((connectError: unknown) => {
        if (mounted) {
          setConnectionError(connectError instanceof Error ? connectError.message : String(connectError));
        }
      });

    return () => {
      mounted = false;
      unsubscribe?.();
    };
    // Bridge subscriptions live for this screen's lifetime.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  function handleBridgeEvent(event: BridgeEvent) {
    if (event.requestId !== activeRequestId.current) {
      if (event.type === "completed" || event.type === "failed" || event.type === "cancelled") {
        operations.current.delete(event.requestId);
      }
      return;
    }

    if (event.type === "started") {
      setStatus({ phase: "running", percent: 0, text: copy.working });
    } else if (event.type === "progress") {
      setStatus({ phase: "running", percent: event.percent, text: copy.working });
    } else if (event.type === "completed") {
      const operation = operations.current.get(event.requestId);
      const resultText = typeof event.result.text === "string" ? event.result.text : "";
      if (operation === "formatJson") setOutput(resultText);
      if (operation === "openTextFile") setInput(resultText);
      const fileName = typeof event.result.fileName === "string" ? event.result.fileName : "";
      setStatus({
        phase: "done",
        percent: 100,
        text: fileName ? `${copy.complete}: ${fileName}` : copy.complete,
      });
      operations.current.delete(event.requestId);
      activeRequestId.current = "";
    } else if (event.type === "failed") {
      setError(errorMessage(event.error.code, event.error.message));
      setStatus({ phase: "failed", percent: 0, text: copy.bridgeError });
      operations.current.delete(event.requestId);
      activeRequestId.current = "";
    } else if (event.type === "cancelled") {
      setStatus({ phase: "cancelled", percent: 0, text: copy.cancelled });
      operations.current.delete(event.requestId);
      activeRequestId.current = "";
    }
  }

  function startOperation(operation: OperationName, payload: Record<string, string>) {
    if (!bridge) return;
    const requestId = makeRequestId();
    const request: OperationRequest = {
      contractVersion: BRIDGE_CONTRACT_VERSION,
      requestId,
      operation,
      payload,
    };
    activeRequestId.current = requestId;
    operations.current.set(requestId, operation);
    setError("");
    setStatus({ phase: "running", percent: 0, text: copy.working });
    try {
      bridge.startOperation(request);
    } catch (operationError) {
      operations.current.delete(requestId);
      activeRequestId.current = "";
      setStatus({ phase: "failed", percent: 0, text: copy.bridgeError });
      setError(operationError instanceof Error ? operationError.message : String(operationError));
    }
  }

  function formatJson() {
    if (!input.trim()) {
      setError(copy.formatError);
      return;
    }
    if (inputBytes > MAX_INPUT_BYTES) {
      setError(copy.sizeLimit);
      return;
    }
    startOperation("formatJson", { text: input, indentation });
  }

  function openFile() {
    startOperation("openTextFile", {});
  }

  function saveFile() {
    const text = output || input;
    if (new TextEncoder().encode(text).length > MAX_INPUT_BYTES) {
      setError(copy.sizeLimit);
      return;
    }
    startOperation("saveTextFile", { text, suggestedName: "formatted.json" });
  }

  function cancelOperation() {
    if (bridge && activeRequestId.current) bridge.cancelOperation(activeRequestId.current);
  }

  return (
    <main className="min-h-screen bg-background text-foreground">
      <div className="mx-auto flex w-full max-w-7xl flex-col gap-8 px-6 py-8 md:px-10 md:py-12">
        <header className="flex flex-col gap-6 border-b border-border pb-7 md:flex-row md:items-end md:justify-between">
          <div className="max-w-2xl">
            <p className="text-xs font-semibold tracking-[0.18em] text-primary">{copy.eyebrow}</p>
            <h1 className="mt-3 text-3xl font-semibold tracking-tight md:text-4xl">{copy.title}</h1>
            <p className="mt-3 max-w-xl text-sm leading-6 text-muted-foreground">{copy.intro}</p>
          </div>
          <div className="flex min-w-64 flex-col gap-2 rounded-lg border border-border bg-card px-4 py-3 shadow-sm">
            <div className="flex items-center gap-2 text-sm font-medium">
              <span className={`size-2 rounded-full ${bridge ? "bg-emerald-500" : "bg-amber-500"}`} />
              {bridge ? copy.connected : connectionError ? copy.disconnected : copy.connecting}
            </div>
            <p className="text-xs text-muted-foreground">
              {copy.contract} {bridge?.contractVersion ?? BRIDGE_CONTRACT_VERSION}
            </p>
          </div>
        </header>

        <section className="grid gap-5 lg:grid-cols-2" aria-label={copy.title}>
          <div className="flex min-w-0 flex-col gap-3">
            <div className="flex items-baseline justify-between gap-4">
              <label htmlFor="json-input" className="text-sm font-semibold">{copy.input}</label>
              <span className={`text-xs tabular-nums ${inputBytes > MAX_INPUT_BYTES ? "text-destructive" : "text-muted-foreground"}`}>
                {inputBytes.toLocaleString()} / 1 MiB {copy.bytes}
              </span>
            </div>
            <textarea
              id="json-input"
              className="min-h-[27rem] resize-y rounded-lg border border-border bg-card p-4 font-mono text-[13px] leading-6 text-foreground shadow-sm outline-none transition focus-visible:ring-2 focus-visible:ring-ring"
              spellCheck={false}
              value={input}
              onChange={(event) => setInput(event.target.value)}
              placeholder={copy.placeholder}
              aria-describedby="input-hint"
            />
            <p id="input-hint" className="text-xs text-muted-foreground">{copy.inputHint}</p>
          </div>

          <div className="flex min-w-0 flex-col gap-3">
            <label htmlFor="json-output" className="text-sm font-semibold">{copy.output}</label>
            <textarea
              id="json-output"
              className="min-h-[27rem] resize-y rounded-lg border border-border bg-muted/35 p-4 font-mono text-[13px] leading-6 text-foreground outline-none"
              spellCheck={false}
              readOnly
              value={output}
              placeholder={copy.outputPlaceholder}
              aria-describedby="output-hint"
            />
            <p id="output-hint" className="text-xs text-muted-foreground">{copy.outputHint}</p>
          </div>
        </section>

        <section className="flex flex-col gap-4 rounded-xl border border-border bg-card p-4 shadow-sm md:flex-row md:items-center md:justify-between md:px-5">
          <div className="flex flex-wrap items-center gap-3">
            <label htmlFor="indentation" className="text-sm font-medium">{copy.indentation}</label>
            <select
              id="indentation"
              className="h-10 rounded-md border border-border bg-background px-3 text-sm outline-none focus-visible:ring-2 focus-visible:ring-ring"
              value={indentation}
              onChange={(event) => setIndentation(event.target.value)}
            >
              <option value="2">{copy.indent2}</option>
              <option value="4">{copy.indent4}</option>
              <option value="tab">{copy.indentTab}</option>
              <option value="minified">{copy.indentMinified}</option>
            </select>
            <span className="hidden h-6 border-l border-border md:block" />
            <Button onClick={openFile} variant="outline" disabled={!bridge}>{copy.open}</Button>
            <Button onClick={saveFile} variant="outline" disabled={!bridge}>{copy.save}</Button>
          </div>
          <div className="flex flex-wrap items-center gap-3">
            {status.phase === "running" && (
              <Button onClick={cancelOperation} variant="ghost" size="sm">{copy.cancel}</Button>
            )}
            <Button onClick={formatJson} disabled={!bridge || inputBytes > MAX_INPUT_BYTES}>
              {copy.format}
            </Button>
          </div>
        </section>

        <footer className="flex flex-col gap-2 text-sm" aria-live="polite">
          <div className="flex items-center gap-3">
            <p className={status.phase === "failed" ? "text-destructive" : "text-muted-foreground"}>
              {status.text}
            </p>
            {status.phase === "running" && (
              <div className="h-1.5 w-40 overflow-hidden rounded-full bg-muted" role="progressbar" aria-valuenow={status.percent} aria-valuemin={0} aria-valuemax={100}>
                <div className="h-full rounded-full bg-primary transition-[width]" style={{ width: `${status.percent}%` }} />
              </div>
            )}
          </div>
          {error && <p className="text-destructive">{error}</p>}
          {connectionError && !bridge && <p className="sr-only">{connectionError}</p>}
        </footer>
      </div>
    </main>
  );
}

export default App;
