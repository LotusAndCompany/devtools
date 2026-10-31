import materialSymbolsFontStyles from "@/shared/styles/fonts.css?inline";

const materialSymbolsFontUrl = import.meta.env.DEV
  ? "/__devtools/MaterialSymbolsOutlined.ttf"
  : "qrc:/fonts/MaterialSymbolsOutlined.ttf";

export function registerMaterialSymbolsFont(): void {
  const style = document.createElement("style");
  style.textContent = materialSymbolsFontStyles.replace(
    "data:text/plain,__MATERIAL_SYMBOLS_FONT_URL__",
    materialSymbolsFontUrl,
  );
  document.head.append(style);
}
