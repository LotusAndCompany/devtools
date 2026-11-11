import { readFileSync } from "node:fs";
import { resolve } from "node:path";
import { describe, expect, it } from "vitest";

const contractPath = resolve(process.cwd(), "src/shared/bridge/types.ts");
const registryPath = resolve(
    process.cwd(),
    "../features/web_frontend/gui/bridge/operation_registry.cpp",
);
const tauriRegistryPath = resolve(process.cwd(), "../src-tauri/src/lib.rs");

function operationNames() {
    const contract = readFileSync(contractPath, "utf8");
    const registry = readFileSync(registryPath, "utf8");
    const tauriRegistry = readFileSync(tauriRegistryPath, "utf8");
    const contractStart = contract.indexOf("export type OperationContracts = {");
    const contractEnd = contract.indexOf("\n};", contractStart);
    const contractBlock = contract.slice(contractStart, contractEnd);
    const frontendNames = [...contractBlock.matchAll(/^  ([A-Za-z][A-Za-z0-9]*): \{/gm)].map(
        ([, name]) => name,
    );
    const nativeNames = [...registry.matchAll(/OperationEntry\{"([A-Za-z][A-Za-z0-9]*)"/g)].map(
        ([, name]) => name,
    );
    const tauriBlock =
        tauriRegistry.match(/const SUPPORTED_OPERATIONS: &\[&str\] = &\[(.*?)\];/s)?.[1] ?? "";
    const tauriNames = [...tauriBlock.matchAll(/"([A-Za-z][A-Za-z0-9]*)"/g)].map(
        ([, name]) => name,
    );
    return {
        frontendNames: frontendNames.toSorted(),
        nativeNames: nativeNames.toSorted(),
        tauriNames: tauriNames.toSorted(),
    };
}

describe("native operation contract parity", () => {
    it("keeps TypeScript operations and the C++ registry in sync", () => {
        const { frontendNames, nativeNames } = operationNames();

        expect(frontendNames).not.toHaveLength(0);
        expect(new Set(frontendNames).size).toBe(frontendNames.length);
        expect(new Set(nativeNames).size).toBe(nativeNames.length);
        expect(nativeNames).toEqual(frontendNames);
    });

    it("keeps every implemented Tauri operation inside the shared contract", () => {
        const { frontendNames, tauriNames } = operationNames();
        expect(tauriNames).toEqual(
            expect.arrayContaining([
                "formatJson",
                "openTextFile",
                "saveTextFile",
                "getSettings",
                "saveSettings",
                "getAboutInfo",
                "imageTakeDropped",
                "saveLastUsedTool",
            ]),
        );
        expect(tauriNames.every((name) => frontendNames.includes(name))).toBe(true);
    });
});
