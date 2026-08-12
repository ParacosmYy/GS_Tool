/*
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Keep interactive content and card landmarks visually clear of the
 *          sticky header band.
 * Module: Web presentation / sticky chrome boundary
 */

const OCCLUDED_CLASS = "is-under-sticky-header";
const SURFACE_OCCLUDED_CLASS = "has-sticky-occlusion";
const CANDIDATE_SELECTOR = ".site-main .button, .site-main summary, .site-main .card-heading, .site-main .auto-form-actions";
const SURFACE_SELECTOR = ".site-main .auth-card, .site-main .chart-card, .site-main .form-card, .site-main .records-card, .site-main .guide-card, .site-main .manual-details";

function intersectsHeader(element, headerRect) {
  const rect = element.getBoundingClientRect();
  return rect.bottom > headerRect.top && rect.top < headerRect.bottom;
}

function isFocused(element) {
  return element.matches(":focus-within");
}

function getSurfaceOcclusionBounds(element, headerRect, shouldGuard) {
  if (!shouldGuard) return null;

  const rect = element.getBoundingClientRect();
  const intersects = rect.bottom > headerRect.top && rect.top < headerRect.bottom;
  if (!intersects) return null;

  return {
    start: Math.max(0, headerRect.top - rect.top),
    end: Math.min(rect.height, headerRect.bottom - rect.top),
  };
}

/**
 * Hide only the visual signal of controls or landmarks crossing sticky chrome.
 *
 * The element keeps its layout box and tab order. A focused control is never
 * dimmed, so keyboard users can still discover and operate it while scrolling.
 */
export function setupStickyOcclusion() {
  const header = document.querySelector(".site-header");
  const root = document.documentElement;
  if (!header || root.dataset.stickyOcclusionReady === "true") return;

  const candidates = [...document.querySelectorAll(CANDIDATE_SELECTOR)]
    .filter((element) => !element.closest(".hero-actions"));
  const surfaces = [...document.querySelectorAll(SURFACE_SELECTOR)];
  if (!candidates.length && !surfaces.length) return;

  root.dataset.stickyOcclusionReady = "true";
  let frame = 0;

  const render = () => {
    const headerRect = header.getBoundingClientRect();
    const shouldGuard = header.classList.contains("is-scrolled");
    candidates.forEach((element) => {
      const occluded = shouldGuard && intersectsHeader(element, headerRect) && !isFocused(element);
      element.classList.toggle(OCCLUDED_CLASS, occluded);
    });
    surfaces.forEach((element) => {
      const occlusionBounds = getSurfaceOcclusionBounds(element, headerRect, shouldGuard);
      if (occlusionBounds) {
        element.style.setProperty("--sticky-occlusion-start", `${occlusionBounds.start}px`);
        element.style.setProperty("--sticky-occlusion-end", `${occlusionBounds.end}px`);
      } else {
        element.style.removeProperty("--sticky-occlusion-start");
        element.style.removeProperty("--sticky-occlusion-end");
      }
      element.classList.toggle(SURFACE_OCCLUDED_CLASS, Boolean(occlusionBounds));
    });
    frame = 0;
  };

  const schedule = () => {
    if (!frame) frame = window.requestAnimationFrame(render);
  };

  window.addEventListener("scroll", schedule, { passive: true });
  window.addEventListener("resize", schedule, { passive: true });
  document.addEventListener("focusin", schedule, { passive: true });
  document.addEventListener("focusout", schedule, { passive: true });
  render();
}
