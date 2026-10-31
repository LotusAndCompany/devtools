import type { ComponentProps } from "react";
import { Tabs as BaseTabs } from "@base-ui/react/tabs";

type TabsListProps = ComponentProps<typeof BaseTabs.List> & {
    variant?: "default" | "line";
};

function Tabs(props: ComponentProps<typeof BaseTabs.Root>) {
    return <BaseTabs.Root {...props} data-slot="tabs" />;
}

function TabsList({ variant = "default", ...props }: TabsListProps) {
    return <BaseTabs.List {...props} data-slot="tabs-list" data-variant={variant} />;
}

function TabsTrigger(props: ComponentProps<typeof BaseTabs.Tab>) {
    return <BaseTabs.Tab {...props} data-slot="tabs-trigger" />;
}

function TabsContent(props: ComponentProps<typeof BaseTabs.Panel>) {
    return <BaseTabs.Panel {...props} data-slot="tabs-content" />;
}

export { Tabs, TabsContent, TabsList, TabsTrigger };
