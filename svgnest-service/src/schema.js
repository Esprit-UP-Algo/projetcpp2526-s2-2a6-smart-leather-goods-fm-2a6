import { z } from "zod";

export const pieceSchema = z.object({
  name: z.string().min(1),
  width_mm: z.number().positive(),
  height_mm: z.number().positive(),
  qty: z.number().int().min(1),
  can_rotate: z.boolean().default(true),
});

export const cutPlanRequestSchema = z.object({
  order_id: z.number().int().nonnegative(),
  id_produit: z.number().int().positive(),
  id_matiere: z.number().int().positive(),
  spacing_mm: z.number().min(0).default(4),
  sheet_width_mm: z.number().positive(),
  sheet_height_mm: z.number().positive(),
  pieces: z.array(pieceSchema).min(1),
  config: z
    .object({
      rotations: z.number().int().min(1).max(36).optional(),
      populationSize: z.number().int().min(2).max(100).optional(),
      mutationRate: z.number().int().min(1).max(100).optional(),
      useHoles: z.boolean().optional(),
      exploreConcave: z.boolean().optional(),
      timeout_ms: z.number().int().min(1000).max(120000).optional(),
    })
    .optional(),
});
