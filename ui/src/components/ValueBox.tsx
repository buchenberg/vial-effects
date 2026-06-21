import { useRef } from "react";
import { useSlider } from "../hooks";
import { PARAM_BY_ID } from "../params";

export interface ValueBoxProps {
  id: string;
  label: string;
  format: (intValue: number) => string; // maps the indexed value to display text
}

// An indexed-parameter box (e.g. VOICES, TEMPO note). Drag vertically or scroll
// to step through integer values; the display is produced by `format`.
export function ValueBox({ id, label, format }: ValueBoxProps) {
  const slider = useSlider(id);
  const spec = PARAM_BY_ID[id];
  const dragRef = useRef<{ startY: number; startVal: number } | null>(null);

  const value = Math.round(slider.scaled);
  const setInt = (v: number) => {
    const clamped = Math.max(spec.min, Math.min(spec.max, v));
    const norm = (clamped - spec.min) / (spec.max - spec.min);
    slider.setNormalised(norm);
  };

  const onPointerDown = (e: React.PointerEvent) => {
    (e.target as Element).setPointerCapture?.(e.pointerId);
    dragRef.current = { startY: e.clientY, startVal: value };
    slider.dragStart();
  };
  const onPointerMove = (e: React.PointerEvent) => {
    if (!dragRef.current) return;
    const steps = Math.round((dragRef.current.startY - e.clientY) / 16);
    setInt(dragRef.current.startVal + steps);
  };
  const endDrag = (e: React.PointerEvent) => {
    if (!dragRef.current) return;
    dragRef.current = null;
    slider.dragEnd();
    (e.target as Element).releasePointerCapture?.(e.pointerId);
  };
  const onWheel = (e: React.WheelEvent) => {
    slider.dragStart();
    setInt(value + (e.deltaY < 0 ? 1 : -1));
    slider.dragEnd();
  };
  const onDoubleClick = () => {
    if (!spec) return;
    setInt(spec.def);
  };

  return (
    <div
      className="box"
      data-param={id}
      role="spinbutton"
      aria-label={label}
      aria-valuenow={value}
      onPointerDown={onPointerDown}
      onPointerMove={onPointerMove}
      onPointerUp={endDrag}
      onPointerCancel={endDrag}
      onWheel={onWheel}
      onDoubleClick={onDoubleClick}
    >
      <div className="box__value">{format(value)}</div>
      <div className="box__label">{label}</div>
    </div>
  );
}
