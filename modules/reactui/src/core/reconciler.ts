import ReactReconciler from "react-reconciler";
import { api, applyProps, NodeId, TEXT_CONTENT_TYPES } from "./api";

type HostContext = null;
type UpdatePayload = { props: Record<string, any> } | null;

const hostConfig: any = {
  now: Date.now,
  supportsMutation: true,

  getRootHostContext(): HostContext { return null; },
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
  finalizeInitialChildren() { return false; },

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

  appendChild(parent: NodeId, child: NodeId) { api().appendChild(parent, child); },
  appendChildToContainer(container: NodeId, child: NodeId) { api().appendChild(container, child); },
  removeChild(parent: NodeId, child: NodeId) { api().removeChild(parent, child); },
  removeChildFromContainer(container: NodeId, child: NodeId) { api().removeChild(container, child); },

  insertBefore(parent: NodeId, child: NodeId, _before: NodeId) {
    api().removeChild(parent, child);
    api().appendChild(parent, child);
  },

  prepareForCommit() { return null; },
  resetAfterCommit() {},
  getPublicInstance(instance: NodeId) { return instance; },
  clearContainer() {},
  preparePortalMount() {},
  detachDeletedInstance() {},

  scheduleTimeout: setTimeout,
  cancelTimeout: clearTimeout,
  noTimeout: -1,
  isPrimaryRenderer: true,
  warnsIfNotActing: false,

  getCurrentEventPriority() { return 16; },
  resolveUpdatePriority() { return 16; },
  getCurrentUpdatePriority() { return 0; },
  setCurrentUpdatePriority(_p: number) {},
  resolveEventTimeStamp() { return -1; },
  resolveEventType() { return null; },
  trackSchedulerEvent() {},
  shouldAttemptEagerTransition() { return false; },

  resetFormInstance(_form: any) {},
  requestPostPaintCallback(_cb: any) {},
  maySuspendCommit() { return false; },
  preloadInstance() { return true; },
  startSuspendingCommit() {},
  suspendInstance() {},
  waitForCommitToBeReady() { return null; },
};

export const Reconciler = ReactReconciler(hostConfig);
