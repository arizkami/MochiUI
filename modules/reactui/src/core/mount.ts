import React from "react";
import { api, NodeId } from "./api";
import { Reconciler } from "./reconciler";

export function mount(element: React.ReactElement): NodeId {
  const container = api().createNode("View");
  const root = (Reconciler as any).createContainer(
    container, 0, null, false, null, "", console.error, null, null, null,
  );
  (Reconciler as any).updateContainerSync(element, root, null, null);
  (Reconciler as any).flushSyncWork();
  api().setRoot(container);
  return container;
}
