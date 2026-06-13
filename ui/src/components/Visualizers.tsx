import { useEffect, useRef } from "react";
import { useSlider } from "../hooks";

type DrawFn = (ctx: CanvasRenderingContext2D, w: number, h: number, accent: string) => void;

function useCanvas(draw: DrawFn, deps: unknown[]) {
  const ref = useRef<HTMLCanvasElement>(null);
  useEffect(() => {
    const canvas = ref.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const w = Math.max(1, Math.floor(rect.width));
    const h = Math.max(1, Math.floor(rect.height));
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    let ctx: CanvasRenderingContext2D | null = null;
    try {
      ctx = canvas.getContext("2d");
    } catch {
      return; // canvas 2d unavailable (e.g. jsdom in unit tests)
    }
    if (!ctx) return;
    ctx.scale(dpr, dpr);
    ctx.clearRect(0, 0, w, h);
    const accent = getComputedStyle(canvas).getPropertyValue("--accent").trim() || "#9b87f5";
    draw(ctx, w, h, accent);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, deps);
  return ref;
}

/** Chorus: low-pass response curve driven by cutoff (MIDI note). */
export function FilterCurve({ cutoffId }: { cutoffId: string }) {
  const cutoff = useSlider(cutoffId);
  const norm = (cutoff.scaled - 8) / (136 - 8);
  const ref = useCanvas((ctx, w, h, col) => {
    const corner = 0.15 * w + norm * 0.7 * w;
    const trace = () => {
      ctx.beginPath();
      ctx.moveTo(0, h * 0.25);
      ctx.lineTo(corner, h * 0.25);
      ctx.quadraticCurveTo(corner + 0.12 * w, h * 0.25, corner + 0.18 * w, h);
    };
    trace();
    ctx.lineTo(0, h);
    ctx.closePath();
    ctx.fillStyle = col + "55";
    ctx.fill();
    trace();
    ctx.strokeStyle = col;
    ctx.lineWidth = 2;
    ctx.stroke();
  }, [norm]);
  return <canvas ref={ref} className="viz" data-viz="chorus" />;
}

/** Delay: decaying impulse train driven by feedback amount. */
export function DelayDecay({ feedbackId }: { feedbackId: string }) {
  const fb = useSlider(feedbackId);
  const amount = Math.min(0.98, Math.abs(fb.scaled));
  const ref = useCanvas((ctx, w, h, col) => {
    ctx.fillStyle = col;
    const bars = 14;
    let level = 1;
    for (let i = 0; i < bars; i++) {
      const x = 6 + (i * (w - 12)) / bars;
      const bh = level * (h - 12);
      ctx.globalAlpha = Math.max(0.2, level);
      ctx.fillRect(x, h - 6 - bh, 4, bh);
      level *= amount;
    }
    ctx.globalAlpha = 1;
  }, [amount]);
  return <canvas ref={ref} className="viz" data-viz="delay" />;
}

/** Reverb: low + high shelf EQ response curve. */
export function ReverbEq({ lowCutId, lowGainId, highCutId, highGainId }: {
  lowCutId: string; lowGainId: string; highCutId: string; highGainId: string;
}) {
  const lc = useSlider(lowCutId).scaled / 128;
  const lg = useSlider(lowGainId).scaled;
  const hc = useSlider(highCutId).scaled / 128;
  const hg = useSlider(highGainId).scaled;
  const ref = useCanvas((ctx, w, h, col) => {
    const mid = h * 0.5;
    const gainToY = (db: number) => mid - (db / 6) * (h * 0.35);
    ctx.beginPath();
    for (let px = 0; px <= w; px++) {
      const x = px / w;
      let db = 0;
      if (x < lc) db += lg * (1 - x / Math.max(0.001, lc));
      if (x > hc) db += hg * ((x - hc) / Math.max(0.001, 1 - hc));
      const y = gainToY(db);
      if (px === 0) ctx.moveTo(px, y); else ctx.lineTo(px, y);
    }
    ctx.strokeStyle = col;
    ctx.lineWidth = 2;
    ctx.stroke();
    ctx.fillStyle = col;
    ctx.beginPath(); ctx.arc(lc * w, gainToY(lg), 5, 0, Math.PI * 2); ctx.fill();
    ctx.beginPath(); ctx.arc(hc * w, gainToY(hg), 5, 0, Math.PI * 2); ctx.fill();
  }, [lc, lg, hc, hg]);
  return <canvas ref={ref} className="viz" data-viz="reverb" />;
}
