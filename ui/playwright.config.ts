import { defineConfig, devices } from "@playwright/test";

// Boots the Vite dev server (which auto-installs the mock JUCE backend) and runs
// the UI end-to-end against headless Chromium.
export default defineConfig({
  testDir: "./e2e",
  fullyParallel: true,
  reporter: "list",
  use: {
    baseURL: "http://localhost:5173",
    viewport: { width: 1000, height: 780 },
  },
  projects: [{ name: "chromium", use: { ...devices["Desktop Chrome"] } }],
  webServer: {
    command: "npm run dev",
    url: "http://localhost:5173",
    reuseExistingServer: !process.env.CI,
    timeout: 60000,
  },
});
