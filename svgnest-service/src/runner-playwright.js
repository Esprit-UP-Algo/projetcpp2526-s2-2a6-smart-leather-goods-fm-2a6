import path from "node:path";
import { fileURLToPath } from "node:url";
import { chromium } from "playwright";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const pageFile = path.join(__dirname, "worker-page.html");

export async function runWithPlaywright(payload, timeoutMs = 15000) {
  const browser = await chromium.launch({ headless: true });
  try {
    const context = await browser.newContext();
    const page = await context.newPage();
    await page.goto(`file://${pageFile}`);
    const result = await page.evaluate(
      async ({ req, timeout }) => window.computeNesting(req, timeout),
      { req: payload, timeout: timeoutMs }
    );
    await context.close();
    return result;
  } finally {
    await browser.close();
  }
}
