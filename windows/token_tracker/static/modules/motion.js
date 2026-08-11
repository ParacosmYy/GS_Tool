/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Own motion state, reveal behavior, number transitions, and pointer feedback.
 */

// Opt into reveal opacity only after this module is available. If a script or
// browser feature fails, the CSS fallback keeps all content fully readable.
document.documentElement.classList.add("motion-ready");

export function setMotionState(element, state) {
  if (!element) return;
  element.dataset.motionState = state;
  element.setAttribute("aria-busy", state === "loading" ? "true" : "false");
}

export function animateNumber(element, value, duration, formatNumber, activeAnimations) {
  if (!element) return;
  const target = Number(value || 0);
  const previousFrame = activeAnimations.get(element);
  if (previousFrame) cancelAnimationFrame(previousFrame);
  const startedAt = performance.now();
  const from = Number(element.dataset.numberValue || 0);
  const ease = (progress) => 1 - Math.pow(1 - progress, 4);
  const render = (now) => {
    const progress = Math.min(1, (now - startedAt) / (duration || 850));
    const current = Math.round(from + (target - from) * ease(progress));
    element.textContent = formatNumber(current);
    element.dataset.numberValue = String(current);
    if (progress < 1) {
      const frame = requestAnimationFrame(render);
      activeAnimations.set(element, frame);
    } else {
      activeAnimations.delete(element);
    }
  };
  const frame = requestAnimationFrame(render);
  activeAnimations.set(element, frame);
}

export function setupReveal() {
  // Reveal only changes presentation. Content is already in the DOM so
  // reduced-motion users, keyboard users, and screen readers see everything.
  const elements = document.querySelectorAll("[data-reveal]");
  if (!("IntersectionObserver" in window)) {
    elements.forEach((element) => element.classList.add("is-visible"));
    return;
  }
  const observer = new IntersectionObserver((entries, instance) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      entry.target.classList.add("is-visible");
      instance.unobserve(entry.target);
    });
  }, { threshold: .12, rootMargin: "0px 0px -8% 0px" });
  elements.forEach((element, index) => {
    if (element.hidden) {
      // Dynamic disclosure panels are hidden at bootstrap; when they open,
      // their readable state must not depend on a missed observer callback.
      element.classList.add("is-visible");
      return;
    }
    element.style.transitionDelay = `${Math.min(index * 70, 350)}ms`;
    observer.observe(element);
    // Make the first viewport deterministic even when a browser delays the
    // initial observer callback while fonts or charts are still settling.
    const bounds = element.getBoundingClientRect();
    if (bounds.top < window.innerHeight * .92 && bounds.bottom > 0) element.classList.add("is-visible");
  });
}

export function setupPointerFollower() {
  const aura = document.querySelector(".pointer-aura");
  const ring = document.querySelector(".pointer-ring");
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  const finePointer = window.matchMedia("(pointer: fine)").matches;
  if (!aura || !ring || reducedMotion || !finePointer) return;

  let targetX = window.innerWidth / 2;
  let targetY = window.innerHeight / 2;
  let auraX = targetX;
  let auraY = targetY;
  let ringX = targetX;
  let ringY = targetY;
  let active = false;
  let frame = 0;
  let pressTimer = 0;
  const interactiveSelector = "a, button, input, textarea, select, summary, [role='button']";

  const move = (event) => {
    targetX = event.clientX;
    targetY = event.clientY;
    if (!active) {
      active = true;
      document.body.classList.add("pointer-ready");
      if (!frame) frame = window.requestAnimationFrame(render);
    }
  };
  const leave = () => {
    active = false;
    document.body.classList.remove("pointer-ready");
    document.body.classList.remove("pointer-interactive", "pointer-press");
    if (pressTimer) window.clearTimeout(pressTimer);
    if (frame) {
      window.cancelAnimationFrame(frame);
      frame = 0;
    }
  };
  const updateInteractiveState = (event) => {
    const target = event.target instanceof Element ? event.target.closest(interactiveSelector) : null;
    const enabled = target && !target.hasAttribute("disabled") && target.getAttribute("aria-disabled") !== "true";
    document.body.classList.toggle("pointer-interactive", Boolean(enabled));
  };
  const press = () => {
    document.body.classList.add("pointer-press");
    if (pressTimer) window.clearTimeout(pressTimer);
    pressTimer = window.setTimeout(() => document.body.classList.remove("pointer-press"), 240);
  };
  const render = () => {
    // Two spring factors create a small depth cue without hijacking the
    // native cursor or adding layout work to cards and charts.
    auraX += (targetX - auraX) * .075;
    auraY += (targetY - auraY) * .075;
    ringX += (targetX - ringX) * .22;
    ringY += (targetY - ringY) * .22;
    aura.style.left = `${auraX}px`;
    aura.style.top = `${auraY}px`;
    ring.style.left = `${ringX}px`;
    ring.style.top = `${ringY}px`;
    frame = active ? window.requestAnimationFrame(render) : 0;
  };
  window.addEventListener("pointermove", move, { passive: true });
  window.addEventListener("pointerover", updateInteractiveState, { passive: true });
  window.addEventListener("pointerdown", press, { passive: true });
  window.addEventListener("pointerleave", leave, { passive: true });
}

/**
 * Add a restrained two-axis drift to the decorative brand illustration.
 *
 * The backdrop is deliberately isolated from content surfaces: it can move
 * and breathe without changing layout, focus order, chart geometry, or touch
 * interaction. The same reduced-motion and fine-pointer gates used by the
 * cursor follower keep the effect respectful on mobile and accessibility
 * configurations.
 */
export function setupBackdropMotion() {
  const backdrop = document.querySelector(".story-backdrop");
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  const finePointer = window.matchMedia("(pointer: fine)").matches;
  if (!backdrop || reducedMotion || !finePointer) return;

  let targetX = 0;
  let targetY = 0;
  let currentX = 0;
  let currentY = 0;
  let frame = 0;

  const render = () => {
    currentX += (targetX - currentX) * .08;
    currentY += (targetY - currentY) * .08;
    backdrop.style.setProperty("--backdrop-shift-x", `${currentX.toFixed(2)}px`);
    backdrop.style.setProperty("--backdrop-shift-y", `${currentY.toFixed(2)}px`);
    const settled = Math.abs(targetX - currentX) < .04 && Math.abs(targetY - currentY) < .04;
    frame = settled ? 0 : window.requestAnimationFrame(render);
  };

  const move = (event) => {
    targetX = ((event.clientX / Math.max(window.innerWidth, 1)) - .5) * 18;
    targetY = ((event.clientY / Math.max(window.innerHeight, 1)) - .5) * 10;
    if (!frame) frame = window.requestAnimationFrame(render);
  };

  const reset = () => {
    targetX = 0;
    targetY = 0;
    if (!frame) frame = window.requestAnimationFrame(render);
  };

  window.addEventListener("pointermove", move, { passive: true });
  window.addEventListener("pointerleave", reset, { passive: true });
}

/**
 * Add bounded pointer light and magnetic button feedback without changing
 * layout. The effect is opt-in at runtime, disabled for touch/reduced-motion,
 * and uses CSS variables so charts and form controls remain composable.
 */
export function setupSurfaceMotion() {
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  const finePointer = window.matchMedia("(pointer: fine)").matches;
  if (reducedMotion || !finePointer) return;

  document.querySelectorAll(".auth-card, .chart-card, .form-card, .guide-card, .records-card").forEach((surface) => {
    surface.addEventListener("pointermove", (event) => {
      const bounds = surface.getBoundingClientRect();
      const x = Math.max(0, Math.min(100, ((event.clientX - bounds.left) / bounds.width) * 100));
      const y = Math.max(0, Math.min(100, ((event.clientY - bounds.top) / bounds.height) * 100));
      surface.style.setProperty("--surface-x", `${x}%`);
      surface.style.setProperty("--surface-y", `${y}%`);
    }, { passive: true });
    surface.addEventListener("pointerleave", () => {
      surface.style.setProperty("--surface-x", "50%");
      surface.style.setProperty("--surface-y", "0%");
    }, { passive: true });
  });

  document.querySelectorAll(".button").forEach((button) => {
    button.addEventListener("pointermove", (event) => {
      const bounds = button.getBoundingClientRect();
      const x = ((event.clientX - bounds.left) / bounds.width - .5) * 6;
      const y = ((event.clientY - bounds.top) / bounds.height - .5) * 4;
      button.style.setProperty("--mag-x", `${x.toFixed(2)}px`);
      button.style.setProperty("--mag-y", `${y.toFixed(2)}px`);
    }, { passive: true });
    button.addEventListener("pointerleave", () => {
      button.style.setProperty("--mag-x", "0px");
      button.style.setProperty("--mag-y", "0px");
    }, { passive: true });
  });
}
