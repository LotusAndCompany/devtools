import { connectNativeBridge as createQtNativeBridge } from "@/platform/qt/bridge";
import { connectNativeBridge as createWKWebViewNativeBridge } from "@/platform/wkwebview/bridge";

export function createNativeBridge() {
  return import.meta.env.VITE_DEVTOOLS_HOST === "wkwebview"
    ? createWKWebViewNativeBridge()
    : createQtNativeBridge();
}
