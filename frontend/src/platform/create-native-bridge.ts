import { connectNativeBridge as createQtNativeBridge } from "@/platform/qt/bridge";
import { connectNativeBridge as createWKWebViewNativeBridge } from "@/platform/wkwebview/bridge";

export function createNativeBridge() {
  if (window.webkit?.messageHandlers?.devtoolsNativeBridge) {
    return createWKWebViewNativeBridge();
  }
  if (window.qt?.webChannelTransport) return createQtNativeBridge();
  return import.meta.env.VITE_DEVTOOLS_HOST === "wkwebview"
    ? createWKWebViewNativeBridge()
    : createQtNativeBridge();
}
