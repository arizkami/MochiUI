export type NodeId = number;

export type SphereApi = {
  createNode: (type: string) => NodeId;
  appendChild: (parent: NodeId, child: NodeId) => void;
  removeChild: (parent: NodeId, child: NodeId) => void;
  setText: (id: NodeId, text: string) => void;
  setStyle: (id: NodeId, key: string, value: string) => void;
  setRoot: (id: NodeId) => void;
  setProp: (id: NodeId, key: string, value: string) => void;
  setCallback: (id: NodeId, event: string, fn: Function) => void;
};

export function api(): SphereApi {
  const nativeApi = (globalThis as any).SphereUI;
  if (!nativeApi) {
    throw new Error("globalThis.SphereUI is missing (native bindings not installed)");
  }
  return nativeApi as SphereApi;
}

export const EVENT_RE = /^on[A-Z]/;

export function isPrimitiveProp(value: unknown): value is string | number | boolean {
  return typeof value === "string" || typeof value === "number" || typeof value === "boolean";
}

export function normalizeStyleInput(input: unknown): Record<string, unknown> {
  if (!input) return {};
  if (Array.isArray(input)) {
    return input.reduce<Record<string, unknown>>((acc, item) => {
      Object.assign(acc, normalizeStyleInput(item));
      return acc;
    }, {});
  }
  if (typeof input === "object") {
    return input as Record<string, unknown>;
  }
  return {};
}
