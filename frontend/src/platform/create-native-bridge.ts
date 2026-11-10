import { isTauri } from "@tauri-apps/api/core";
import { connectTauriBridge } from "@/platform/tauri/bridge";
import { connectNativeBridge } from "@/platform/wkwebview/bridge";

export function createNativeBridge() {
  return isTauri() ? connectTauriBridge() : connectNativeBridge();
}
