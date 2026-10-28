import { useEffect, useMemo, useState, type KeyboardEvent, type MouseEvent, type ReactNode } from "react";
import { Button } from "@/components/ui/button";
import type { OperationName } from "@/bridge";

export type Locale = "en" | "ja";
export type OperationRunner = (operation: OperationName, payload: Record<string, unknown>) => Promise<Record<string, unknown>>;
type ToolPageProps = { id: string; locale: Locale; run: OperationRunner; active?: boolean };
type RegexOptions = { global: boolean; caseInsensitive: boolean; multiline: boolean; dotMatchesEverything: boolean; useUnicodeProperties: boolean; extendedPatternSyntax: boolean };

const text = (locale: Locale, en: string, ja: string) => locale === "ja" ? ja : en;

function useOperation(run: OperationRunner) {
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");
  const execute = async (operation: OperationName, payload: Record<string, unknown>) => {
    setBusy(true);
    setError("");
    try { return await run(operation, payload); }
    catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); return null; }
    finally { setBusy(false); }
  };
  return { busy, error, execute, setError };
}

function Heading({ title, detail }: { title: string; detail: string }) {
  return <header className="page-heading"><h1>{title}</h1><p>{detail}</p></header>;
}

function Field({ label, children }: { label: string; children: ReactNode }) {
  return <label className="field"><span className="field-label">{label}</span>{children}</label>;
}

function Area({ label, value, onChange, placeholder, rows = 12, readOnly = false }: { label: string; value: string; onChange?: (value: string) => void; placeholder?: string; rows?: number; readOnly?: boolean }) {
  return <Field label={label}><textarea className="form-control code-area" rows={rows} value={value} onChange={(event) => onChange?.(event.target.value)} placeholder={placeholder} readOnly={readOnly} spellCheck={false} /></Field>;
}

function OutputMessage({ error, message }: { error: string; message?: string }) {
  if (!error && !message) return null;
  return <p className={`operation-message ${error ? "error" : "success"}`} role="status">{error || message}</p>;
}

export function ToolPage({ id, locale, run, active }: ToolPageProps) {
  if (id === "image") return <ImagePage locale={locale} run={run} />;
  if (id === "phrase") return <PhrasePage locale={locale} run={run} active={active ?? false} />;
  if (id === "http") return <HttpPage locale={locale} run={run} />;
  if (id === "data") return <DataPage locale={locale} run={run} />;
  if (id === "db") return <DatabasePage locale={locale} run={run} />;
  if (id === "timestamp") return <TimestampPage locale={locale} run={run} />;
  if (id === "diff") return <DiffPage locale={locale} run={run} />;
  if (id === "markdown") return <MarkdownPage locale={locale} run={run} />;
  if (id === "command") return <CommandPage locale={locale} run={run} />;
  if (id === "qr") return <QrPage locale={locale} run={run} />;
  if (id === "regex") return <RegexPage locale={locale} run={run} />;
  return <div className="empty-state">{text(locale, "This tool is not available.", "このツールは利用できません。")}</div>;
}

function ImagePage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [image, setImage] = useState("");
  const [fileName, setFileName] = useState("");
  const [width, setWidth] = useState(0);
  const [height, setHeight] = useState(0);
  const [targetWidth, setTargetWidth] = useState("800");
  const [targetHeight, setTargetHeight] = useState("600");
  const [horizontalScale, setHorizontalScale] = useState("100");
  const [verticalScale, setVerticalScale] = useState("100");
  const [keepAspect, setKeepAspect] = useState(false);
  const [smooth, setSmooth] = useState(true);
  const [transparency, setTransparency] = useState("1");
  const [tolerance, setTolerance] = useState("0.1");
  const [colorMode, setColorMode] = useState("rgb");
  const [contiguousOnly, setContiguousOnly] = useState(true);
  const [splitMode, setSplitMode] = useState("division");
  const [horizontalDivisions, setHorizontalDivisions] = useState("2");
  const [verticalDivisions, setVerticalDivisions] = useState("2");
  const [cellWidth, setCellWidth] = useState("128");
  const [cellHeight, setCellHeight] = useState("128");
  const [ignoreRemainders, setIgnoreRemainders] = useState(true);

  const updateImage = (result: Record<string, unknown>) => {
    setImage(String(result.imageDataUrl ?? ""));
    setWidth(Number(result.width ?? 0));
    setHeight(Number(result.height ?? 0));
    if (result.fileName) setFileName(String(result.fileName));
  };
  const performEdit = async (action: string, extra: Record<string, unknown> = {}) => {
    const result = await execute("imageEdit", { action, ...extra });
    if (result) updateImage(result);
  };
  const open = async () => {
    const result = await execute("imageOpen", {});
    if (result) updateImage(result);
  };
  const save = async () => { const result = await execute("imageSave", {}); if (result) setFileName(String(result.fileName ?? fileName)); };
  const split = async () => { await execute("imageSplit", { mode: splitMode === "division" ? "division" : "cell-size", horizontalDivisions: Number(horizontalDivisions), verticalDivisions: Number(verticalDivisions), cellWidth: Number(cellWidth), cellHeight: Number(cellHeight), ignoreRemainders }); };
  const chooseTransparentPoint = (event: MouseEvent<HTMLImageElement>) => {
    const rect = event.currentTarget.getBoundingClientRect();
    const x = Math.max(0, Math.min(width - 1, Math.floor((event.clientX - rect.left) * width / rect.width)));
    const y = Math.max(0, Math.min(height - 1, Math.floor((event.clientY - rect.top) * height / rect.height)));
    void performEdit("transparent", { x, y, transparency: Number(transparency), tolerance: Number(tolerance), colorMode, contiguousOnly });
  };

  return <div className="tool-page"><Heading title={text(locale, "Image Editor", "画像編集")} detail={text(locale, "Resize, transform, make a selected color transparent and split images.", "サイズ変更、回転・反転、透明化、画像分割ができます。")}/>
    <div className="toolbar-row"><Button variant="outline" onClick={() => void open()} disabled={busy}>{text(locale, "Open image", "画像を開く")}</Button><Button variant="outline" onClick={() => void execute("imageReset", {}).then((result) => { if (result) updateImage(result); })} disabled={busy || !image}>{text(locale, "Reset", "リセット")}</Button><Button onClick={() => void save()} disabled={busy || !image}>{text(locale, "Save image", "画像を保存")}</Button><span className="muted-text">{fileName}{image ? ` · ${width} × ${height}` : ""}</span></div>
    <OutputMessage error={error}/>
    <div className="image-editor-layout"><div className="image-preview-panel">{image ? <img src={image} alt={text(locale, "Editable image preview. Click to select a color for transparency.", "編集プレビュー。クリックして透明化する色を選択。")} onClick={chooseTransparentPoint}/> : <div className="empty-state">{text(locale, "Open an image to begin.", "画像を開いて編集を始めてください。")}</div>}</div>
      <div className="image-controls">
        <section className="input-card"><h2>{text(locale, "Resize", "サイズ変更")}</h2><div className="form-grid two"><Field label={text(locale, "Width (px)", "幅 (px)")}><input className="form-control" type="number" min="1" max="32768" value={targetWidth} onChange={(event) => setTargetWidth(event.target.value)}/></Field><Field label={text(locale, "Height (px)", "高さ (px)")}><input className="form-control" type="number" min="1" max="32768" value={targetHeight} onChange={(event) => setTargetHeight(event.target.value)}/></Field><Field label={text(locale, "Horizontal scale (%)", "横倍率 (%)")}><input className="form-control" type="number" min="0.1" max="1000" step="0.1" value={horizontalScale} onChange={(event) => setHorizontalScale(event.target.value)}/></Field><Field label={text(locale, "Vertical scale (%)", "縦倍率 (%)")}><input className="form-control" type="number" min="0.1" max="1000" step="0.1" value={verticalScale} onChange={(event) => setVerticalScale(event.target.value)}/></Field></div><label className="inline-check"><input type="checkbox" checked={keepAspect} onChange={(event) => setKeepAspect(event.target.checked)}/>{text(locale, "Keep aspect ratio", "縦横比を維持")}</label><label className="inline-check"><input type="checkbox" checked={smooth} onChange={(event) => setSmooth(event.target.checked)}/>{text(locale, "Smooth scaling", "滑らかに拡大縮小")}</label><div className="button-row"><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("resize-size", { width: Number(targetWidth), height: Number(targetHeight), keepAspectRatio: keepAspect, smoothScaling: smooth })}>{text(locale, "Apply size", "サイズを適用")}</Button><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("resize-scale", { horizontalPercent: Number(horizontalScale), verticalPercent: Number(verticalScale), keepAspectRatio: keepAspect, smoothScaling: smooth })}>{text(locale, "Apply scale", "倍率を適用")}</Button></div></section>
        <section className="input-card"><h2>{text(locale, "Rotate and flip", "回転・反転")}</h2><div className="button-row"><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("rotate-left")}>↶ {text(locale, "Rotate left", "左に回転")}</Button><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("rotate-right")}>↷ {text(locale, "Rotate right", "右に回転")}</Button><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("flip-horizontal")}>{text(locale, "Flip horizontal", "左右反転")}</Button><Button variant="outline" disabled={busy || !image} onClick={() => void performEdit("flip-vertical")}>{text(locale, "Flip vertical", "上下反転")}</Button></div></section>
        <section className="input-card"><h2>{text(locale, "Make a color transparent", "色を透明化")}</h2><p className="muted-text">{text(locale, "Click a pixel in the preview to apply these settings.", "プレビュー上のピクセルをクリックすると設定を適用します。")}</p><div className="form-grid two"><Field label={text(locale, "Color mode", "色空間")}><select className="form-control" value={colorMode} onChange={(event) => setColorMode(event.target.value)}><option value="rgb">RGB</option><option value="hsl">HSL</option><option value="hsv">HSV</option></select></Field><Field label={text(locale, "Tolerance (0–1)", "許容範囲 (0–1)")}><input className="form-control" type="number" min="0" max="1" step="0.05" value={tolerance} onChange={(event) => setTolerance(event.target.value)}/></Field><Field label={text(locale, "Transparency (0–1)", "透明度 (0–1)")}><input className="form-control" type="number" min="0" max="1" step="0.05" value={transparency} onChange={(event) => setTransparency(event.target.value)}/></Field></div><label className="inline-check"><input type="checkbox" checked={contiguousOnly} onChange={(event) => setContiguousOnly(event.target.checked)}/>{text(locale, "Only connected pixels", "隣接する領域のみ")}</label></section>
        <section className="input-card"><h2>{text(locale, "Split image", "画像を分割")}</h2><Field label={text(locale, "Split by", "分割方法")}><select className="form-control" value={splitMode} onChange={(event) => setSplitMode(event.target.value)}><option value="division">{text(locale, "Number of parts", "分割数")}</option><option value="cell-size">{text(locale, "Cell size", "セルサイズ")}</option></select></Field>{splitMode === "division" ? <div className="form-grid two"><Field label={text(locale, "Columns", "横の分割数")}><input className="form-control" type="number" min="1" value={horizontalDivisions} onChange={(event) => setHorizontalDivisions(event.target.value)}/></Field><Field label={text(locale, "Rows", "縦の分割数")}><input className="form-control" type="number" min="1" value={verticalDivisions} onChange={(event) => setVerticalDivisions(event.target.value)}/></Field></div> : <div className="form-grid two"><Field label={text(locale, "Cell width", "セル幅")}><input className="form-control" type="number" min="1" value={cellWidth} onChange={(event) => setCellWidth(event.target.value)}/></Field><Field label={text(locale, "Cell height", "セル高さ")}><input className="form-control" type="number" min="1" value={cellHeight} onChange={(event) => setCellHeight(event.target.value)}/></Field></div>}<label className="inline-check"><input type="checkbox" checked={ignoreRemainders} onChange={(event) => setIgnoreRemainders(event.target.checked)}/>{text(locale, "Ignore remainder pixels", "余りのピクセルを無視")}</label><Button variant="outline" onClick={() => void split()} disabled={busy || !image}>{text(locale, "Choose folder and split", "保存先を選んで分割")}</Button></section>
      </div></div>
  </div>;
}

function PhrasePage({ locale, run, active }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [templates, setTemplates] = useState<Array<Record<string, unknown>>>([]);
  const [filename, setFilename] = useState("");
  const [title, setTitle] = useState("");
  const [content, setContent] = useState("");
  const [message, setMessage] = useState("");
  const refresh = async () => { const result = await execute("phraseData", { action: "list" }); if (result) setTemplates((result.templates as Array<Record<string, unknown>>) ?? []); };
  useEffect(() => { void run("phraseData", { action: "list" }).then((result) => setTemplates((result.templates as Array<Record<string, unknown>>) ?? [])).catch(() => {}); }, [run]);
  const select = (item: Record<string, unknown>) => { setFilename(String(item.filename ?? "")); setTitle(String(item.title ?? "")); setContent(String(item.content ?? "")); setMessage(""); };
  const save = async () => { const result = await execute("phraseData", { action: "save", filename, title, content }); if (result) { setFilename(String(result.filename)); setMessage(text(locale, "Template saved.", "定型文を保存しました。")); await refresh(); } };
  const remove = async () => { if (!filename) return; const result = await execute("phraseData", { action: "delete", filename }); if (result) { setFilename(""); setTitle(""); setContent(""); setMessage(text(locale, "Template deleted.", "定型文を削除しました。")); await refresh(); } };
  const copy = async () => { const result = await run("copyToClipboard", { text: content }); setMessage(result.copied ? text(locale, "Copied to clipboard.", "クリップボードにコピーしました。") : ""); };
  const onKeyDown = (event: KeyboardEvent<HTMLDivElement>) => {
    if (!active || !(event.metaKey || event.ctrlKey)) return;
    const key = event.key.toLowerCase();
    if (key === "s") { event.preventDefault(); if (!busy) void save(); }
    else if (key === "n") { event.preventDefault(); setFilename(""); setTitle(""); setContent(""); setMessage(""); }
    else if (key === "c" && event.shiftKey) { event.preventDefault(); if (content) void copy(); }
  };
  const onDeleteKey = (event: KeyboardEvent<HTMLDivElement>) => {
    const target = event.target;
    const isEditor = target instanceof HTMLElement &&
      (target.isContentEditable || ["INPUT", "TEXTAREA", "SELECT"].includes(target.tagName));
    if (!active || isEditor || !["Backspace", "Delete"].includes(event.key) || !filename) return;
    event.preventDefault();
    if (!busy) void remove();
  };
  return <div className="tool-page" onKeyDown={(event) => { onKeyDown(event); onDeleteKey(event); }}><Heading title={text(locale, "Phrase Generation", "定型文生成")} detail={text(locale, "Create reusable text templates and keep them in the local template library.", "再利用できる文章を作成し、ローカルのテンプレート一覧に保存します。")}/><div className="phrase-layout"><aside className="input-card template-list"><div className="toolbar-row"><h2>{text(locale, "Templates", "定型文一覧")}</h2><Button variant="outline" onClick={() => { setFilename(""); setTitle(""); setContent(""); setMessage(""); }}>{text(locale, "New", "新規")} <kbd>⌘N</kbd></Button></div>{templates.length === 0 && <p className="muted-text">{text(locale, "No saved templates.", "保存された定型文はありません。")}</p>}{templates.map((item) => <button key={String(item.filename)} className={`template-item ${filename === item.filename ? "active" : ""}`} onClick={() => select(item)}><strong>{String(item.title)}</strong><span>{String(item.content).slice(0, 72)}</span></button>)}</aside><div className="input-card"><Field label={text(locale, "Title", "タイトル")}><input className="form-control" value={title} onChange={(event) => setTitle(event.target.value)}/></Field><Area label={text(locale, "Template text", "テンプレート本文")} value={content} onChange={setContent} rows={16}/><div className="button-row"><Button onClick={() => void save()} disabled={busy}>{text(locale, "Save template", "定型文を保存")} <kbd>⌘S</kbd></Button><Button variant="outline" onClick={() => void copy()} disabled={!content}>{text(locale, "Copy text", "本文をコピー")} <kbd>⌘⇧C</kbd></Button><Button variant="outline" className="danger-button" onClick={() => void remove()} disabled={busy || !filename}>{text(locale, "Delete", "削除")}</Button></div><OutputMessage error={error} message={message}/></div></div></div>;
}

function HttpPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute, setError } = useOperation(run);
  const [method, setMethod] = useState("GET");
  const [url, setUrl] = useState("");
  const [headers, setHeaders] = useState("");
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [body, setBody] = useState("");
  const [parameters, setParameters] = useState<Array<{ key: string; value: string; description: string }>>(
    Array.from({ length: 10 }, () => ({ key: "", value: "", description: "" })),
  );
  const [response, setResponse] = useState<Record<string, unknown> | null>(null);
  const send = async () => {
    const parsedHeaders: Record<string, string> = {};
    for (const line of headers.split(/\r?\n/)) {
      if (!line.trim()) continue;
      const separator = line.indexOf(":");
      if (separator <= 0) {
        setError(text(locale, "Each header must use the name: value format.", "ヘッダーは name: value の形式で入力してください。"));
        return;
      }
      parsedHeaders[line.slice(0, separator).trim()] = line.slice(separator + 1).trim();
    }
    const result = await execute("sendHttpRequest", {
      method, url, headers: parsedHeaders, username, password, body,
      parameters: parameters.filter((parameter) => parameter.key.trim()),
    });
    if (result) setResponse(result);
  };
  return <div className="tool-page"><Heading title={text(locale, "HTTP Request", "HTTPリクエスト")} detail={text(locale, "Send HTTP requests with query parameters, headers, authentication and a body.", "クエリパラメーター、ヘッダー、認証、本文を設定してHTTPリクエストを送信します。")}/><section className="input-card"><div className="http-request-row"><Field label={text(locale, "Method", "メソッド")}><select className="form-control" value={method} onChange={(event) => setMethod(event.target.value)}><option>GET</option><option>POST</option><option>PUT</option><option>DELETE</option></select></Field><Field label="URL"><input className="form-control" value={url} onChange={(event) => setUrl(event.target.value)} placeholder="https://api.example.com/v1/items"/></Field><Button onClick={() => void send()} disabled={busy}>{busy ? text(locale, "Sending…", "送信中…") : text(locale, "Send", "送信")}</Button></div><section className="input-card"><h2>{text(locale, "Parameters", "パラメーター")}</h2><div className="http-parameter-grid"><strong>{text(locale, "Key", "キー")}</strong><strong>{text(locale, "Value", "値")}</strong><strong>{text(locale, "Description", "説明")}</strong>{parameters.map((parameter, index) => <div className="http-parameter-row" key={index}><input className="form-control" aria-label={`${text(locale, "Parameter key", "パラメーターキー")} ${index + 1}`} value={parameter.key} onChange={(event) => setParameters((current) => current.map((item, itemIndex) => itemIndex === index ? { ...item, key: event.target.value } : item))}/><input className="form-control" aria-label={`${text(locale, "Parameter value", "パラメーター値")} ${index + 1}`} value={parameter.value} onChange={(event) => setParameters((current) => current.map((item, itemIndex) => itemIndex === index ? { ...item, value: event.target.value } : item))}/><input className="form-control" aria-label={`${text(locale, "Parameter description", "パラメーター説明")} ${index + 1}`} value={parameter.description} onChange={(event) => setParameters((current) => current.map((item, itemIndex) => itemIndex === index ? { ...item, description: event.target.value } : item))}/></div>)}</div></section><div className="two-column http-options"><Area label={text(locale, "Headers (one name: value per line)", "ヘッダー (1行に1つ name: value)")} value={headers} onChange={setHeaders} rows={8} placeholder="Accept: application/json"/><div className="input-card auth-card"><h2>{text(locale, "Basic authentication", "Basic認証")}</h2><Field label={text(locale, "Username", "ユーザー名")}><input className="form-control" autoComplete="username" value={username} onChange={(event) => setUsername(event.target.value)}/></Field><Field label={text(locale, "Password", "パスワード")}><input className="form-control" type="password" autoComplete="current-password" value={password} onChange={(event) => setPassword(event.target.value)}/></Field></div></div><Area label={text(locale, "Request body", "リクエスト本文")} value={body} onChange={setBody} rows={8}/></section><OutputMessage error={error}/>{response && <section className="input-card response-card"><div className="toolbar-row"><h2>{text(locale, "Response", "レスポンス")}</h2><span className="muted-text">{String(response.status ?? "")} {String(response.reason ?? "")} · {String(response.elapsedMs ?? "")} ms · {String(response.bytes ?? "")} B</span></div><Area label={text(locale, "Response body", "レスポンス本文")} value={String(response.body ?? "")} readOnly rows={16}/></section>}</div>;
}

function DataPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [input, setInput] = useState("");
  const [output, setOutput] = useState("");
  const [format, setFormat] = useState("json");
  const [indentation, setIndentation] = useState("2");
  const convert = async () => { const result = await execute("convertData", { input, format, indentation }); if (result) setOutput(String(result.output ?? "")); };
  const open = async () => { const result = await execute("openTextFile", { fileType: "data" }); if (result) setInput(String(result.text ?? "")); };
  const save = async () => { await execute("saveTextFile", { text: output, suggestedName: `converted.${format.startsWith("yaml") ? "yaml" : format}`, fileType: "data" }); };
  return <div className="tool-page"><Heading title={text(locale, "Data Conversion", "データ変換")} detail={text(locale, "Parse JSON, YAML or TOML and emit the selected format.", "JSON、YAML、TOMLを解析し、指定した形式に変換します。")}/><div className="toolbar-row"><Button variant="outline" onClick={() => void open()} disabled={busy}>{text(locale, "Open file", "ファイルを開く")}</Button><Field label={text(locale, "Output format", "出力形式")}><select className="form-control compact" value={format} onChange={(event) => setFormat(event.target.value)}><option value="json">JSON</option><option value="yaml-block">YAML (block)</option><option value="yaml-flow">YAML (flow)</option><option value="toml">TOML</option></select></Field><Field label={text(locale, "Indentation", "インデント")}><select className="form-control compact" value={indentation} onChange={(event) => setIndentation(event.target.value)}><option value="2">2</option><option value="4">4</option><option value="tab">Tab</option><option value="minified">Minified</option></select></Field><Button onClick={() => void convert()} disabled={busy}>{text(locale, "Convert", "変換")}</Button></div><div className="two-column"><Area label={text(locale, "Input", "入力")} value={input} onChange={setInput} placeholder={'{"name":"DevTools"}'}/><Area label={text(locale, "Output", "出力")} value={output} onChange={setOutput} readOnly={false}/></div><div className="action-row"><Button variant="outline" onClick={() => void save()} disabled={!output || busy}>{text(locale, "Save output", "出力を保存")}</Button></div><OutputMessage error={error}/></div>;
}

function TimestampPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [value, setValue] = useState("");
  const [source, setSource] = useState("seconds");
  const [result, setResult] = useState<Record<string, unknown> | null>(null);
  const convert = async () => { const next = await execute("convertTimestamp", { value, source }); if (next) setResult(next); };
  return <div className="tool-page"><Heading title={text(locale, "Timestamp Conversion", "タイムスタンプ変換")} detail={text(locale, "Convert seconds, milliseconds and ISO-8601 timestamps.", "秒、ミリ秒、ISO-8601形式の時刻を変換します。")}/><div className="input-card"><div className="toolbar-row"><Field label={text(locale, "Input format", "入力形式")}><select className="form-control compact" value={source} onChange={(event) => setSource(event.target.value)}><option value="seconds">Unix seconds</option><option value="milliseconds">Unix milliseconds</option><option value="iso">ISO-8601</option></select></Field><Field label={text(locale, "Value", "値")}><input className="form-control" value={value} onChange={(event) => setValue(event.target.value)} placeholder="1726259557"/></Field><Button onClick={() => void convert()} disabled={busy}>{text(locale, "Convert", "変換")}</Button></div></div><OutputMessage error={error}/>{result && <div className="result-grid">{[["seconds", "Unix seconds"], ["milliseconds", "Unix milliseconds"], ["utc", "ISO-8601 UTC"], ["local", text(locale, "Local time", "ローカル時刻")]].map(([key, label]) => <ResultField key={key} label={label} value={String(result[key] ?? "")} run={run} locale={locale}/>)}</div>}</div>;
}

function ResultField({ label, value, run, locale }: { label: string; value: string; run: OperationRunner; locale: Locale }) {
  const [copied, setCopied] = useState(false);
  const copy = async () => { await run("copyToClipboard", { text: value }); setCopied(true); window.setTimeout(() => setCopied(false), 1500); };
  return <div className="result-card"><div className="result-heading"><span>{label}</span><Button variant="ghost" onClick={() => void copy()}>{copied ? "✓" : text(locale, "Copy", "コピー")}</Button></div><code>{value}</code></div>;
}

function DiffPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [left, setLeft] = useState("");
  const [right, setRight] = useState("");
  const [rows, setRows] = useState<Array<Record<string, unknown>>>([]);
  const compare = async () => { const result = await execute("compareText", { left, right }); if (result) setRows((result.rows as Array<Record<string, unknown>>) ?? []); };
  return <div className="tool-page"><Heading title={text(locale, "Text Diff", "テキスト差分")} detail={text(locale, "Compare two versions with the native line-diff service.", "ネイティブの行単位差分処理で2つのテキストを比較します。")}/><div className="two-column"><Area label={text(locale, "Original", "変更前")} value={left} onChange={setLeft}/><Area label={text(locale, "Modified", "変更後")} value={right} onChange={setRight}/></div><div className="action-row"><Button onClick={() => void compare()} disabled={busy}>{text(locale, "Compare", "比較")}</Button></div><OutputMessage error={error}/>{rows.length > 0 && <div className="diff-table" role="table" aria-label={text(locale, "Diff result", "差分結果")}>{rows.map((row, index) => <div key={index} className="diff-row"><code className={row.leftChanged ? "removed" : ""}>{String(row.left ?? "")}</code><code className={row.rightChanged ? "added" : ""}>{String(row.right ?? "")}</code></div>)}</div>}</div>;
}

function MarkdownPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [markdown, setMarkdown] = useState("# Markdown\n\nWrite here to preview.");
  const [preview, setPreview] = useState("");
  const [previewTheme, setPreviewTheme] = useState("system");
  const update = async () => { const result = await execute("renderMarkdown", { markdown }); if (result) setPreview(String(result.html ?? "")); };
  const open = async () => { const result = await execute("openTextFile", { fileType: "markdown" }); if (result) setMarkdown(String(result.text ?? "")); };
  const save = async (html = false) => { await execute("saveTextFile", { text: html ? preview : markdown, fileType: html ? "html" : "markdown", suggestedName: html ? "preview.html" : "document.md" }); };
  const previewDocument = useMemo(() => {
    if (!preview) return "";
    const policy = `<meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src data:; style-src 'unsafe-inline'; font-src data:; base-uri 'none'; form-action 'none'">`;
    return preview.replace("<head>", `<head>${policy}`);
  }, [preview]);
  return <div className="tool-page"><Heading title={text(locale, "Markdown Preview", "Markdownプレビュー")} detail={text(locale, "Edit Markdown and preview the rendered document in an isolated frame.", "Markdownを編集し、分離したフレームで表示を確認します。")}/><div className="toolbar-row"><Button variant="outline" onClick={() => void open()} disabled={busy}>{text(locale, "Open", "開く")}</Button><Button onClick={() => void update()} disabled={busy}>{text(locale, "Preview", "プレビュー")}</Button><Button variant="outline" onClick={() => void save()} disabled={busy}>{text(locale, "Save Markdown", "Markdownを保存")}</Button><Button variant="outline" onClick={() => void save(true)} disabled={!preview || busy}>{text(locale, "Export HTML", "HTMLを出力")}</Button><Field label={text(locale, "Preview theme", "プレビューのテーマ")}><select className="form-control compact" value={previewTheme} onChange={(event) => setPreviewTheme(event.target.value)}><option value="system">{text(locale, "System", "システム")}</option><option value="light">{text(locale, "Light", "ライト")}</option><option value="dark">{text(locale, "Dark", "ダーク")}</option></select></Field></div><div className="two-column markdown-columns"><Area label={text(locale, "Markdown source", "Markdownソース")} value={markdown} onChange={setMarkdown} rows={20}/><div className={`preview-panel markdown-${previewTheme}`}><span className="field-label">{text(locale, "Preview", "プレビュー")}</span><iframe title={text(locale, "Markdown preview", "Markdownプレビュー")} sandbox="" referrerPolicy="no-referrer" srcDoc={previewDocument}/></div></div><OutputMessage error={error}/></div>;
}

function CommandPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [category, setCategory] = useState("git");
  const [functions, setFunctions] = useState<Array<Record<string, unknown>>>([]);
  const [functionIndex, setFunctionIndex] = useState(0);
  const [optionIndex, setOptionIndex] = useState(0);
  const [value, setValue] = useState("");
  const [command, setCommand] = useState("");
  useEffect(() => { setFunctionIndex(0); setOptionIndex(0); void run("commandData", { action: "list", category }).then((result) => setFunctions((result.functions as Array<Record<string, unknown>>) ?? [])).catch(() => setFunctions([])); }, [category, run]);
  const options = (functions[functionIndex]?.options as Array<Record<string, unknown>>) ?? [];
  const generate = async () => { const result = await execute("commandData", { action: "generate", category, functionIndex, optionIndex, value }); if (result) setCommand(String(result.command ?? "")); };
  return <div className="tool-page"><Heading title={text(locale, "Command Generation", "コマンド生成")} detail={text(locale, "Build a command using the native command catalog.", "ネイティブのコマンド一覧からコマンドを作成します。")}/><div className="input-card"><div className="form-grid"><Field label={text(locale, "Category", "カテゴリ")}><select className="form-control" value={category} onChange={(event) => setCategory(event.target.value)}><option value="git">Git</option><option value="docker">Docker</option><option value="compose">Docker Compose</option></select></Field><Field label={text(locale, "Command", "コマンド")}><select className="form-control" value={functionIndex} onChange={(event) => { setFunctionIndex(Number(event.target.value)); setOptionIndex(0); }}>{functions.map((item, index) => <option key={index} value={index}>{String(item.name)}</option>)}</select></Field><Field label={text(locale, "Option", "オプション")}><select className="form-control" value={optionIndex} onChange={(event) => setOptionIndex(Number(event.target.value))}>{options.map((item, index) => <option key={index} value={index}>{String(item.name)}{item.title ? ` — ${String(item.title)}` : ""}</option>)}</select></Field>{Boolean(options[optionIndex]?.required || options[optionIndex]?.quotes) && <Field label={String(options[optionIndex]?.title || text(locale, "Value", "値"))}><input className="form-control" value={value} onChange={(event) => setValue(event.target.value)}/></Field>}</div><div className="action-row"><Button onClick={() => void generate()} disabled={busy || !functions.length}>{text(locale, "Generate command", "コマンドを作成")}</Button></div></div><OutputMessage error={error}/>{command && <ResultField label={text(locale, "Generated command", "生成したコマンド")} value={command} run={run} locale={locale}/>}</div>;
}

const qrFields: Record<string, Array<[string, string, boolean?]>> = {
  text: [["text_content", "Text", true]], url: [["url_content", "URL", true]], email: [["email_address", "Email", true]], phone: [["phone_number", "Phone number", true]],
  sms: [["sms_phone", "Phone number", true], ["sms_message", "Message", true]],
  wifi: [["wifi_ssid", "SSID", true], ["wifi_password", "Password"]],
  contact: [["contact_name", "Name", true], ["contact_phone", "Phone"], ["contact_email", "Email"]],
  calendar: [["cal_summary", "Event title", true], ["cal_start", "Start (ISO-8601)", true], ["cal_end", "End (ISO-8601)", true]],
  geo: [["geo_lat", "Latitude", true], ["geo_lng", "Longitude", true]],
};

function QrPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [type, setType] = useState("text");
  const [parameters, setParameters] = useState<Record<string, string>>({ text_content: "" });
  const [content, setContent] = useState("");
  const [image, setImage] = useState("");
  const [message, setMessage] = useState("");
  const fields = qrFields[type] ?? [];
  const generate = async () => { setMessage(""); const result = await execute("generateQr", { type, parameters }); if (result) { setContent(String(result.content ?? "")); setImage(String(result.imageDataUrl ?? "")); } };
  const clear = () => { setParameters(type === "text" ? { text_content: "" } : type === "wifi" ? { wifi_security: "WPA", wifi_hidden: "false" } : {}); setContent(""); setImage(""); setMessage(""); };
  const copy = async () => { const result = await execute("copyToClipboard", { imageDataUrl: image }); if (result) setMessage(text(locale, "QR image copied.", "QR画像をコピーしました。")); };
  const save = async () => { const result = await execute("saveQrImage", { type, parameters }); if (result) setMessage(text(locale, `Saved ${String(result.fileName ?? "QR image")}.`, `${String(result.fileName ?? "QR画像")}を保存しました。`)); };
  return <div className="tool-page"><Heading title={text(locale, "QR Code Generation", "QRコード生成")} detail={text(locale, "Generate QR codes using the native validation and image services.", "ネイティブの検証と画像処理でQRコードを生成します。")}/><div className="qr-layout"><div className="input-card"><Field label={text(locale, "QR type", "QRの種類")}><select className="form-control" value={type} onChange={(event) => { setType(event.target.value); setParameters(event.target.value === "text" ? { text_content: "" } : event.target.value === "wifi" ? { wifi_security: "WPA", wifi_hidden: "false" } : {}); setContent(""); setImage(""); setMessage(""); }}><option value="text">Text</option><option value="url">URL</option><option value="email">Email</option><option value="phone">Phone</option><option value="sms">SMS</option><option value="wifi">Wi-Fi</option><option value="contact">Contact</option><option value="calendar">Calendar</option><option value="geo">Location</option></select></Field>{fields.map(([key, label, required]) => <Field key={key} label={`${label}${required ? " *" : ""}`}><input className="form-control" type={key === "wifi_password" ? "password" : key.startsWith("cal_") ? "datetime-local" : "text"} value={parameters[key] ?? ""} onChange={(event) => setParameters((current) => ({ ...current, [key]: event.target.value }))}/></Field>)}{type === "wifi" && <><Field label={text(locale, "Security", "セキュリティ")}><select className="form-control" value={parameters.wifi_security ?? "WPA"} onChange={(event) => setParameters((current) => ({ ...current, wifi_security: event.target.value }))}><option>WPA</option><option>WEP</option><option value="nopass">{text(locale, "None", "なし")}</option></select></Field><label className="inline-check"><input type="checkbox" checked={parameters.wifi_hidden === "true"} onChange={(event) => setParameters((current) => ({ ...current, wifi_hidden: String(event.target.checked) }))}/>{text(locale, "Hidden network", "非公開ネットワーク")}</label></>}<div className="button-row"><Button onClick={() => void generate()} disabled={busy}>{text(locale, "Generate", "生成")}</Button><Button variant="outline" onClick={clear} disabled={busy}>{text(locale, "Clear", "クリア")}</Button></div></div><div className="qr-preview input-card">{image ? <img src={image} alt={text(locale, "Generated QR code", "生成したQRコード")}/> : <span>{text(locale, "QR code preview", "QRコードのプレビュー")}</span>}{content && <><code>{content}</code><div className="button-row"><Button variant="outline" onClick={() => void copy()} disabled={busy}>{text(locale, "Copy image", "画像をコピー")}</Button><Button variant="outline" onClick={() => void save()} disabled={busy}>{text(locale, "Save image", "画像を保存")}</Button></div></>}</div></div><OutputMessage error={error} message={message}/></div>;
}

function DatabasePage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [type, setType] = useState("QSQLITE");
  const [host, setHost] = useState("localhost");
  const [database, setDatabase] = useState("");
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [history, setHistory] = useState<Array<Record<string, unknown>>>([]);
  const [selectedFileName, setSelectedFileName] = useState("");
  const [connected, setConnected] = useState(false);
  const [displayName, setDisplayName] = useState("");
  const [tables, setTables] = useState<string[]>([]);
  const [sql, setSql] = useState("SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;");
  const [columns, setColumns] = useState<string[]>([]);
  const [rows, setRows] = useState<Array<Array<unknown>>>([]);
  const [truncated, setTruncated] = useState(false);
  const refreshHistory = async () => { const result = await run("databaseHistory", {}); setHistory((result.history as Array<Record<string, unknown>>) ?? []); setSelectedFileName(String(result.selectedFileName ?? "")); if (result.connected) setConnected(true); };
  useEffect(() => { let active = true; void run("databaseHistory", {}).then(async (result) => { if (!active) return; setHistory((result.history as Array<Record<string, unknown>>) ?? []); const selected = String(result.selectedFileName ?? ""); setSelectedFileName(selected); if (selected && !result.connected) { const connectedResult = await run("databaseConnect", { type: "QSQLITE" }); if (active && connectedResult.connected) { setConnected(true); setDisplayName(String(connectedResult.displayName ?? `SQLite: ${selected}`)); setTables((connectedResult.tables as string[]) ?? []); } } else if (result.connected) setConnected(true); }).catch(() => {}); return () => { active = false; }; }, [run]);
  const selectFile = async () => { const result = await execute("databaseSelectFile", {}); if (result) { setSelectedFileName(String(result.fileName ?? "")); setType("QSQLITE"); } };
  const connect = async (historyIndex?: number) => { const payload = historyIndex === undefined ? { type, host, database, username, password } : { historyIndex, password }; const result = await execute("databaseConnect", payload); if (result) { setConnected(true); setDisplayName(String(result.displayName ?? (type === "QSQLITE" ? `SQLite: ${selectedFileName}` : `${database}@${host}`))); setTables((result.tables as string[]) ?? []); await refreshHistory(); } };
  const query = async () => { const result = await execute("databaseQuery", { sql }); if (result) { setColumns((result.columns as string[]) ?? []); setRows((result.rows as Array<Array<unknown>>) ?? []); setTruncated(Boolean(result.truncated)); } };
  const loadTables = async () => { const result = await execute("databaseTables", {}); if (result) setTables((result.tables as string[]) ?? []); };
  const disconnect = async () => { await execute("databaseDisconnect", {}); setConnected(false); setDisplayName(""); setTables([]); setRows([]); setColumns([]); setSelectedFileName(""); };
  return <div className="tool-page"><Heading title={text(locale, "Database", "データベース")} detail={text(locale, "Connect to SQLite, MySQL or PostgreSQL, browse tables and run SQL.", "SQLite、MySQL、PostgreSQLに接続し、テーブルの確認とSQL実行ができます。")}/><div className="db-layout"><aside className="input-card db-sidebar"><div className="toolbar-row"><h2>{text(locale, "Connections", "接続")}</h2><Button variant="ghost" onClick={() => void refreshHistory()} aria-label={text(locale, "Refresh connections", "接続を更新")}>↻</Button></div>{connected && <div className="connected-card"><strong>{displayName}</strong><Button variant="outline" onClick={() => void disconnect()}>{text(locale, "Disconnect", "切断")}</Button></div>}{history.map((item) => <div key={String(item.index)} className="history-item"><div><strong>{String(item.displayName)}</strong><span>{String(item.host || item.database)}</span></div><Button variant="outline" disabled={busy} onClick={() => void connect(Number(item.index))}>{text(locale, "Connect", "接続")}</Button></div>)}<div className="db-connect-form"><h3>{text(locale, "New connection", "新しい接続")}</h3><Field label={text(locale, "Database type", "データベースの種類")}><select className="form-control" value={type} onChange={(event) => setType(event.target.value)}><option value="QSQLITE">SQLite</option><option value="QMYSQL">MySQL</option><option value="QPSQL">PostgreSQL</option></select></Field>{type === "QSQLITE" ? <><div className="selected-file">{selectedFileName || text(locale, "No file selected", "ファイル未選択")}</div><Button variant="outline" onClick={() => void selectFile()} disabled={busy}>{text(locale, "Choose SQLite file", "SQLiteファイルを選択")}</Button></> : <><Field label={text(locale, "Host", "ホスト")}><input className="form-control" value={host} onChange={(event) => setHost(event.target.value)}/></Field><Field label={text(locale, "Database", "データベース")}><input className="form-control" value={database} onChange={(event) => setDatabase(event.target.value)}/></Field><Field label={text(locale, "Username", "ユーザー名")}><input className="form-control" value={username} onChange={(event) => setUsername(event.target.value)}/></Field><Field label={text(locale, "Password", "パスワード")}><input className="form-control" type="password" value={password} onChange={(event) => setPassword(event.target.value)}/></Field></>}<Button onClick={() => void connect()} disabled={busy || (type === "QSQLITE" ? !selectedFileName : !host || !database || !username)}>{text(locale, "Connect", "接続")}</Button></div></aside><div className="db-workspace"><div className="input-card"><div className="toolbar-row"><h2>{text(locale, "Tables", "テーブル")}</h2><Button variant="outline" onClick={() => void loadTables()} disabled={!connected || busy}>{text(locale, "Refresh", "更新")}</Button></div><div className="table-chips">{tables.map((table) => <button key={table} onClick={() => setSql(`SELECT * FROM "${table.replaceAll('"', '""')}" LIMIT 1000;`)}>{table}</button>)}{tables.length === 0 && <span className="muted-text">{connected ? text(locale, "No tables found.", "テーブルがありません。") : text(locale, "Connect to a database to view tables.", "データベースに接続するとテーブルを表示します。")}</span>}</div></div><div className="input-card"><Area label="SQL" value={sql} onChange={setSql} rows={9}/><div className="action-row"><Button onClick={() => void query()} disabled={!connected || busy}>{text(locale, "Execute", "実行")}</Button></div></div>{columns.length > 0 && <div className="table-wrap"><table><thead><tr>{columns.map((column) => <th key={column}>{column}</th>)}</tr></thead><tbody>{rows.map((row, rowIndex) => <tr key={rowIndex}>{row.map((value, columnIndex) => <td key={columnIndex}>{value === null ? <em>NULL</em> : String(value)}</td>)}</tr>)}</tbody></table>{truncated && <p className="muted-text">{text(locale, "Showing the first 1,000 rows.", "先頭の1,000行を表示しています。")}</p>}</div>}</div></div><OutputMessage error={error}/></div>;
}

function RegexPage({ locale, run }: Omit<ToolPageProps, "id">) {
  const { busy, error, execute } = useOperation(run);
  const [pattern, setPattern] = useState("");
  const [input, setInput] = useState("");
  const [replacement, setReplacement] = useState("");
  const [options, setOptions] = useState<RegexOptions>({ global: true, caseInsensitive: false, multiline: false, dotMatchesEverything: false, useUnicodeProperties: false, extendedPatternSyntax: false });
  const [matches, setMatches] = useState<Array<Record<string, unknown>>>([]);
  const [replacedText, setReplacedText] = useState("");
  const [quickReference, setQuickReference] = useState(true);
  const [settingsLoaded, setSettingsLoaded] = useState(false);
  const evaluate = async () => { const result = await execute("evaluateRegex", { pattern, text: input, replacement, ...options }); if (result) { setMatches((result.matches as Array<Record<string, unknown>>) ?? []); setReplacedText(String(result.replacedText ?? "")); } };
  useEffect(() => { void run("regexSettings", { action: "load" }).then((result) => { setPattern(String(result.pattern ?? "")); setInput(String(result.testText ?? "")); setReplacement(String(result.replacePattern ?? "")); setOptions((current) => ({ ...current, ...((result.options as Partial<RegexOptions>) ?? {}) })); setSettingsLoaded(true); }).catch(() => setSettingsLoaded(true)); }, [run]);
  useEffect(() => { if (!settingsLoaded) return; const timer = window.setTimeout(() => { void run("regexSettings", { action: "save", pattern, testText: input, replacePattern: replacement, options }); }, 400); return () => window.clearTimeout(timer); }, [pattern, input, replacement, options, settingsLoaded, run]);
  const setOption = (key: keyof RegexOptions) => setOptions((current) => ({ ...current, [key]: !current[key] }));
  const copyMatches = async () => { const plain = matches.map((match, index) => `${index + 1}: ${String(match.value ?? "")} @ ${String(match.offset ?? 0)}`).join("\n"); await run("copyToClipboard", { text: plain }); };
  const presets = [{ label: "Email", pattern: "[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}" }, { label: "URL", pattern: "https?://[^\\s]+" }, { label: "IPv4", pattern: "\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b" }, { label: "ISO date", pattern: "\\b\\d{4}-\\d{2}-\\d{2}\\b" }, { label: "Phone (E.164)", pattern: "\\+[1-9]\\d{1,14}" }, { label: "UUID v4", pattern: "\\b[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}\\b" }];
  const flags: Array<[keyof RegexOptions, string, string]> = [["global", "Global", "すべて検索"], ["caseInsensitive", "Case insensitive", "大文字小文字を無視"], ["multiline", "Multiline", "複数行"], ["dotMatchesEverything", "Dot matches newline", "ドットを改行にも一致"], ["useUnicodeProperties", "Unicode properties", "Unicodeプロパティ"], ["extendedPatternSyntax", "Extended syntax", "拡張構文"]];
  const insert = (value: string) => setPattern((current) => current + value);
  return <div className="tool-page"><Heading title={text(locale, "Regex Tester", "正規表現テスター")} detail={text(locale, "Test Qt regular-expression syntax, inspect captures and preview replacement.", "Qtの正規表現構文をテストし、キャプチャと置換結果を確認します。")}/><div className="regex-layout"><div className="input-card"><div className="toolbar-row"><Field label={text(locale, "Preset", "プリセット")}><select className="form-control compact" defaultValue="" onChange={(event) => { const preset = presets.find((item) => item.label === event.target.value); if (preset) setPattern(preset.pattern); }}><option value="">{text(locale, "Select preset pattern…", "プリセットを選択…")}</option>{presets.map((preset) => <option key={preset.label}>{preset.label}</option>)}</select></Field><Button variant="outline" onClick={() => void run("copyToClipboard", { text: pattern })}>{text(locale, "Copy pattern", "パターンをコピー")}</Button></div><Field label={text(locale, "Pattern", "パターン")}><input className="form-control code-input" value={pattern} onChange={(event) => setPattern(event.target.value)} placeholder="(\\w+)"/></Field><div className="flag-grid">{flags.map(([key, en, ja]) => <label key={key} className="inline-check"><input type="checkbox" checked={options[key]} onChange={() => setOption(key)}/>{text(locale, en, ja)}</label>)}</div><Area label={text(locale, "Test text", "テスト文字列")} value={input} onChange={setInput} rows={14}/><div className="action-row"><Button onClick={() => void evaluate()} disabled={busy}>{text(locale, "Test pattern", "テスト")}</Button><Button variant="outline" onClick={() => void copyMatches()} disabled={!matches.length}>{text(locale, "Copy matches", "マッチをコピー")}</Button></div></div><div className="regex-results"><section className="input-card"><div className="toolbar-row"><h2>{text(locale, "Matches", "マッチ")}</h2><span className="muted-text">{matches.length}</span></div>{matches.length === 0 ? <p className="muted-text">{text(locale, "No matches yet.", "マッチ結果はありません。")}</p> : <div className="match-list">{matches.map((match, index) => <div key={index} className="match-item"><strong>{String(match.value)}</strong><span>{text(locale, "Offset", "位置")} {String(match.offset)} · {text(locale, "Length", "長さ")} {String(match.length)}</span>{((match.groups as Array<Record<string, unknown>>) ?? []).slice(1).map((group) => <code key={String(group.index)}>{String(group.name || group.index)}: {String(group.value)}</code>)}</div>)}</div>}</section><section className="input-card"><h2>{text(locale, "Replacement", "置換")}</h2><Field label={text(locale, "Replacement pattern", "置換文字列")}><input className="form-control code-input" value={replacement} onChange={(event) => setReplacement(event.target.value)} placeholder="$1"/></Field><Area label={text(locale, "Result", "結果")} value={replacedText} readOnly rows={8}/><Button variant="outline" onClick={() => void run("copyToClipboard", { text: replacedText })} disabled={!replacedText}>{text(locale, "Copy result", "結果をコピー")}</Button></section><section className="input-card"><div className="toolbar-row"><h2>{text(locale, "Quick reference", "クイックリファレンス")}</h2><Button variant="ghost" onClick={() => setQuickReference((value) => !value)}>{quickReference ? "−" : "+"}</Button></div>{quickReference && <div className="quick-reference">{[["\\d", text(locale, "Digit", "数字")], ["\\w", text(locale, "Word character", "単語文字")], ["\\s", text(locale, "Whitespace", "空白")], [".", text(locale, "Any character", "任意の文字")], ["^", text(locale, "Start of line", "行頭")], ["$", text(locale, "End of line", "行末")], ["(?:…)", text(locale, "Non-capturing group", "非キャプチャグループ")], ["(?<name>…)", text(locale, "Named capture", "名前付きキャプチャ")]].map(([token, description]) => <button key={token} onClick={() => insert(token)}><code>{token}</code><span>{description}</span></button>)}</div>}</section></div></div><OutputMessage error={error}/></div>;
}
