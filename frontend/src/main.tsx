import { createRoot } from "react-dom/client";
import App from "@/app/App";
import { TooltipProvider } from "@/shared/components/ui/tooltip";
import "@/shared/styles/index.css";

const rootElement = document.getElementById("root");
if (!rootElement) {
    throw new Error("Missing app root element.");
}

createRoot(rootElement).render(
    <TooltipProvider>
        <App />
    </TooltipProvider>,
);
