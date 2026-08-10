/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Same-origin JSON client with one error contract and CSRF policy.
 */

/**
 * Create the browser API client for the current authenticated page.
 *
 * The client intentionally owns transport concerns only. It does not know
 * about charts, forms, token math, or provider-specific response fields.
 */
export function createApiClient(csrfToken) {
  return async function requestJson(url, options = {}) {
    const headers = Object.assign({}, options.headers || {});
    if (options.method && options.method !== "GET") {
      headers["Content-Type"] = "application/json";
      headers["X-CSRF-Token"] = csrfToken;
    }
    const response = await fetch(url, Object.assign({}, options, { headers }));
    const payload = await response.json().catch(() => ({}));
    if (!response.ok) {
      const error = payload.error;
      const message = typeof error === "string" ? error : error?.message;
      const requestId = payload.request_id ? `（请求号 ${payload.request_id}）` : "";
      throw new Error(`${message || "请求失败，请稍后重试"}${requestId}`);
    }
    return payload;
  };
}
