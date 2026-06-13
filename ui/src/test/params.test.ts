import { describe, it, expect } from "vitest";
import { PARAMS, classify, formatValue } from "../params";

describe("param table", () => {
  it("has 34 unique ids matching the C++ spec count", () => {
    expect(PARAMS.length).toBe(34);
    expect(new Set(PARAMS.map((p) => p.id)).size).toBe(34);
  });

  it("classifies control kinds", () => {
    expect(classify("chorus_on")).toBe("toggle");
    expect(classify("delay_style")).toBe("combo");
    expect(classify("chorus_sync")).toBe("combo");
    expect(classify("reverb_size")).toBe("slider");
  });
});

describe("formatValue mirrors vial display scaling", () => {
  it("linear percentage", () => {
    expect(formatValue("reverb_dry_wet", 0.25)).toBe("25%");
  });

  it("exponential seconds (decay = 2^x)", () => {
    expect(formatValue("reverb_decay_time", 0)).toBe("1 secs"); // 2^0 = 1 s
    expect(formatValue("reverb_decay_time", 1)).toBe("2 secs"); // 2^1 = 2 s
  });

  it("exponential inverted seconds (frequency display = 1 / 2^x)", () => {
    // delay_frequency raw 0 -> 1/2^0 = 1 s
    expect(formatValue("delay_frequency", 0).startsWith("1")).toBe(true);
  });

  it("quadratic percentage (x^2 * 100)", () => {
    expect(formatValue("reverb_chorus_amount", 0.5)).toBe("25%"); // 0.5^2 * 100
  });
});
