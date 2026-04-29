function colorFor(name) {
  const n = String(name || "").toLowerCase();
  if (n.includes("face")) return "#5b6ee1";
  if (n.includes("dos")) return "#ef6a8b";
  if (n.includes("rabat")) return "#ffaf5f";
  if (n.includes("poche")) return "#48e38f";
  if (n.includes("band")) return "#76a7ff";
  return "#8e99f3";
}

export function runShelfMock(input) {
  const spacing = input.spacing_mm ?? 4;
  const sheetW = input.sheet_width_mm;
  const sheetH = input.sheet_height_mm;
  const units = [];
  for (const p of input.pieces) {
    for (let i = 0; i < p.qty; i += 1) units.push(p);
  }
  units.sort((a, b) => Math.max(b.width_mm, b.height_mm) - Math.max(a.width_mm, a.height_mm));

  const placements = [];
  let sheet = 0;
  let x = 0;
  let y = 0;
  let shelfH = 0;
  let used = 0;

  for (const p of units) {
    let w = p.width_mm;
    let h = p.height_mm;
    let rotated = false;
    if (p.can_rotate && x + w > sheetW && x + h <= sheetW && y + w <= sheetH) {
      [w, h] = [h, w];
      rotated = true;
    }
    if (x + w > sheetW) {
      x = 0;
      y += shelfH + spacing;
      shelfH = 0;
    }
    if (y + h > sheetH) {
      sheet += 1;
      x = 0;
      y = 0;
      shelfH = 0;
    }
    placements.push({
      name: p.name,
      sheet_index: sheet,
      x_mm: +x.toFixed(3),
      y_mm: +y.toFixed(3),
      w_mm: +w.toFixed(3),
      h_mm: +h.toFixed(3),
      rotated,
      color: colorFor(p.name),
    });
    used += w * h;
    x += w + spacing;
    shelfH = Math.max(shelfH, h);
  }

  const sheetsUsed = sheet + 1;
  const total = sheetW * sheetH * sheetsUsed;
  const waste = Math.max(0, total - used);
  const wastePct = total > 0 ? (waste * 100) / total : 0;

  return {
    placements,
    sheets_used: sheetsUsed,
    sheet_width_mm: sheetW,
    sheet_height_mm: sheetH,
    used_area_mm2: +used.toFixed(3),
    total_area_mm2: +total.toFixed(3),
    waste_area_mm2: +waste.toFixed(3),
    waste_pct: +wastePct.toFixed(3),
    waste_percentage: +wastePct.toFixed(3),
  };
}
