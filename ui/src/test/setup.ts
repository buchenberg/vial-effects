// Vitest setup: install the mock JUCE backend before any component imports the
// bridge, and add jest-dom matchers.
import "../juce/mock";
import "@testing-library/jest-dom/vitest";
