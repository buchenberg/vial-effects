// Parameter spec + display formatting for the UI.
// Mirrors src/dsp/EffectParameters.h (ranges/scales) and vial's value_bridge.h
// (skewValue -> display text). The C++ side owns the engine values; this is
// purely for rendering labels/readouts and classifying control types.

export type Scale = "linear" | "indexed" | "quadratic" | "exponential" | "squareRoot";

export interface ParamSpec {
  id: string;
  min: number;
  max: number;
  def: number;
  postOffset: number;
  displayMultiply: number;
  scale: Scale;
  displayInvert: boolean;
  unit: string;
  name: string;
}

const P = (
  id: string, min: number, max: number, def: number, postOffset: number,
  displayMultiply: number, scale: Scale, displayInvert: boolean, unit: string, name: string,
): ParamSpec => ({ id, min, max, def, postOffset, displayMultiply, scale, displayInvert, unit, name });

export const PARAMS: ParamSpec[] = [
  // Chorus
  P("chorus_on", 0, 1, 0, 0, 1, "indexed", false, "", "Chorus Switch"),
  P("chorus_dry_wet", 0, 1, 0.5, 0, 100, "linear", false, "%", "Chorus Mix"),
  P("chorus_feedback", -0.95, 0.95, 0.4, 0, 100, "linear", false, "%", "Chorus Feedback"),
  P("chorus_cutoff", 8, 136, 60, 0, 1, "linear", false, "", "Chorus Filter Cutoff"),
  P("chorus_spread", 0, 1, 1, 0, 1, "linear", false, "", "Chorus Filter Spread"),
  P("chorus_voices", 1, 4, 4, 0, 4, "indexed", false, "", "Chorus Voices"),
  P("chorus_frequency", -6, 3, -3, 0, 1, "exponential", true, " secs", "Chorus Frequency"),
  P("chorus_sync", 0, 3, 1, 0, 1, "indexed", false, "", "Chorus Sync"),
  P("chorus_tempo", 0, 10, 4, 0, 1, "indexed", false, "", "Chorus Tempo"),
  P("chorus_mod_depth", 0, 1, 0.5, 0, 100, "linear", false, "%", "Chorus Mod Depth"),
  P("chorus_delay_1", -10, -5.64386, -9, 0, 1000, "exponential", false, " ms", "Chorus Delay 1"),
  P("chorus_delay_2", -10, -5.64386, -7, 0, 1000, "exponential", false, " ms", "Chorus Delay 2"),
  // Delay
  P("delay_on", 0, 1, 0, 0, 1, "indexed", false, "", "Delay Switch"),
  P("delay_dry_wet", 0, 1, 0.3334, 0, 100, "linear", false, "%", "Delay Mix"),
  P("delay_feedback", -1, 1, 0.5, 0, 100, "linear", false, "%", "Delay Feedback"),
  P("delay_frequency", -2, 9, 2, 0, 1, "exponential", true, " secs", "Delay Frequency"),
  P("delay_style", 0, 3, 0, 0, 1, "indexed", false, "", "Delay Style"),
  P("delay_filter_cutoff", 8, 136, 60, 0, 1, "linear", false, "", "Delay Filter Cutoff"),
  P("delay_filter_spread", 0, 1, 1, 0, 1, "linear", false, "", "Delay Filter Spread"),
  P("delay_sync", 0, 3, 1, 0, 1, "indexed", false, "", "Delay Sync"),
  P("delay_tempo", 4, 12, 9, 0, 1, "indexed", false, "", "Delay Tempo"),
  // Reverb
  P("reverb_on", 0, 1, 0, 0, 1, "indexed", false, "", "Reverb Switch"),
  P("reverb_pre_low_cutoff", 0, 128, 0, 0, 1, "linear", false, "", "Low Cut"),
  P("reverb_pre_high_cutoff", 0, 128, 110, 0, 1, "linear", false, "", "High Cut"),
  P("reverb_low_shelf_cutoff", 0, 128, 0, 0, 1, "linear", false, "", "Reverb Low Cutoff"),
  P("reverb_low_shelf_gain", -6, 0, 0, 0, 1, "linear", false, " dB", "Reverb Low Gain"),
  P("reverb_high_shelf_cutoff", 0, 128, 90, 0, 1, "linear", false, "", "Reverb High Cutoff"),
  P("reverb_high_shelf_gain", -6, 0, -1, 0, 1, "linear", false, " dB", "Reverb High Gain"),
  P("reverb_dry_wet", 0, 1, 0.25, 0, 100, "linear", false, "%", "Reverb Mix"),
  P("reverb_delay", 0, 0.3, 0, 0, 1, "linear", false, " secs", "Reverb Delay"),
  P("reverb_decay_time", -6, 6, 0, 0, 1, "exponential", false, " secs", "Reverb Decay Time"),
  P("reverb_size", 0, 1, 0.5, 0, 100, "linear", false, "%", "Reverb Size"),
  P("reverb_chorus_amount", 0, 1, 0.223607, 0, 100, "quadratic", false, "%", "Reverb Chorus Amount"),
  P("reverb_chorus_frequency", -8, 3, -2, 0, 1, "exponential", false, " Hz", "Reverb Chorus Frequency"),
];

export const PARAM_BY_ID: Record<string, ParamSpec> = Object.fromEntries(
  PARAMS.map((p) => [p.id, p]),
);

export const SYNC_CHOICES = ["Seconds", "Tempo", "Tempo Dotted", "Tempo Triplets"];
export const DELAY_STYLE_CHOICES = ["Mono", "Stereo", "Ping Pong", "Mid Ping Pong"];
export const SYNCED_FREQUENCY_NAMES = [
  "Freeze", "32/1", "16/1", "8/1", "4/1", "2/1", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64",
];

export type ControlKind = "toggle" | "combo" | "slider";

export function classify(id: string): ControlKind {
  if (id.endsWith("_on")) return "toggle";
  if (id === "delay_style" || id.endsWith("_sync")) return "combo";
  return "slider";
}

// Mirror of value_bridge.h::skewValue — maps an engine (raw) value to its
// human display magnitude before applying displayMultiply/postOffset.
function skew(spec: ParamSpec, value: number): number {
  switch (spec.scale) {
    case "quadratic":
      return value * value;
    case "exponential":
      return spec.displayInvert ? 1 / Math.pow(2, value) : Math.pow(2, value);
    case "squareRoot":
      return Math.sqrt(value);
    default:
      return value;
  }
}

// Format a slider's scaled (raw) value the way vial displays it.
export function formatValue(id: string, scaled: number): string {
  const spec = PARAM_BY_ID[id];
  if (!spec) return scaled.toFixed(2);
  const display = spec.displayMultiply * skew(spec, scaled) + spec.postOffset;
  const decimals = Math.abs(display) >= 100 ? 0 : Math.abs(display) >= 10 ? 1 : 2;
  const rounded = parseFloat(display.toFixed(decimals)); // trims trailing zeros (25.0 -> 25)
  return `${rounded}${spec.unit}`;
}
