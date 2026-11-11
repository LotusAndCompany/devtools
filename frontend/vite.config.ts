import { createReadStream } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vitest/config";

const frontendRoot = fileURLToPath(new URL(".", import.meta.url));
const sourceRoot = path.resolve(frontendRoot, "src");
const devToolsLogoPath = path.resolve(frontendRoot, "../res/dev-tools_logo.png");
const developmentAssets = new Map<string, { path: string; contentType: string }>([
    ["/__devtools/dev-tools_logo.png", { path: devToolsLogoPath, contentType: "image/png" }],
]);

const developmentCsp = [
    "default-src 'none'",
    "base-uri 'self'",
    "form-action 'none'",
    "script-src 'self' 'unsafe-inline'",
    "style-src 'self' 'unsafe-inline'",
    "img-src 'self' data:",
    "font-src 'self' data:",
    "connect-src 'self' ws://127.0.0.1:5173",
    "object-src 'none'",
    "frame-src 'self'",
].join("; ");

const productionCsp = [
    "default-src 'none'",
    "base-uri 'none'",
    "form-action 'none'",
    "script-src file:",
    "style-src file: 'unsafe-inline'",
    "img-src file: data:",
    "font-src file: data:",
    "connect-src 'none'",
    "object-src 'none'",
    "frame-src 'self'",
].join("; ");

const tauriDevelopmentCsp = [
    "default-src 'self'",
    "base-uri 'none'",
    "form-action 'none'",
    "script-src 'self' 'unsafe-inline'",
    "style-src 'self' 'unsafe-inline'",
    "img-src 'self' asset: http://asset.localhost blob: data:",
    "font-src 'self' data:",
    "connect-src 'self' ipc: http://ipc.localhost ws://127.0.0.1:5173",
    "object-src 'none'",
    "frame-src 'self'",
].join("; ");

const tauriModes = new Set(["tauri-dev", "tauri-build"]);

export default defineConfig(({ mode }) => {
    const isTauri = tauriModes.has(mode);
    const csp = mode === "development" ? developmentCsp : productionCsp;
    const cspHeader = `${csp}; frame-ancestors 'none'`;
    const tauriCspHeader = `${tauriDevelopmentCsp}; frame-ancestors 'none'`;

    return {
        root: frontendRoot,
        base: "./",
        plugins: [
            react(),
            tailwindcss(),
            {
                name: "devtools-content-security-policy",
                enforce: "post",
                configureServer(server) {
                    server.middlewares.use((request, response, next) => {
                        const requestPath = request.url?.split("?")[0];
                        const developmentAsset = requestPath
                            ? developmentAssets.get(requestPath)
                            : undefined;
                        if (developmentAsset) {
                            if (request.method !== "GET" && request.method !== "HEAD") {
                                response.statusCode = 405;
                                response.end();
                                return;
                            }

                            response.setHeader("Content-Type", developmentAsset.contentType);
                            response.setHeader("Cache-Control", "no-cache");
                            if (request.method === "HEAD") {
                                response.end();
                                return;
                            }

                            createReadStream(developmentAsset.path)
                                .on("error", next)
                                .pipe(response);
                            return;
                        }
                        if (requestPath === "/" || requestPath === "/index.html") {
                            response.setHeader(
                                "Content-Security-Policy",
                                isTauri ? tauriCspHeader : cspHeader,
                            );
                        }
                        next();
                    });
                },
                transformIndexHtml(html: string) {
                    return isTauri
                        ? html.replace(/\s*<meta http-equiv="Content-Security-Policy"[^>]*>/, "")
                        : html.replace("__CONTENT_SECURITY_POLICY__", csp);
                },
                generateBundle(_options, bundle) {
                    const index = bundle["index.html"];
                    if (!index || index.type !== "asset") {
                        throw new Error(
                            `The Tauri HTML entry was not emitted as a text asset: ${Object.keys(bundle).join(", ")}`,
                        );
                    }
                    const htmlSource =
                        typeof index.source === "string"
                            ? index.source
                            : new TextDecoder().decode(index.source);
                    index.source = htmlSource
                        .replace(
                            /<script type="module" crossorigin src="([^"]+)"><\/script>/,
                            '<script defer src="$1"></script>',
                        )
                        .replace(
                            /<link rel="stylesheet" crossorigin href="([^"]+)">/,
                            '<link rel="stylesheet" href="$1">',
                        );
                },
            },
        ],
        resolve: {
            alias: {
                "@": sourceRoot,
            },
        },
        server: {
            host: "127.0.0.1",
            port: 5173,
            strictPort: true,
            cors: {
                origin: "http://127.0.0.1:5173",
            },
        },
        build: {
            outDir: path.resolve(frontendRoot, "dist"),
            emptyOutDir: true,
            cssCodeSplit: false,
            rollupOptions: {
                output: {
                    entryFileNames: "assets/app.js",
                    chunkFileNames: "assets/[name].js",
                    assetFileNames: (asset) => {
                        if (asset.name?.endsWith(".css")) return "assets/style.css";
                        return "assets/[name][extname]";
                    },
                    format: "iife",
                },
            },
            assetsInlineLimit: 20_000,
        },
        test: {
            environment: "happy-dom",
            setupFiles: ["./src/test/setup.ts"],
            clearMocks: true,
            restoreMocks: true,
        },
    };
});
