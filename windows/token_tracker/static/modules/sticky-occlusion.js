/*
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Keep interactive content and card landmarks visually clear of the
 *          sticky header band.
 * Module: Web presentation / sticky chrome boundary
 */

const OCCLUDED_CLASS = "is-under-sticky-header";
const CANDIDATE_SELECTOR = ".site-main .button, .site-main summary, .site-main .card-heading";

function intersectsHeader(element, headerRect) {
  const rect = element.getBoundingClientRect();
  return rect.bottom > headerRect.top && rect.top < headerRect.bottom;
}

function isFocused(element) {
  return element.matches(":focus-within");
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
  if (!candidates.length) return;

  root.dataset.stickyOcclusionReady = "true";
  let frame = 0;

  const render = () => {
    const headerRect = header.getBoundingClientRect();
    const shouldGuard = header.classList.contains("is-scrolled");
    candidates.forEach((element) => {
      const occluded = shouldGuard && intersectsHeader(element, headerRect) && !isFocused(element);
      element.classList.toggle(OCCLUDED_CLASS, occluded);
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
