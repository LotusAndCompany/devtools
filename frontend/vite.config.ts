import { createReadStream } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vitest/config";

const frontendRoot = fileURLToPath(new URL(".", import.meta.url));
const sourceRoot = path.resolve(frontendRoot, "src");
const materialSymbolsFontPath = path.resolve(
  frontendRoot,
  "../res/fonts/MaterialSymbolsOutlined.ttf",
);

const developmentCsp = [
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

const developmentCspHeader = `${developmentCsp}; frame-ancestors 'none'`;

const productionCsp = [
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

export default defineConfig(({ mode }) => {
  const csp = mode === "development" ? developmentCsp : productionCsp;

  return {
    root: frontendRoot,
    base: "./",
    plugins: [
      react(),
      tailwindcss(),
      {
        name: "devtools-content-security-policy",
        configureServer(server) {
          server.middlewares.use((request, response, next) => {
            const requestPath = request.url?.split("?")[0];
            if (requestPath === "/__devtools/MaterialSymbolsOutlined.ttf") {
              if (request.method !== "GET" && request.method !== "HEAD") {
                response.statusCode = 405;
                response.end();
                return;
              }

              response.setHeader("Content-Type", "font/ttf");
              response.setHeader("Cache-Control", "no-cache");
              if (request.method === "HEAD") {
                response.end();
                return;
              }

              createReadStream(materialSymbolsFontPath).on("error", next).pipe(response);
              return;
            }
            if (requestPath === "/" || requestPath === "/index.html") {
              response.setHeader("Content-Security-Policy", developmentCspHeader);
            }
            next();
          });
        },
        transformIndexHtml(html: string) {
          return html.replace("__CONTENT_SECURITY_POLICY__", csp);
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
          assetFileNames: (asset) =>
            asset.name?.endsWith(".css") ? "assets/style.css" : "assets/[name][extname]",
        },
      },
    },
    test: {
      environment: "happy-dom",
      setupFiles: ["./src/test/setup.ts"],
      clearMocks: true,
      restoreMocks: true,
    },
  };
});
