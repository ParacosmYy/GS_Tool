/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Add lightweight pointer and focus feedback to auth scenes.
 */

import { setupNavigation } from "./modules/navigation.js";
import { setupBackdropMotion, setupPointerFollower, setupSurfaceMotion } from "./modules/motion.js";

(function () {
  "use strict";

  setupPointerFollower();
  setupBackdropMotion();
  setupSurfaceMotion();
  setupNavigation();

  const scene = document.querySelector("[data-auth-scene]");
  const finePointer = window.matchMedia("(pointer: fine)").matches;
  const desktopViewport = window.matchMedia("(min-width: 621px)").matches;
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  if (!scene) return;

  // Focus feedback is an accessibility behavior, not a pointer-only effect;
  // register it before the optional mouse spotlight can opt out.
  scene.querySelectorAll("input").forEach((input) => {
    input.addEventListener("focus", () => { scene.dataset.authFocus = "field"; });
    input.addEventListener("blur", () => { delete scene.dataset.authFocus; });
  });

  // Script autofocus is useful for the desktop keyboard path, but its focus
  // ring should stay quiet until the user begins an intentional interaction.
  // The marker is removed on the first pointer or keyboard event so normal
  // :focus-visible guidance immediately resumes.
  const clearAutofocusMarker = () => { delete scene.dataset.authAutofocus; };
  scene.addEventListener("pointerdown", clearAutofocusMarker, { passive: true });
  scene.addEventListener("keydown", clearAutofocusMarker, { passive: true });

  const initialField = scene.querySelector('input[name="username"]');
  if (finePointer && desktopViewport && initialField && document.activeElement === document.body) {
    // Desktop users keep the fast keyboard path, while preventScroll avoids
    // mobile browsers moving the entire authentication scene to the field.
    window.requestAnimationFrame(() => {
      scene.dataset.authAutofocus = "true";
      initialField.focus({ preventScroll: true });
    });
  }
  if (!finePointer || reducedMotion) return;

  let frame = 0;
  let pointerX = window.innerWidth / 2;
  let pointerY = window.innerHeight / 2;

  function renderSpotlight() {
    const bounds = scene.getBoundingClientRect();
    const x = Math.max(0, Math.min(100, ((pointerX - bounds.left) / bounds.width) * 100));
    const y = Math.max(0, Math.min(100, ((pointerY - bounds.top) / bounds.height) * 100));
    scene.style.setProperty("--auth-spotlight-x", `${x}%`);
    scene.style.setProperty("--auth-spotlight-y", `${y}%`);
    frame = 0;
  }

  function handlePointerMove(event) {
    pointerX = event.clientX;
    pointerY = event.clientY;
    if (!frame) frame = window.requestAnimationFrame(renderSpotlight);
  }

  function resetSpotlight() {
    scene.style.setProperty("--auth-spotlight-x", "50%");
    scene.style.setProperty("--auth-spotlight-y", "50%");
  }

  window.addEventListener("pointermove", handlePointerMove, { passive: true });
  window.addEventListener("pointerleave", resetSpotlight, { passive: true });

})();
