import { createRoot } from "react-dom/client";
import App from "@/app/App";
import { registerMaterialSymbolsFont } from "@/shared/lib/material-symbols";
import "@/shared/styles/index.css";

const rootElement = document.getElementById("root");
if (!rootElement) {
    throw new Error("Missing app root element.");
}

registerMaterialSymbolsFont();
createRoot(rootElement).render(<App />);
