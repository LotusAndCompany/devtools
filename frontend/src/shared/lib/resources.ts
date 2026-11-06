import devToolsLogo from "../../../../res/dev-tools_logo.png";

export const devToolsLogoUrl = import.meta.env.DEV
  ? "/__devtools/dev-tools_logo.png"
  : devToolsLogo;
