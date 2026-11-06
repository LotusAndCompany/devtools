import { connectNativeBridge } from "@/platform/wkwebview/bridge";

export function createNativeBridge() {
  return connectNativeBridge();
}
