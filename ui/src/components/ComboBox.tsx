import { useState } from "react";
import { useCombo } from "../hooks";

export interface ComboBoxProps {
  id: string;
  label: string;
}

export function ComboBox({ id, label }: ComboBoxProps) {
  const combo = useCombo(id);
  const [open, setOpen] = useState(false);
  const current = combo.choices[combo.index] ?? "";

  return (
    <div
      className="box combo"
      data-param={id}
      role="combobox"
      aria-label={label}
      aria-expanded={open}
      onClick={() => setOpen((o) => !o)}
    >
      <div className="box__value" style={{ fontSize: 18 }}>{current}</div>
      <div className="box__label">{label}</div>
      {open && (
        <div className="combo__menu" role="listbox">
          {combo.choices.map((c, i) => (
            <div
              key={c}
              role="option"
              aria-selected={i === combo.index}
              className={`combo__item${i === combo.index ? " combo__item--active" : ""}`}
              onClick={(e) => {
                e.stopPropagation();
                combo.set(i);
                setOpen(false);
              }}
            >
              {c}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
