/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Coordinate period selection, sliding active feedback, and pressed semantics.
 * Module: Web observatory / range control boundary
 */

function measurePill(switcher, buttons, activeButton) {
  if (!switcher || !activeButton || !buttons.length) return;
  const firstRect = buttons[0].getBoundingClientRect();
  const activeRect = activeButton.getBoundingClientRect();
  switcher.style.setProperty("--range-pill-width", `${activeRect.width}px`);
  switcher.style.setProperty("--range-pill-x", `${activeRect.left - firstRect.left}px`);
}

function markPressed(buttons, activeButton) {
  buttons.forEach((button) => {
    const active = button === activeButton;
    button.classList.toggle("is-active", active);
    button.setAttribute("aria-pressed", String(active));
  });
}

/**
 * Mount the period control without owning summary fetching or period state.
 * The callback remains the page orchestration boundary.
 */
export function setupRangeSwitcher({ onChange } = {}) {
  const switcher = document.querySelector(".range-switcher");
  const buttons = [...(switcher?.querySelectorAll(".range-button") || [])];
  if (!switcher || !buttons.length) return;

  let activeButton = buttons.find((button) => button.classList.contains("is-active")) || buttons[0];
  const updateIndicator = () => measurePill(switcher, buttons, activeButton);
  const setActive = (button, notify = false) => {
    if (!button || !buttons.includes(button)) return;
    activeButton = button;
    markPressed(buttons, activeButton);
    switcher.dataset.rangeReady = "true";
    // Paint the indicator in the same task as the pressed-state change so a
    // slow summary request cannot leave the old pill behind for a frame. The
    // follow-up frame still catches font, resize, and responsive layout shifts.
    updateIndicator();
    requestAnimationFrame(updateIndicator);
    if (notify) onChange?.(button.dataset.period || "day");
  };

  buttons.forEach((button) => {
    button.addEventListener("click", () => setActive(button, true));
  });
  setActive(activeButton);

  if ("ResizeObserver" in window) {
    const observer = new ResizeObserver(updateIndicator);
    observer.observe(switcher);
  } else {
    window.addEventListener("resize", updateIndicator, { passive: true });
  }
}
