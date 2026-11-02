import { createReadStream } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { loadEnv } from "vite";
import { defineConfig } from "vitest/config";

const frontendRoot = fileURLToPath(new URL(".", import.meta.url));
const sourceRoot = path.resolve(frontendRoot, "src");
const devToolsLogoPath = path.resolve(frontendRoot, "../res/dev-tools_logo.png");
const developmentAssets = new Map<string, { path: string; contentType: string }>([
  ["/__devtools/dev-tools_logo.png", { path: devToolsLogoPath, contentType: "image/png" }],
]);

const qtDevelopmentCsp = [
  "default-src 'none'",
  "base-uri 'self'",
  "form-action 'self'",
  "script-src 'self' 'unsafe-inline' qrc:",
  "style-src 'self' 'unsafe-inline'",
  "img-src 'self' data: qrc:",
  "font-src 'self' data: qrc:",
  "connect-src 'self' ws://127.0.0.1:5173 qrc:",
  "object-src 'none'",
  "frame-src 'none'",
].join("; ");

const wkDevelopmentCsp = [
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

const qtProductionCsp = [
  "default-src 'none'",
  "base-uri 'self'",
  "form-action 'self'",
  "script-src 'self' qrc:",
  "style-src 'self' 'unsafe-inline'",
  "img-src 'self' data: qrc:",
  "font-src 'self' data: qrc:",
  "connect-src 'self' qrc:",
  "object-src 'none'",
  "frame-src 'none'",
].join("; ");

const wkProductionCsp = [
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

export default defineConfig(({ mode }) => {
  const useWKWebView = loadEnv(mode, frontendRoot, "VITE_").VITE_DEVTOOLS_HOST === "wkwebview";
  const csp =
    mode === "development"
      ? useWKWebView
        ? wkDevelopmentCsp
        : qtDevelopmentCsp
      : useWKWebView
        ? wkProductionCsp
        : qtProductionCsp;
  const cspHeader = `${csp}; frame-ancestors 'none'`;

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
            const developmentAsset = requestPath ? developmentAssets.get(requestPath) : undefined;
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

              createReadStream(developmentAsset.path).on("error", next).pipe(response);
              return;
            }
            if (requestPath === "/" || requestPath === "/index.html") {
              response.setHeader("Content-Security-Policy", cspHeader);
            }
            next();
          });
        },
        transformIndexHtml(html: string) {
          return html.replace("__CONTENT_SECURITY_POLICY__", csp);
        },
        generateBundle(_options, bundle) {
          if (!useWKWebView) return;
          const index = bundle["index.html"];
          if (!index || index.type !== "asset") {
            throw new Error(
              `The WKWebView HTML entry was not emitted as a text asset: ${Object.keys(bundle).join(", ")}`,
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
          format: useWKWebView ? "iife" : "es",
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
