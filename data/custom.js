class _ {
    static css = `
/* ============================================
   MODERN DARK THEME - CSS Overrides
   Settings Library v1.3.16
   ============================================ */

body.theme_light,
body.theme_dark {
  --accent: #00b4d8;
  --back: #080a10;
  --tab: #1a1d27;
  --font: #e8eaed;
  --font_tint: #8b8fa3;
  --font_inv: #ffffff;
  --shadow: rgba(0, 0, 0, 0.4);
  --shadow_light: rgba(255, 255, 255, 0.03);
  --dark: #2a2d3a;
  --error: #ff6b6b;
  --font_fam: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
}

body {
  background: #080a10 !important;
}

.main { max-width: 480px; }

.group_col {
  background: var(--tab) !important;
  border: 1px solid rgba(255,255,255,0.06);
  border-radius: 14px !important;
  box-shadow: 0 2px 16px rgba(0,0,0,0.3) !important;
  margin: 0 8px !important;
  overflow: hidden;
}

.group_col > .row,
.group_col > .widget {
  border-color: rgba(255,255,255,0.06) !important;
}

.group_title span {
  color: var(--font_tint) !important;
  font-size: 13px !important;
  font-weight: 600 !important;
  letter-spacing: 0.8px;
  text-transform: uppercase;
  padding: 14px 14px 6px !important;
}

.page > .row > .group_row {
  background: transparent !important;
  box-shadow: none !important;
  border-radius: 0 !important;
  margin: 0 !important;
  padding: 10px 14px !important;
}

.widget_row label {
  color: var(--font) !important;
  font-weight: 400;
}

.value {
  color: var(--font_tint) !important;
}

.value.bold {
  color: var(--font) !important;
  font-weight: 600 !important;
}

.button {
  background: #00b4d8 !important;
  color: #ffffff !important;
  border-radius: 10px !important;
  font-weight: 600 !important;
  box-shadow: 0 2px 8px rgba(0,180,216,0.25) !important;
  border: none !important;
}

.button:active {
  filter: brightness(0.85) !important;
  transform: scale(0.98);
}

.buttons > .button {
  border-radius: 10px !important;
}

.group_col > .buttons:last-child:not(:only-child) {
  background: transparent !important;
  box-shadow: none !important;
  border-top: 1px solid rgba(255,255,255,0.06) !important;
  gap: 0 !important;
  padding: 0 !important;
}

.group_col > .buttons:last-child:not(:only-child) > .button {
  background: transparent !important;
  box-shadow: none !important;
  color: #00b4d8 !important;
  border-radius: 0 !important;
}

.slider {
  background: var(--dark) !important;
  background-image: linear-gradient(var(--accent), var(--accent)) !important;
  border-radius: 4px !important;
  height: 5px !important;
}

.slider::-webkit-slider-thumb {
  background: var(--accent) !important;
  border: 3px solid var(--tab) !important;
  box-shadow: 0 0 0 2px var(--accent), 0 2px 8px rgba(0,0,0,0.3) !important;
  height: 22px !important;
  width: 22px !important;
}

.switch {
  background-color: var(--dark) !important;
  border-radius: 16px !important;
  height: 28px !important;
  width: 48px !important;
}

.switch::before {
  background: var(--font_tint) !important;
  height: 22px !important;
  width: 22px !important;
  top: 3px !important;
  left: 3px !important;
}

.switch:checked {
  background-color: var(--accent) !important;
}

.switch:checked::before {
  background: #fff !important;
  left: 23px !important;
}

.select {
  background: var(--tab);
  border-radius: 8px;
  border: 1px solid rgba(255,255,255,0.06);
}

.page > .widget {
  background: var(--tab) !important;
  border: 1px solid rgba(255,255,255,0.06) !important;
  border-radius: 12px !important;
}

.tab {
  border-radius: 8px !important;
  font-weight: 500;
}

.tab.active {
  background: var(--accent) !important;
  color: var(--font_inv) !important;
}

.dialog {
  background: var(--tab) !important;
  border: 1px solid rgba(255,255,255,0.06) !important;
  border-radius: 16px !important;
}

.dialog_back {
  backdrop-filter: blur(12px) brightness(0.5) !important;
}

.popup {
  background: var(--accent) !important;
  border-radius: 12px !important;
  font-weight: 500;
}

body::-webkit-scrollbar {
  width: 5px !important;
  height: 5px !important;
}

body::-webkit-scrollbar-thumb {
  background: var(--font_tint) !important;
  border-radius: 6px !important;
}

.log {
  color: var(--accent) !important;
  background: rgba(0,180,216,0.05) !important;
  border: 1px solid rgba(255,255,255,0.06);
  border-radius: 8px !important;
  font-size: 14px !important;
}

.header .ws {
  display: none !important;
}
`;
}