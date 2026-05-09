import { createRenderer, type App, type Component } from "@vue/runtime-core";
import { api, EVENT_RE, isPrimitiveProp, NodeId, normalizeStyleInput } from "./api";

type HostNode = NodeId;
type HostElement = NodeId;
type HostContainer = NodeId;

const parentByNode = new Map<NodeId, NodeId | null>();

function applyProp(id: NodeId, key: string, value: unknown) {
  if (value == null) return;

  if (key === "style") {
    const style = normalizeStyleInput(value);
    for (const [styleKey, styleValue] of Object.entries(style)) {
      if (styleValue == null) continue;
      api().setStyle(id, styleKey, String(styleValue));
    }
    return;
  }

  if (typeof value === "function" && EVENT_RE.test(key)) {
    api().setCallback(id, key, value);
    return;
  }

  if (isPrimitiveProp(value)) {
    api().setProp(id, key, String(value));
  }
}

const renderer = createRenderer<HostNode, HostElement>({
  patchProp(el, key, _prevValue, nextValue) {
    applyProp(el, key, nextValue);
  },

  insert(child, parent, _anchor) {
    const priorParent = parentByNode.get(child);
    if (priorParent === parent) {
      return;
    }
    if (priorParent != null) {
      api().removeChild(priorParent, child);
    }
    parentByNode.set(child, parent);
    api().appendChild(parent, child);
  },

  remove(child) {
    const parent = parentByNode.get(child);
    if (parent != null) {
      api().removeChild(parent, child);
    }
    parentByNode.delete(child);
  },

  createElement(type) {
    return api().createNode(type);
  },

  createText(text) {
    const id = api().createNode("Text");
    api().setProp(id, "text", text);
    return id;
  },

  createComment(_text) {
    const id = api().createNode("Text");
    api().setProp(id, "text", "");
    return id;
  },

  setText(node, text) {
    api().setProp(node, "text", text);
  },

  setElementText(node, text) {
    api().setProp(node, "text", text);
  },

  parentNode(node) {
    return parentByNode.get(node) ?? null;
  },

  nextSibling(_node) {
    return null;
  },

  querySelector(_selector) {
    return null;
  },

  setScopeId() {},
  insertStaticContent(content, parent, _anchor) {
    const id = api().createNode("Text");
    api().setProp(id, "text", content);
    parentByNode.set(id, parent);
    api().appendChild(parent, id);
    return [id, id];
  },
});

export function mountVueApp(rootComponent: Component): HostContainer {
  const container = api().createNode("View");
  parentByNode.clear();
  const app = renderer.createApp(rootComponent) as App;
  app.mount(container);
  api().setRoot(container);
  return container;
}
