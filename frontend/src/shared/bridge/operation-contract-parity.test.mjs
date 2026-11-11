import { readFileSync } from "node:fs";
import { resolve } from "node:path";
import { describe, expect, it } from "vitest";

const contractPath = resolve(process.cwd(), "src/shared/bridge/types.ts");
const tauriRegistryPath = resolve(process.cwd(), "../src-tauri/src/lib.rs");

function operationNames() {
    const contract = readFileSync(contractPath, "utf8");
    const tauriRegistry = readFileSync(tauriRegistryPath, "utf8");
    const contractStart = contract.indexOf("export type OperationContracts = {");
    const contractEnd = contract.indexOf("\n};", contractStart);
    const contractBlock = contract.slice(contractStart, contractEnd);
    const frontendNames = [...contractBlock.matchAll(/^ {4}([A-Za-z][A-Za-z0-9]*): \{/gm)].map(
        ([, name]) => name,
    );
    const tauriBlock =
        tauriRegistry.match(/const SUPPORTED_OPERATIONS: &\[&str\] = &\[(.*?)\];/s)?.[1] ?? "";
    const tauriNames = [...tauriBlock.matchAll(/"([A-Za-z][A-Za-z0-9]*)"/g)].map(
        ([, name]) => name,
    );
    return {
        frontendNames: frontendNames.toSorted(),
        tauriNames: tauriNames.toSorted(),
    };
}

describe("native operation contract parity", () => {
    it("keeps the Tauri operation registry in sync with the shared contract", () => {
        const { frontendNames, tauriNames } = operationNames();

        expect(frontendNames).not.toHaveLength(0);
        expect(new Set(frontendNames).size).toBe(frontendNames.length);
        expect(new Set(tauriNames).size).toBe(tauriNames.length);
        expect(tauriNames).toEqual(frontendNames);
    });
});
