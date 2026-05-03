import unittest
from unittest.mock import AsyncMock, patch

from fastapi.testclient import TestClient

from app.main import app
from app.models.schemas import GeneratedVisualItem
from app.services.concept_visual_prompts import batch_slots_slice, capsule_cache_key
from app.services.generated_image_store import local_file_path, public_image_url, write_png_atomic


def _sample_visual_item(
    *,
    year: int = 2030,
    concept_index: int = 1,
    cache_key: str = "v13|2030|phase_3|ci1|jacket|premium|s|p|m|sil|fp123456",
    image_url: str | None = None,
    status: str = "ok",
) -> GeneratedVisualItem:
    url = image_url if image_url is not None else public_image_url(cache_key, base_url="http://testserver")
    return GeneratedVisualItem(
        year=year,
        concept_index=concept_index,
        product_type="veste",
        variant="premium",
        style="styleA",
        palette="paletteA",
        material="materialA",
        silhouette="silA",
        status=status,
        image_url=url,
        cached=True,
        cache_key=cache_key,
    )


class ImageGenerationPipelineTests(unittest.TestCase):
    def test_deterministic_concept_mapping_12(self) -> None:
        slots = batch_slots_slice(12)
        self.assertEqual(len(slots), 12)
        self.assertEqual([int(s["concept_index"]) for s in slots], list(range(1, 13)))
        self.assertEqual(len({s["category_label"] for s in slots}), 12)

    def test_cache_key_uniqueness(self) -> None:
        k1 = capsule_cache_key(
            year=2030,
            forecast_phase="phase_3",
            concept_index=1,
            product_type="jacket",
            variant="premium",
            style="x",
            palette="y",
            material="z",
            silhouette="s",
            prompt_fp="abc123",
        )
        k2 = capsule_cache_key(
            year=2031,
            forecast_phase="phase_3",
            concept_index=1,
            product_type="jacket",
            variant="premium",
            style="x",
            palette="y",
            material="z",
            silhouette="s",
            prompt_fp="abc123",
        )
        self.assertNotEqual(k1, k2)

    def test_route_contract_local_url_and_required_fields(self) -> None:
        cache_key = "v13|2030|phase_3|ci1|jacket|premium|s|p|m|sil|fp123456"
        png_like = b"\x89PNG\r\n\x1a\n" + (b"\x00" * 1500)
        ok, _ = write_png_atomic(cache_key, png_like)
        self.assertTrue(ok)
        expected_local = local_file_path(cache_key)
        self.assertTrue(expected_local.exists())

        async def _fake_generate_visuals(*_a, **_kw):
            return [_sample_visual_item(cache_key=cache_key)]

        with patch(
            "app.services.generative.generate_visuals_for_year",
            new=AsyncMock(side_effect=_fake_generate_visuals),
        ):
            client = TestClient(app)
            r = client.get("/generate-visuals-capsule", params={"year": 2030, "count": 1, "fast_mode": "true"})
            self.assertEqual(r.status_code, 200)
            payload = r.json()
            self.assertIn("items", payload)
            self.assertEqual(len(payload["items"]), 1)
            item = payload["items"][0]
            required = {
                "concept_index",
                "product_type",
                "variant",
                "category_label",
                "status",
                "image_url",
                "cache_key",
                "cache_hit",
                "prompt",
            }
            self.assertTrue(required.issubset(set(item.keys())))
            self.assertEqual(item["status"], "ok")
            self.assertIn("/static/generated/", item["image_url"])
            self.assertNotIn("replicate.com", item["image_url"])
            self.assertNotIn("pollinations.ai", item["image_url"])

    def test_generate_visuals_uses_local_pipeline(self) -> None:
        async def _fake_visuals(*_a, **_kw):
            return [
                _sample_visual_item(
                    image_url="http://testserver/static/generated/fo_x.png",
                )
            ]

        with patch(
            "app.services.generative.generate_visuals_for_year",
            new=AsyncMock(side_effect=_fake_visuals),
        ):
            client = TestClient(app)
            r = client.get("/generate-visuals", params={"year": 2030, "limit": 3})
            self.assertEqual(r.status_code, 200)
            payload = r.json()
            self.assertEqual(payload["year"], 2030)
            self.assertIn("items", payload)
            self.assertEqual(len(payload["items"]), 1)
            self.assertIn("/static/generated/", payload["items"][0]["image_url"])


if __name__ == "__main__":
    unittest.main()
