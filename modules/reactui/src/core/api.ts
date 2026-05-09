export type NodeId = number;

export type SphereApi = {
  createNode:  (type: string) => NodeId;
  appendChild: (parent: NodeId, child: NodeId) => void;
  removeChild: (parent: NodeId, child: NodeId) => void;
  setText:     (id: NodeId, text: string) => void;
  setStyle:    (id: NodeId, key: string, value: string) => void;
  setRoot:     (id: NodeId) => void;
  setProp:     (id: NodeId, key: string, value: string) => void;
  setCallback: (id: NodeId, event: string, fn: Function) => void;
};

export function api(): SphereApi {
  const nativeApi = (globalThis as any).SphereUI;
  if (!nativeApi) {
    throw new Error("globalThis.SphereUI is missing (native bindings not installed)");
  }
  return nativeApi as SphereApi;
}

export const SKIP_PROPS = new Set(["style", "children", "key", "ref"]);
export const EVENT_RE = /^on[A-Z]/;
export const TEXT_CONTENT_TYPES = new Set(["Text", "Button", "text", "button"]);

export function applyProps(id: NodeId, props: Record<string, any> | null | undefined) {
  if (!props) return;

  if (props.style) {
    for (const [k, v] of Object.entries(props.style)) {
      if (v == null) continue;
      api().setStyle(id, k, String(v));
    }
  }

  for (const [k, v] of Object.entries(props)) {
    if (SKIP_PROPS.has(k) || v == null) continue;
    if (typeof v === "function" && EVENT_RE.test(k)) {
      api().setCallback(id, k, v);
    } else if (typeof v !== "object" && typeof v !== "function") {
      api().setProp(id, k, String(v));
    }
  }

  if (typeof props.children === "string") {
    api().setProp(id, "text", props.children);
  }
}
