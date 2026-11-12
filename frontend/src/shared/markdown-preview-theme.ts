import previewStyles from "@/shared/styles/markdown-preview.css?inline";

export function applyMarkdownPreviewTheme(frame: HTMLIFrameElement | null) {
    const iframeDocument = frame?.contentDocument;
    if (!iframeDocument?.head) return;
    iframeDocument.body?.classList.add("markdown-preview-content");

    const rootStyles = getComputedStyle(document.documentElement);
    const color = (name: string) => rootStyles.getPropertyValue(name).trim();
    const previewTokens = new Set(
        Array.from(previewStyles.matchAll(/var\((--[\w-]+)/g), ([, token]) => token),
    );
    const tokenDeclarations = Array.from(previewTokens, (token) => {
        const value = color(token);
        return value ? `${token}: ${value};` : "";
    })
        .filter(Boolean)
        .join("\n");
    const style =
        iframeDocument.getElementById("devtools-preview-theme") ??
        iframeDocument.createElement("style");
    style.id = "devtools-preview-theme";
    style.textContent = `:root { color-scheme: ${rootStyles.colorScheme}; ${tokenDeclarations} }\n${previewStyles}`;
    if (!style.parentElement) iframeDocument.head.appendChild(style);
}

export function watchMarkdownPreviewTheme(applyPreviewTheme: () => void) {
    const observer = new MutationObserver(applyPreviewTheme);
    observer.observe(document.documentElement, {
        attributes: true,
        attributeFilter: ["data-theme"],
    });
    const colorScheme = window.matchMedia("(prefers-color-scheme: dark)");
    colorScheme.addEventListener("change", applyPreviewTheme);
    applyPreviewTheme();
    return () => {
        observer.disconnect();
        colorScheme.removeEventListener("change", applyPreviewTheme);
    };
}
