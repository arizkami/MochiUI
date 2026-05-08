import React from "react";
import ReactReconciler from "react-reconciler";

type NodeId = number;

type AureliaApi = {
  createNode:  (type: string) => NodeId;
  appendChild: (parent: NodeId, child: NodeId) => void;
  removeChild: (parent: NodeId, child: NodeId) => void;
  setText:     (id: NodeId, text: string) => void;
  setStyle:    (id: NodeId, key: string, value: string) => void;
  setRoot:     (id: NodeId) => void;
  setProp:     (id: NodeId, key: string, value: string) => void;
  setCallback: (id: NodeId, event: string, fn: Function) => void;
};

function api(): AureliaApi {
  const a = (globalThis as any).AureliaUI;
  if (!a) throw new Error("globalThis.AureliaUI is missing (native bindings not installed)");
  return a as AureliaApi;
}

// Props that never get forwarded to the native bridge
const SKIP_PROPS = new Set(["style", "children", "key", "ref"]);
// Event handlers: any prop matching /^on[A-Z]/
const EVENT_RE = /^on[A-Z]/;

// Component types whose text children are collected as the children prop rather
// than creating separate TextNode instances.
const TEXT_CONTENT_TYPES = new Set(["Text", "Button", "text", "button"]);

function applyProps(id: NodeId, props: Record<string, any> | null | undefined) {
  if (!props) return;

  // Styles
  if (props.style) {
    for (const [k, v] of Object.entries(props.style)) {
      if (v == null) continue;
      api().setStyle(id, k, String(v));
    }
  }

  // Data props and event callbacks
  for (const [k, v] of Object.entries(props)) {
    if (SKIP_PROPS.has(k) || v == null) continue;
    if (typeof v === "function" && EVENT_RE.test(k)) {
      api().setCallback(id, k, v);
    } else if (typeof v !== "object" && typeof v !== "function") {
      api().setProp(id, k, String(v));
    }
  }

  // For text-content components children arrive as a string in props.children
  if (typeof props.children === "string") {
    api().setProp(id, "text", props.children);
  }
}

// ── react-reconciler host config ──────────────────────────────────────────────

type HostContext  = null;
type UpdatePayload = { props: Record<string, any> } | null;

const hostConfig: any = {
  now: Date.now,
  supportsMutation: true,

  getRootHostContext():  HostContext { return null; },
  getChildHostContext(): HostContext { return null; },

  shouldSetTextContent(type: string): boolean {
    return TEXT_CONTENT_TYPES.has(type);
  },

  createInstance(type: string, props: Record<string, any>): NodeId {
    const id = api().createNode(type);
    applyProps(id, props);
    return id;
  },

  createTextInstance(text: string): NodeId {
    const id = api().createNode("Text");
    api().setProp(id, "text", text);
    return id;
  },

  appendInitialChild(parent: NodeId, child: NodeId) { api().appendChild(parent, child); },
  finalizeInitialChildren()                          { return false; },

  prepareUpdate(
    _instance: NodeId,
    _type: string,
    _oldProps: Record<string, any>,
    newProps: Record<string, any>,
  ): UpdatePayload {
    return { props: newProps };
  },

  commitUpdate(instance: NodeId, payload: UpdatePayload) {
    if (payload) applyProps(instance, payload.props);
  },

  commitTextUpdate(instance: NodeId, _old: string, newText: string) {
    api().setProp(instance, "text", newText);
  },

  appendChild(parent: NodeId, child: NodeId)              { api().appendChild(parent, child); },
  appendChildToContainer(container: NodeId, child: NodeId) { api().appendChild(container, child); },
  removeChild(parent: NodeId, child: NodeId)              { api().removeChild(parent, child); },
  removeChildFromContainer(container: NodeId, child: NodeId) { api().removeChild(container, child); },

  insertBefore(parent: NodeId, child: NodeId, _before: NodeId) {
    // Yoga order is rebuilt on layout; remove-then-append keeps the list stable
    api().removeChild(parent, child);
    api().appendChild(parent, child);
  },

  prepareForCommit()  { return null; },
  resetAfterCommit()  {},
  getPublicInstance(instance: NodeId) { return instance; },
  clearContainer()    {},
  preparePortalMount(){},
  detachDeletedInstance() {},

  scheduleTimeout: setTimeout,
  cancelTimeout:   clearTimeout,
  noTimeout:       -1,
  isPrimaryRenderer: true,
  warnsIfNotActing:  false,

  // React 19 / react-reconciler 0.33 priority + scheduling API
  getCurrentEventPriority()         { return 16; }, // DefaultEventPriority
  resolveUpdatePriority()           { return 16; }, // DefaultEventPriority
  getCurrentUpdatePriority()        { return 0;  },
  setCurrentUpdatePriority(_p: number) {},
  resolveEventTimeStamp()           { return -1; }, // -1 = no active event
  resolveEventType()                { return null; },
  trackSchedulerEvent()             {},
  shouldAttemptEagerTransition()    { return false; },

  // Suspense / concurrent-mode stubs (non-concurrent renderer)
  resetFormInstance(_form: any)  {},
  requestPostPaintCallback(_cb: any) {},
  maySuspendCommit()             { return false; },
  preloadInstance()              { return true;  },
  startSuspendingCommit()        {},
  suspendInstance()              {},
  waitForCommitToBeReady()       { return null;  },
};

const Reconciler = ReactReconciler(hostConfig);

export function mount(element: React.ReactElement): NodeId {
  const container = api().createNode("View");
  const root = (Reconciler as any).createContainer(
    container, 0, null, false, null, "", console.error, null, null, null,
  );
  // react-reconciler 0.33 (React 19): updateContainer is async.
  // Use the sync path so the tree is committed before we call setRoot.
  (Reconciler as any).updateContainerSync(element, root, null, null);
  (Reconciler as any).flushSyncWork();
  api().setRoot(container);
  return container;
}

// ── StyleSheet ────────────────────────────────────────────────────────────────

export const StyleSheet = {
  /** Identity function — mirrors RN's StyleSheet.create for type safety. */
  create<T extends Record<string, Record<string, any>>>(styles: T): T {
    return styles;
  },
};

// ── Style type definitions ────────────────────────────────────────────────────

export interface ViewStyle {
  flex?:            number;
  flexGrow?:        number;
  flexShrink?:      number;
  flexBasis?:       number;
  flexDirection?:   "row" | "column";
  flexWrap?:        "wrap" | "nowrap";
  alignItems?:      "flex-start" | "center" | "flex-end" | "stretch";
  justifyContent?:  "flex-start" | "center" | "flex-end" | "space-between" | "space-around" | "space-evenly";
  width?:           number | string;
  height?:          number | string;
  minWidth?:        number;
  minHeight?:       number;
  margin?:          number;
  marginTop?:       number;
  marginBottom?:    number;
  marginLeft?:      number;
  marginRight?:     number;
  marginHorizontal?: number;
  marginVertical?:   number;
  padding?:         number;
  paddingTop?:      number;
  paddingBottom?:   number;
  paddingLeft?:     number;
  paddingRight?:    number;
  paddingHorizontal?: number;
  paddingVertical?:   number;
  gap?:             number;
  backgroundColor?: string;
  borderRadius?:    number;
  overflow?:        "hidden" | "visible";
  position?:        "absolute" | "relative";
  top?:             number;
  left?:            number;
  right?:           number;
  bottom?:          number;
}

export interface TextStyle extends ViewStyle {
  color?:       string;
  fontSize?:    number;
  fontWeight?:  "normal" | "bold" | "100" | "200" | "300" | "400" | "500" | "600" | "700" | "800" | "900";
  textAlign?:   "left" | "center" | "right";
}

// ── Component prop types ──────────────────────────────────────────────────────

export interface ViewProps {
  style?:    ViewStyle;
  children?: React.ReactNode;
  onPress?:  () => void;
}

export interface TextProps {
  style?:    TextStyle;
  children?: React.ReactNode;
}

export interface ButtonProps {
  title:     string;
  onPress?:  () => void;
  style?:    ViewStyle;
}

export interface TouchableOpacityProps {
  onPress?:  () => void;
  style?:    ViewStyle;
  children?: React.ReactNode;
}

export interface ScrollViewProps {
  style?:                ViewStyle;
  contentContainerStyle?: ViewStyle;
  horizontal?:           boolean;
  children?:             React.ReactNode;
}

export interface TextInputProps {
  value?:           string;
  placeholder?:     string;
  onChangeText?:    (text: string) => void;
  onSubmitEditing?: () => void;
  style?:           ViewStyle & TextStyle;
}

export interface SwitchProps {
  value:          boolean;
  onValueChange?: (value: boolean) => void;
  style?:         ViewStyle;
}

export interface SliderProps {
  value?:         number;
  minimumValue?:  number;
  maximumValue?:  number;
  onValueChange?: (value: number) => void;
  style?:         ViewStyle;
  vertical?:      boolean;
}

export interface ImageProps {
  source?: { uri?: string } | number;
  style?:  ViewStyle;
}

export interface FlatListProps<T> {
  data:          T[];
  renderItem:    (info: { item: T; index: number }) => React.ReactElement | null;
  keyExtractor?: (item: T, index: number) => string;
  style?:        ViewStyle;
  contentContainerStyle?: ViewStyle;
  horizontal?:   boolean;
}

// ── RN-like components ────────────────────────────────────────────────────────
// Each thin wrapper renders a native-type element handled by the C++ bridge.

export const View = (props: ViewProps) =>
  React.createElement("View", props as any);

export const Text = (props: TextProps) =>
  React.createElement("Text", props as any);

export const Button = ({ title, onPress, style }: ButtonProps) =>
  React.createElement("Button", { title, onPress, style } as any);

export const TouchableOpacity = (props: TouchableOpacityProps) =>
  React.createElement("TouchableOpacity", props as any);

export const Pressable = TouchableOpacity;

export const SafeAreaView = (props: ViewProps) =>
  React.createElement("SafeAreaView", props as any);

export const ScrollView = ({ horizontal, style, contentContainerStyle, children }: ScrollViewProps) =>
  React.createElement(
    horizontal ? "HScrollView" : "ScrollView",
    { style: { ...(style ?? {}), ...(contentContainerStyle ?? {}) } } as any,
    children,
  );

export const TextInput = (props: TextInputProps) =>
  React.createElement("TextInput", props as any);

export const Switch = (props: SwitchProps) =>
  React.createElement("Switch", props as any);

export const Slider = (props: SliderProps) =>
  React.createElement("Slider", props as any);

export const Image = (props: ImageProps) =>
  React.createElement("Image", props as any);

export function FlatList<T>({
  data,
  renderItem,
  style,
  horizontal,
}: FlatListProps<T>): React.ReactElement {
  const nativeType = horizontal ? "HScrollView" : "FlatList";
  const items = data
    .map((item, index) => renderItem({ item, index }))
    .filter((el): el is React.ReactElement => el != null);
  return React.createElement(nativeType as any, { style } as any, ...items);
}
