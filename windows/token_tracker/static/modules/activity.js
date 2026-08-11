/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Keep personal work-signal capture and recent activity rendering
 *          outside the dashboard orchestration module.
 * Module: Web observatory / structured activity boundary
 */

import { setLiveMessage } from "./live-region.js";
import { setMotionState } from "./motion.js";

const DIRECTION_LABELS = {
  coding: "编码开发",
  debugging: "调试排错",
  research: "技术研究",
  writing: "文档写作",
  planning: "方案规划",
  review: "代码审查",
  other: "其他",
};

const OUTCOME_LABELS = { success: "成功", partial: "部分完成", failure: "失败" };

function formatCode(event) {
  return event.error_code || event.result_code || "—";
}

function appendCell(row, value, className = "") {
  const cell = document.createElement("td");
  cell.textContent = value == null ? "" : String(value);
  if (className) cell.className = className;
  row.appendChild(cell);
}

function renderEvents(body, events, formatNumber) {
  body.replaceChildren();
  if (!events.length) {
    const row = document.createElement("tr");
    const cell = document.createElement("td");
    cell.colSpan = 5;
    cell.className = "table-empty table-empty--signal";
    cell.textContent = "当前还没有工作事件。完成一次 AI 辅助工作后再回来看看。";
    row.appendChild(cell);
    body.appendChild(row);
    return;
  }
  events.forEach((event) => {
    const row = document.createElement("tr");
    appendCell(row, DIRECTION_LABELS[event.direction] || event.direction);
    appendCell(row, OUTCOME_LABELS[event.outcome] || event.outcome);
    appendCell(row, event.efficiency_score == null ? "—" : `${formatNumber(event.efficiency_score)}%`, "table-number");
    appendCell(row, formatCode(event), "table-source");
    appendCell(row, event.created_at || "", "table-source");
    body.appendChild(row);
  });
}

function formPayload(form) {
  const values = Object.fromEntries(new FormData(form).entries());
  return {
    protocol_version: 1,
    command: "work_event.create",
    idempotency_key: globalThis.crypto?.randomUUID?.() || `web-${Date.now()}-${Math.random().toString(16).slice(2)}`,
    payload: {
      direction: values.direction,
      outcome: values.outcome,
      efficiency_score: values.efficiency_score || null,
      result_code: values.result_code || null,
      error_code: values.error_code || null,
      project: values.project || "",
      task_type: values.task_type || "",
      note: values.note || "",
    },
  };
}

export function createActivityController({ api, formatNumber }) {
  const form = document.getElementById("work-event-form");
  const body = document.getElementById("work-events-body");
  const message = document.getElementById("work-event-message");
  const status = document.getElementById("work-event-status");
  const card = document.getElementById("activity-entry");

  async function loadEvents() {
    if (!body || !status) return;
    try {
      const payload = await api("/api/v1/events/work?limit=20");
      renderEvents(body, payload.events || [], formatNumber);
      setLiveMessage(status, `已加载 ${formatNumber(payload.pagination?.total || 0)} 条工作事件`);
    } catch (error) {
      setLiveMessage(status, "读取失败", true);
      if (message) setLiveMessage(message, error.message, true);
    }
  }

  function mount() {
    if (!form || !body) return;
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      if (form.dataset.submitting === "true") return;
      form.dataset.submitting = "true";
      const submit = form.querySelector('button[type="submit"]');
      if (submit) submit.disabled = true;
      setMotionState(card, "loading");
      setMotionState(form, "loading");
      setLiveMessage(message, "保存中……");
      try {
        await api("/api/v1/events/work", { method: "POST", body: JSON.stringify(formPayload(form)) });
        form.reset();
        setLiveMessage(message, "工作信号已保存。");
        setMotionState(card, "success");
        setMotionState(form, "success");
        await loadEvents();
      } catch (error) {
        setLiveMessage(message, error.message, true);
        setMotionState(card, "error");
        setMotionState(form, "error");
      } finally {
        form.dataset.submitting = "false";
        if (submit) submit.disabled = false;
      }
    });
    loadEvents();
  }

  return { mount };
}
