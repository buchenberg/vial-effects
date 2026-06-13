// Dev/test mock of the native JUCE backend.
//
// In the real plugin JUCE injects `window.__JUCE__` before any page script runs,
// so this module detects that and does nothing. In the Vite dev server, Vitest,
// and Playwright there is no native side, so we install a faithful stand-in that
// speaks the same event protocol as check_native_interop.js's Backend: the
// frontend emits `requestInitialUpdate` / `valueChanged`; we reply through
// `emitByBackend` with `propertiesChanged` + `valueChanged`.

import { PARAMS, PARAM_BY_ID, classify, SYNC_CHOICES, DELAY_STYLE_CHOICES } from "../params";

type EventObj = Record<string, unknown>;
type Fn = (e: EventObj) => void;

class MockBackend {
  private map = new Map<string, Map<number, Fn>>();
  private nextId = 0;

  addEventListener(eventId: string, fn: Fn): [string, number] {
    if (!this.map.has(eventId)) this.map.set(eventId, new Map());
    const id = this.nextId++;
    this.map.get(eventId)!.set(id, fn);
    return [eventId, id];
  }

  removeEventListener([eventId, id]: [string, number]) {
    this.map.get(eventId)?.delete(id);
  }

  emitByBackend(eventId: string, objectJson: string) {
    const obj = JSON.parse(objectJson);
    this.map.get(eventId)?.forEach((fn) => fn(obj));
  }

  // Frontend -> "native"
  emitEvent(eventId: string, payload: EventObj) {
    const type = payload?.eventType;
    if (type === "requestInitialUpdate") {
      this.sendInitial(eventId);
    } else if (type === "valueChanged") {
      // Store, then echo back like the native relay does so the frontend state
      // (and any subscribed React components) update.
      store.set(eventId, payload.value);
      this.emitByBackend(eventId, JSON.stringify({ eventType: "valueChanged", value: payload.value }));
    }
    // sliderDragStarted/Ended: ignored
  }

  private sendInitial(eventId: string) {
    const push = (o: EventObj) => this.emitByBackend(eventId, JSON.stringify(o));

    if (eventId.startsWith("__juce__slider")) {
      const id = eventId.slice("__juce__slider".length);
      const spec = PARAM_BY_ID[id];
      if (!spec) return;
      const indexed = spec.scale === "indexed";
      push({
        eventType: "propertiesChanged",
        start: spec.min, end: spec.max, skew: 1,
        name: spec.name, label: "",
        numSteps: indexed ? spec.max - spec.min + 1 : 100,
        interval: indexed ? 1 : 0,
        parameterIndex: PARAMS.indexOf(spec),
      });
      push({ eventType: "valueChanged", value: store.get(eventId) ?? spec.def });
    } else if (eventId.startsWith("__juce__toggle")) {
      const id = eventId.slice("__juce__toggle".length);
      const spec = PARAM_BY_ID[id];
      if (!spec) return;
      push({ eventType: "propertiesChanged", name: spec.name, parameterIndex: PARAMS.indexOf(spec) });
      push({ eventType: "valueChanged", value: store.get(eventId) ?? spec.def >= 0.5 });
    } else if (eventId.startsWith("__juce__comboBox")) {
      const id = eventId.slice("__juce__comboBox".length);
      const spec = PARAM_BY_ID[id];
      if (!spec) return;
      const choices = id.endsWith("_sync") ? SYNC_CHOICES : DELAY_STYLE_CHOICES;
      push({ eventType: "propertiesChanged", name: spec.name, parameterIndex: PARAMS.indexOf(spec), choices });
      const def = Math.round(spec.def);
      const normalised = choices.length > 1 ? def / (choices.length - 1) : 0;
      push({ eventType: "valueChanged", value: store.get(eventId) ?? normalised });
    }
  }
}

const store = new Map<string, unknown>();

export function installMockBackend() {
  const w = window as unknown as { __JUCE__?: unknown };
  if (w.__JUCE__) return; // real native backend present

  const sliders = PARAMS.filter((p) => classify(p.id) === "slider").map((p) => p.id);
  const toggles = PARAMS.filter((p) => classify(p.id) === "toggle").map((p) => p.id);
  const comboBoxes = PARAMS.filter((p) => classify(p.id) === "combo").map((p) => p.id);

  (window as unknown as { __JUCE__: unknown }).__JUCE__ = {
    postMessage: () => {},
    initialisationData: {
      __juce__platform: ["web-mock"],
      __juce__functions: [],
      __juce__registeredGlobalEventIds: [],
      __juce__sliders: sliders,
      __juce__toggles: toggles,
      __juce__comboBoxes: comboBoxes,
    },
    backend: new MockBackend(),
  };
}

installMockBackend();
