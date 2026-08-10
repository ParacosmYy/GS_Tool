/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Keep dynamic status and error announcements consistent across views.
 */

/**
 * Apply the accessible politeness contract to a live region.
 *
 * Visual styling remains owned by the page stylesheet. This module only
 * changes semantics so a failure is announced assertively while normal
 * progress remains polite and does not interrupt the user.
 *
 * @param {HTMLElement|null} element - Existing status element.
 * @param {boolean} isError - Whether the state represents an error.
 */
export function setLiveRegionSemantics(element, isError) {
  if (!element) return;
  element.setAttribute("role", isError ? "alert" : "status");
  element.setAttribute("aria-live", isError ? "assertive" : "polite");
  element.setAttribute("aria-atomic", "true");
  element.classList.toggle("is-error", Boolean(isError));
}

/**
 * Update a live region without replacing its DOM node or event bindings.
 *
 * @param {HTMLElement|null} element - Existing status element.
 * @param {string} message - Human-readable status text.
 * @param {boolean} isError - Whether the state represents an error.
 */
export function setLiveMessage(element, message, isError = false) {
  if (!element) return;
  setLiveRegionSemantics(element, isError);
  element.textContent = message || "";
}
