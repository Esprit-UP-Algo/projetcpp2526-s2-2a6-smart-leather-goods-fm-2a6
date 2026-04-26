import unittest

from app.services.replicate_adapters import build_replicate_input_for_ref, classify_replicate_model


class ReplicateAdaptersTests(unittest.TestCase):
    def test_classify_stability_sdxl(self) -> None:
        self.assertEqual(
            classify_replicate_model("stability-ai/sdxl:abc123"),
            "stability_sdxl",
        )

    def test_classify_flux(self) -> None:
        self.assertEqual(
            classify_replicate_model("black-forest-labs/flux-schnell:deadbeef"),
            "flux",
        )

    def test_classify_lightning(self) -> None:
        self.assertEqual(
            classify_replicate_model("lucataco/sdxl-lightning-multi-controlnet:x"),
            "sdxl_lightning",
        )

    def test_stability_input_exact_schema(self) -> None:
        inp, fam = build_replicate_input_for_ref(
            "stability-ai/sdxl:v1",
            full_prompt="hello\nAvoid: bad",
            num_inference_steps=6,
            side_px=768,
            guidance_scale=3.0,
        )
        self.assertEqual(fam, "stability_sdxl")
        self.assertNotIn("scheduler", inp)
        self.assertEqual(inp["width"], 1024)
        self.assertEqual(inp["height"], 1024)
        self.assertEqual(inp["num_inference_steps"], 25)
        self.assertEqual(inp["guidance_scale"], 7.5)
        self.assertEqual(inp["negative_prompt"], "blurry, low quality, distorted")
        self.assertEqual(inp["prompt"], "hello")

    def test_lightning_input_no_scheduler(self) -> None:
        inp, fam = build_replicate_input_for_ref(
            "lucataco/foo-lightning-bar:1",
            full_prompt="x\nAvoid: y",
            num_inference_steps=8,
            side_px=512,
            guidance_scale=5.0,
        )
        self.assertEqual(fam, "sdxl_lightning")
        self.assertNotIn("scheduler", inp)
