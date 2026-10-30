const materialSymbolsFontUrl = "qrc:/fonts/MaterialSymbolsOutlined.ttf";

export function registerMaterialSymbolsFont(): void {
  const style = document.createElement("style");
  style.textContent = `
        @font-face {
            font-family: "Material Symbols Outlined";
            font-style: normal;
            font-weight: 100 700;
            src: url("${materialSymbolsFontUrl}") format("truetype");
        }
    `;
  document.head.append(style);
}
