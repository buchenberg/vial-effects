import type { ReactNode } from "react";
import { useToggle } from "../hooks";

export interface PanelProps {
  title: string;
  kind: "chorus" | "delay" | "reverb";
  onId: string;
  children: ReactNode;
}

export function Panel({ title, kind, onId, children }: PanelProps) {
  const power = useToggle(onId);
  return (
    <section className={`panel panel--${kind}`} data-panel={kind}>
      <div className="panel__rail">
        <div
          className={`panel__dot${power.value ? "" : " panel__dot--off"}`}
          style={{ background: "var(--accent)", boxShadow: "0 0 10px var(--accent)" }}
          role="switch"
          aria-checked={power.value}
          aria-label={`${title} on/off`}
          data-param={onId}
          onClick={power.toggle}
        />
        <div className="panel__title">{title}</div>
      </div>
      <div className="panel__body">{children}</div>
    </section>
  );
}
