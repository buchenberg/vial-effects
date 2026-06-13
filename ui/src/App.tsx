import { Panel } from "./components/Panel";
import { Knob } from "./components/Knob";
import { ValueBox } from "./components/ValueBox";
import { ComboBox } from "./components/ComboBox";
import { FilterCurve, DelayDecay, ReverbEq } from "./components/Visualizers";
import { SYNCED_FREQUENCY_NAMES, PARAM_BY_ID } from "./params";

const syncName = (i: number) => SYNCED_FREQUENCY_NAMES[Math.max(0, Math.min(SYNCED_FREQUENCY_NAMES.length - 1, i))];
// Vial shows the voice count as pairs × display_multiply (4) — e.g. 4 -> "16".
const voiceCount = (v: number) => `${v * PARAM_BY_ID.chorus_voices.displayMultiply}`;

export default function App() {
  return (
    <div className="rack">
      <Panel title="Chorus" kind="chorus" onId="chorus_on">
        <div className="col">
          <div className="row">
            <ValueBox id="chorus_voices" label="Voices" format={voiceCount} />
            <ValueBox id="chorus_tempo" label="Tempo" format={syncName} />
          </div>
          <div className="row">
            <Knob id="chorus_mod_depth" label="Depth" />
            <Knob id="chorus_delay_1" label="Delay 1" />
            <Knob id="chorus_delay_2" label="Delay 2" />
          </div>
        </div>
        <FilterCurve cutoffId="chorus_cutoff" />
        <div className="col">
          <div className="row">
            <Knob id="chorus_feedback" label="Feedback" />
            <Knob id="chorus_dry_wet" label="Mix" />
          </div>
          <div className="row">
            <Knob id="chorus_cutoff" label="Cutoff" />
            <Knob id="chorus_spread" label="Spread" />
          </div>
        </div>
      </Panel>

      <Panel title="Delay" kind="delay" onId="delay_on">
        <div className="col">
          <ValueBox id="delay_tempo" label="Tempo" format={syncName} />
          <ComboBox id="delay_style" label="Mode" />
        </div>
        <DelayDecay feedbackId="delay_feedback" />
        <div className="col">
          <div className="row">
            <Knob id="delay_feedback" label="Feedback" />
            <Knob id="delay_dry_wet" label="Mix" />
          </div>
          <div className="row">
            <Knob id="delay_filter_cutoff" label="Cutoff" />
            <Knob id="delay_filter_spread" label="Spread" />
          </div>
        </div>
      </Panel>

      <Panel title="Reverb" kind="reverb" onId="reverb_on">
        <div className="col">
          <Knob id="reverb_pre_low_cutoff" label="Low Cut" />
          <Knob id="reverb_pre_high_cutoff" label="High Cut" />
        </div>
        <ReverbEq
          lowCutId="reverb_low_shelf_cutoff"
          lowGainId="reverb_low_shelf_gain"
          highCutId="reverb_high_shelf_cutoff"
          highGainId="reverb_high_shelf_gain"
        />
        <div className="col">
          <div className="row">
            <Knob id="reverb_low_shelf_cutoff" label="Low Freq" />
            <Knob id="reverb_high_shelf_cutoff" label="High Freq" />
            <Knob id="reverb_chorus_amount" label="Chor Amt" />
            <Knob id="reverb_delay" label="Delay" />
            <Knob id="reverb_dry_wet" label="Mix" />
          </div>
          <div className="row">
            <Knob id="reverb_low_shelf_gain" label="Low Gain" />
            <Knob id="reverb_high_shelf_gain" label="High Gain" />
            <Knob id="reverb_chorus_frequency" label="Chor Freq" />
            <Knob id="reverb_size" label="Size" />
            <Knob id="reverb_decay_time" label="Time" />
          </div>
        </div>
      </Panel>
    </div>
  );
}
