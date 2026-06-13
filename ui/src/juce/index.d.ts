// Type declarations for the vendored JUCE frontend bridge (index.js).

export interface ListenerList {
  addListener(fn: () => void): number;
  removeListener(id: number): void;
}

export interface SliderProperties {
  start: number;
  end: number;
  skew: number;
  name: string;
  label: string;
  numSteps: number;
  interval: number;
  parameterIndex: number;
}

export interface SliderState {
  name: string;
  properties: SliderProperties;
  valueChangedEvent: ListenerList;
  propertiesChangedEvent: ListenerList;
  getScaledValue(): number;
  getNormalisedValue(): number;
  setNormalisedValue(v: number): void;
  sliderDragStarted(): void;
  sliderDragEnded(): void;
}

export interface ToggleState {
  name: string;
  properties: { name: string; parameterIndex: number };
  valueChangedEvent: ListenerList;
  propertiesChangedEvent: ListenerList;
  getValue(): boolean;
  setValue(v: boolean): void;
}

export interface ComboBoxState {
  name: string;
  properties: { name: string; parameterIndex: number; choices: string[] };
  valueChangedEvent: ListenerList;
  propertiesChangedEvent: ListenerList;
  getChoiceIndex(): number;
  setChoiceIndex(i: number): void;
}

export function getSliderState(name: string): SliderState;
export function getToggleState(name: string): ToggleState;
export function getComboBoxState(name: string): ComboBoxState;
export function getNativeFunction(name: string): (...args: unknown[]) => Promise<unknown>;
