import cors from "cors";
import express from "express";
import { runShelfMock } from "./nesting.js";
import { runWithPlaywright } from "./runner-playwright.js";
import { cutPlanRequestSchema } from "./schema.js";

const app = express();
app.use(cors());
app.use(express.json({ limit: "2mb" }));

const PORT = Number(process.env.PORT || 8010);
const ENGINE = String(process.env.NEST_ENGINE || "playwright").toLowerCase();

app.get("/health", (_req, res) => {
  res.json({
    ok: true,
    service: "svgnest-service",
    engine: ENGINE,
    ts: new Date().toISOString(),
  });
});

app.post("/generate-cut-plan", async (req, res) => {
  try {
    const parsed = cutPlanRequestSchema.safeParse(req.body);
    if (!parsed.success) {
      return res.status(400).json({
        success: false,
        error: "Payload invalide",
        details: parsed.error.issues,
      });
    }
    const payload = parsed.data;
    const timeoutMs = payload.config?.timeout_ms ?? 15000;

    const plan =
      ENGINE === "mock" ? runShelfMock(payload) : await runWithPlaywright(payload, timeoutMs);

    return res.json({
      success: true,
      algorithm: ENGINE === "mock" ? "shelf-mock-node" : "playwright-worker-shelf",
      order_id: payload.order_id,
      id_produit: payload.id_produit,
      id_matiere: payload.id_matiere,
      ...plan,
      diagnostics: {
        generated_at: new Date().toISOString(),
        engine: ENGINE,
      },
    });
  } catch (error) {
    return res.status(500).json({
      success: false,
      error: "Erreur interne svgnest-service",
      details: String(error?.message || error),
    });
  }
});

app.listen(PORT, () => {
  console.log(`[svgnest-service] listening on http://127.0.0.1:${PORT}`);
  console.log(`[svgnest-service] engine=${ENGINE}`);
});
