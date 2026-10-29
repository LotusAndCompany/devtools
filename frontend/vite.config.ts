import path from "node:path";
import { fileURLToPath } from "node:url";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vite";

const frontendRoot = fileURLToPath(new URL(".", import.meta.url));

const developmentCsp = [
  "default-src 'none'",
  "base-uri 'self'",
  "form-action 'self'",
  "script-src 'self' 'unsafe-inline' qrc:",
  "style-src 'self' 'unsafe-inline'",
  "img-src 'self' data:",
  "font-src 'self' data:",
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
  "style-src 'self'",
  "img-src 'self' data:",
  "font-src 'self' data:",
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
        "@": frontendRoot,
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
  };
});
