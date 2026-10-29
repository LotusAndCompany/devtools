import { createRoot } from "react-dom/client";
import App from "@/App";
import "@/ui.css";

const rootElement = document.getElementById("root");
if (!rootElement) {
    throw new Error("Missing app root element.");
}

createRoot(rootElement).render(<App />);
