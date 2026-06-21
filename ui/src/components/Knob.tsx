import { useRef, useState } from "react";
import { useSlider } from "../hooks";
import { PARAM_BY_ID, formatValue } from "../params";

const START_ANGLE = -135; // degrees
const END_ANGLE = 135;
const SWEEP = END_ANGLE - START_ANGLE;

function polar(cx: number, cy: number, r: number, deg: number) {
  const rad = (deg - 90) * (Math.PI / 180);
  return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) };
}

function arcPath(cx: number, cy: number, r: number, a0: number, a1: number) {
  const p0 = polar(cx, cy, r, a0);
  const p1 = polar(cx, cy, r, a1);
  const large = Math.abs(a1 - a0) > 180 ? 1 : 0;
  return `M ${p0.x} ${p0.y} A ${r} ${r} 0 ${large} 1 ${p1.x} ${p1.y}`;
}

export interface KnobProps {
  id: string;
  label: string;
  size?: number;
}

export function Knob({ id, label, size = 46 }: KnobProps) {
  const slider = useSlider(id);
  const spec = PARAM_BY_ID[id];
  const dragRef = useRef<{ startY: number; startNorm: number } | null>(null);
  const [hover, setHover] = useState(false);
  const [dragging, setDragging] = useState(false);

  const norm = slider.normalised;
  const angle = START_ANGLE + norm * SWEEP;
  const r = size / 2;
  const cx = r + 4;
  const cy = r + 4;
  const radius = r - 4;
  const pointer = polar(cx, cy, radius - 3, angle);

  const onPointerDown = (e: React.PointerEvent) => {
    (e.target as Element).setPointerCapture?.(e.pointerId);
    dragRef.current = { startY: e.clientY, startNorm: norm };
    setDragging(true);
    slider.dragStart();
  };
  const onPointerMove = (e: React.PointerEvent) => {
    if (!dragRef.current) return;
    const speed = e.shiftKey ? 600 : 180;
    const delta = (dragRef.current.startY - e.clientY) / speed;
    slider.setNormalised(dragRef.current.startNorm + delta);
  };
  const endDrag = (e: React.PointerEvent) => {
    if (!dragRef.current) return;
    dragRef.current = null;
    setDragging(false);
    slider.dragEnd();
    (e.target as Element).releasePointerCapture?.(e.pointerId);
  };
  const onWheel = (e: React.WheelEvent) => {
    const step = e.shiftKey ? 0.005 : 0.03;
    slider.dragStart();
    slider.setNormalised(norm + (e.deltaY < 0 ? step : -step));
    slider.dragEnd();
  };
  const onDoubleClick = () => {
    if (!spec) return;
    const def = (spec.def - spec.min) / (spec.max - spec.min);
    slider.dragStart();
    slider.setNormalised(def);
    slider.dragEnd();
  };

  const dim = size + 8;
  return (
    <div className="knob" data-param={id}>
      <svg
        className="knob__dial"
        width={dim}
        height={dim}
        role="slider"
        aria-label={label}
        aria-valuenow={Math.round(norm * 100)}
        onPointerDown={onPointerDown}
        onPointerMove={onPointerMove}
        onPointerUp={endDrag}
        onPointerCancel={endDrag}
        onWheel={onWheel}
        onDoubleClick={onDoubleClick}
        onMouseEnter={() => setHover(true)}
        onMouseLeave={() => setHover(false)}
      >
        <circle cx={cx} cy={cy} r={radius + 2} fill="#1c1c22" />
        <path d={arcPath(cx, cy, radius, START_ANGLE, END_ANGLE)} stroke="var(--track)" strokeWidth={3} fill="none" strokeLinecap="round" />
        {norm > 0.001 && (
          <path d={arcPath(cx, cy, radius, START_ANGLE, angle)} stroke="var(--accent)" strokeWidth={3} fill="none" strokeLinecap="round" />
        )}
        <line x1={cx} y1={cy} x2={pointer.x} y2={pointer.y} stroke="var(--text)" strokeWidth={2} strokeLinecap="round" />
      </svg>
      <div className="knob__label">{label}</div>
      <div className="knob__value">{hover || dragging ? formatValue(id, slider.scaled) : ""}</div>
    </div>
  );
}
