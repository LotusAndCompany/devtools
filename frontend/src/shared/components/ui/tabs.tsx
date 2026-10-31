import type { ComponentProps } from "react";
import { Tabs as BaseTabs } from "@base-ui/react/tabs";

function TabsList(props: ComponentProps<typeof BaseTabs.List>) {
    return <BaseTabs.List {...props} data-slot="tabs-list" />;
}

function TabsTab(props: ComponentProps<typeof BaseTabs.Tab>) {
    return <BaseTabs.Tab {...props} data-slot="tabs-tab" />;
}

function TabsPanel(props: ComponentProps<typeof BaseTabs.Panel>) {
    return <BaseTabs.Panel {...props} data-slot="tabs-panel" />;
}

const Tabs = {
    Root: BaseTabs.Root,
    List: TabsList,
    Tab: TabsTab,
    Panel: TabsPanel,
};

export { Tabs };
