// React hooks wrapping the JUCE relay state objects (see src/juce/index.js).
import { useEffect, useState, useCallback } from "react";
import {
  getSliderState,
  getToggleState,
  getComboBoxState,
  type SliderState,
  type ToggleState,
  type ComboBoxState,
} from "./juce/index.js";

/** Subscribe a component to a state object's valueChangedEvent. */
function useValueSubscription(state: { valueChangedEvent: { addListener(f: () => void): number; removeListener(id: number): void } }): number {
  const [tick, setTick] = useState(0);
  useEffect(() => {
    const id = state.valueChangedEvent.addListener(() => setTick((t) => t + 1));
    // Re-read once after subscribing: the backend's initial value can arrive
    // between first render and listener attachment, and that event would
    // otherwise be missed (leaving the control stuck at its default).
    setTick((t) => t + 1);
    return () => state.valueChangedEvent.removeListener(id);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [state]);
  return tick;
}

export interface SliderControl {
  scaled: number;
  normalised: number;
  start: number;
  end: number;
  setNormalised: (v: number) => void;
  dragStart: () => void;
  dragEnd: () => void;
  state: SliderState;
}

export function useSlider(id: string): SliderControl {
  const [state] = useState<SliderState>(() => getSliderState(id));
  useValueSubscription(state);
  const setNormalised = useCallback((v: number) => state.setNormalisedValue(Math.max(0, Math.min(1, v))), [state]);
  return {
    scaled: state.getScaledValue(),
    normalised: state.getNormalisedValue(),
    start: state.properties.start,
    end: state.properties.end,
    setNormalised,
    dragStart: () => state.sliderDragStarted(),
    dragEnd: () => state.sliderDragEnded(),
    state,
  };
}

export interface ToggleControl {
  value: boolean;
  toggle: () => void;
  set: (v: boolean) => void;
}

export function useToggle(id: string): ToggleControl {
  const [state] = useState<ToggleState>(() => getToggleState(id));
  useValueSubscription(state);
  return {
    value: state.getValue(),
    toggle: () => state.setValue(!state.getValue()),
    set: (v: boolean) => state.setValue(v),
  };
}

export interface ComboControl {
  index: number;
  choices: string[];
  set: (i: number) => void;
}

export function useCombo(id: string): ComboControl {
  const [state] = useState<ComboBoxState>(() => getComboBoxState(id));
  useValueSubscription(state);
  return {
    index: state.getChoiceIndex(),
    choices: state.properties.choices ?? [],
    set: (i: number) => state.setChoiceIndex(i),
  };
}
