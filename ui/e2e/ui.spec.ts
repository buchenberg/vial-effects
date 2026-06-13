import { test, expect } from "@playwright/test";

test.describe("Vial Effects UI", () => {
  test.beforeEach(async ({ page }) => {
    await page.goto("/");
    await expect(page.locator('[data-panel="chorus"]')).toBeVisible();
  });

  test("renders the three effect panels", async ({ page }) => {
    await expect(page.locator('[data-panel="chorus"]')).toBeVisible();
    await expect(page.locator('[data-panel="delay"]')).toBeVisible();
    await expect(page.locator('[data-panel="reverb"]')).toBeVisible();
    await expect(page.locator('.panel__title', { hasText: "Chorus" })).toBeVisible();
    await expect(page.locator('.panel__title', { hasText: "Delay" })).toBeVisible();
    await expect(page.locator('.panel__title', { hasText: "Reverb" })).toBeVisible();
  });

  test("control inventory is present and interactive", async ({ page }) => {
    // representative knobs from each panel
    await expect(page.locator('[data-param="chorus_dry_wet"]')).toBeVisible();
    await expect(page.locator('[data-param="delay_feedback"]')).toBeVisible();
    await expect(page.locator('[data-param="reverb_decay_time"]')).toBeVisible();

    // delay mode combo opens and selects
    const combo = page.locator('[data-param="delay_style"]');
    await combo.click();
    await page.getByRole("option", { name: "Ping Pong", exact: true }).click();
    await expect(combo).toContainText("Ping Pong");

    // power dot toggles
    const dot = page.locator('[data-param="reverb_on"]');
    const wasOff = (await dot.getAttribute("aria-checked")) === "false";
    await dot.click();
    await expect(dot).toHaveAttribute("aria-checked", wasOff ? "true" : "false");
  });

  test("visual snapshot of the rack", async ({ page }) => {
    await expect(page.locator(".rack")).toHaveScreenshot("rack.png", { maxDiffPixelRatio: 0.02 });
  });
});
