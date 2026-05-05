import ReactReconciler from "react-reconciler";

type NodeId = number;

type AureliaApi = {
  createNode: (type: string) => NodeId;
  appendChild: (parent: NodeId, child: NodeId) => void;
  removeChild: (parent: NodeId, child: NodeId) => void;
  setText: (id: NodeId, text: string) => void;
  setStyle: (id: NodeId, key: string, value: string) => void;
  setRoot: (id: NodeId) => void;
};

function api(): AureliaApi {
  const a = (globalThis as any).AureliaUI;
  if (!a) throw new Error("globalThis.AureliaUI is missing (native bindings not installed)");
  return a as AureliaApi;
}

type HostContext = null;
type UpdatePayload = { props: any } | null;

const hostConfig: any = {
  now: Date.now,
  supportsMutation: true,

  getRootHostContext(): HostContext {
    return null;
  },
  getChildHostContext(): HostContext {
    return null;
  },

  createInstance(type: string, props: any): NodeId {
    const id = api().createNode(type);
    if (props?.style) {
      for (const [k, v] of Object.entries(props.style)) {
        if (v == null) continue;
        api().setStyle(id, String(k), String(v));
      }
    }
    if (typeof props?.text === "string") api().setText(id, props.text);
    return id;
  },

  createTextInstance(text: string): NodeId {
    const id = api().createNode("div");
    api().setText(id, text);
    return id;
  },

  appendInitialChild(parent: NodeId, child: NodeId) {
    api().appendChild(parent, child);
  },

  finalizeInitialChildren() {
    return false;
  },

  prepareUpdate(_instance: NodeId, _type: string, _oldProps: any, newProps: any): UpdatePayload {
    return { props: newProps };
  },

  commitUpdate(instance: NodeId, updatePayload: UpdatePayload) {
    if (!updatePayload) return;
    const props = updatePayload.props;
    if (props?.style) {
      for (const [k, v] of Object.entries(props.style)) {
        if (v == null) continue;
        api().setStyle(instance, String(k), String(v));
      }
    }
    if (typeof props?.text === "string") api().setText(instance, props.text);
  },

  commitTextUpdate(textInstance: NodeId, _oldText: string, newText: string) {
    api().setText(textInstance, newText);
  },

  appendChild(parent: NodeId, child: NodeId) {
    api().appendChild(parent, child);
  },

  appendChildToContainer(container: NodeId, child: NodeId) {
    api().appendChild(container, child);
  },

  removeChild(parent: NodeId, child: NodeId) {
    api().removeChild(parent, child);
  },

  removeChildFromContainer(container: NodeId, child: NodeId) {
    api().removeChild(container, child);
  },

  insertBefore(parent: NodeId, child: NodeId, beforeChild: NodeId) {
    // Minimal: yoga order isn't stable yet; treat as remove+append.
    api().removeChild(parent, child);
    api().appendChild(parent, child);
    void beforeChild;
  },

  prepareForCommit() {
    return null;
  },
  resetAfterCommit() {},

  shouldSetTextContent() {
    return false;
  },
  getPublicInstance(instance: NodeId) {
    return instance;
  },

  clearContainer() {},
  preparePortalMount() {},
  detachDeletedInstance() {},

  scheduleTimeout: setTimeout,
  cancelTimeout: clearTimeout,
  noTimeout: -1,
  isPrimaryRenderer: true,
  warnsIfNotActing: false,
  getCurrentEventPriority() {
    return 0;
  },
};

const Reconciler = ReactReconciler(hostConfig);

export function mount(element: any) {
  const container = api().createNode("column");
  // react-reconciler signature differs across versions; use `any` to avoid
  // pinning to a specific arity.
  const root = (Reconciler as any).createContainer(container, 0, null, false, null, "", console.error, null, null, null);
  Reconciler.updateContainer(element, root, null, () => {});
  api().setRoot(container);
  return container;
}
