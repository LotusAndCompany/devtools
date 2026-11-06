import { readFileSync } from "node:fs";
import { resolve } from "node:path";
import { describe, expect, it } from "vitest";

const contractPath = resolve(process.cwd(), "src/shared/bridge/types.ts");
const registryPath = resolve(
    process.cwd(),
    "../features/web_frontend/gui/bridge/operation_registry.cpp",
);

function operationNames() {
    const contract = readFileSync(contractPath, "utf8");
    const registry = readFileSync(registryPath, "utf8");
    const contractStart = contract.indexOf("export type OperationContracts = {");
    const contractEnd = contract.indexOf("\n};", contractStart);
    const contractBlock = contract.slice(contractStart, contractEnd);
    const frontendNames = [...contractBlock.matchAll(/^  ([A-Za-z][A-Za-z0-9]*): \{/gm)].map(
        ([, name]) => name,
    );
    const nativeNames = [...registry.matchAll(/OperationEntry\{"([A-Za-z][A-Za-z0-9]*)"/g)].map(
        ([, name]) => name,
    );
    return { frontendNames: frontendNames.toSorted(), nativeNames: nativeNames.toSorted() };
}

describe("native operation contract parity", () => {
    it("keeps TypeScript operations and the C++ registry in sync", () => {
        const { frontendNames, nativeNames } = operationNames();

        expect(frontendNames).not.toHaveLength(0);
        expect(new Set(frontendNames).size).toBe(frontendNames.length);
        expect(new Set(nativeNames).size).toBe(nativeNames.length);
        expect(nativeNames).toEqual(frontendNames);
    });
});
