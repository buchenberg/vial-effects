import { describe, it, expect } from "vitest";
import { render, screen, fireEvent } from "@testing-library/react";
import App from "../App";
import { Knob } from "../components/Knob";
import { ComboBox } from "../components/ComboBox";
import { getSliderState, getToggleState, getComboBoxState } from "../juce/index.js";

describe("App layout", () => {
  it("renders the three effect panels", () => {
    render(<App />);
    expect(document.querySelector('[data-panel="chorus"]')).toBeInTheDocument();
    expect(document.querySelector('[data-panel="delay"]')).toBeInTheDocument();
    expect(document.querySelector('[data-panel="reverb"]')).toBeInTheDocument();
  });

  it("renders the expected control inventory per panel", () => {
    render(<App />);
    // a representative knob from each panel
    expect(document.querySelector('[data-param="chorus_dry_wet"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="delay_feedback"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="reverb_decay_time"]')).toBeInTheDocument();
    // delay mode combo + chorus voices box
    expect(document.querySelector('[data-param="delay_style"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="chorus_voices"]')).toBeInTheDocument();
  });
});

describe("control bindings", () => {
  it("scrolling a knob updates the bound slider value", () => {
    render(<Knob id="reverb_size" label="Size" />);
    const dial = screen.getByRole("slider", { name: "Size" });
    const before = getSliderState("reverb_size").getScaledValue();
    fireEvent.wheel(dial, { deltaY: -100 }); // scroll up -> increase
    expect(getSliderState("reverb_size").getScaledValue()).toBeGreaterThan(before);
  });

  it("panel power dot toggles the *_on parameter", () => {
    render(<App />);
    const dot = document.querySelector('[data-param="reverb_on"]') as HTMLElement;
    const before = getToggleState("reverb_on").getValue();
    fireEvent.click(dot);
    expect(getToggleState("reverb_on").getValue()).toBe(!before);
  });

  it("combo selects a delay mode", () => {
    render(<ComboBox id="delay_style" label="Mode" />);
    fireEvent.click(screen.getByRole("combobox", { name: "Mode" }));
    const option = screen.getByRole("option", { name: "Ping Pong" });
    fireEvent.click(option);
    expect(getComboBoxState("delay_style").getChoiceIndex()).toBe(2);
  });
});
