import { createApiClient } from "./modules/api-client.js";
import { createChartRenderer } from "./modules/charts.js";
/* Author: AI Token Tracker Engineering Team | Maintainer: Project Owner | Purpose: Dashboard orchestration and feature-specific form state. */

import { animateNumber, setMotionState, setupBackdropMotion, setupPointerFollower, setupReveal, setupSurfaceMotion } from "./modules/motion.js";

(function () {
  "use strict";

  const csrfToken = document.querySelector('meta[name="csrf-token"]')?.content || "";
  const state = { period: "day", detectedModels: [] };
  const numberFormat = new Intl.NumberFormat("zh-CN");
  const activeAnimations = new Map();
  const api = createApiClient(csrfToken);

  function byId(id) { return document.getElementById(id); }
  function formatNumber(value) { return numberFormat.format(Number(value || 0)); }
  function setMessage(element, message, isError) {
    element.textContent = message || "";
    element.classList.toggle("is-error", Boolean(isError));
  }

  function setDashboardStatus(message, isError) {
    const element = byId("dashboard-status");
    if (!element) return;
    element.textContent = message || "";
    element.classList.toggle("is-error", Boolean(isError));
  }

  const charts = createChartRenderer(numberFormat, formatNumber);

  function renderRecords(records) {
    const body = byId("records-body");
    body.replaceChildren();
    if (!records.length) {
      const row = document.createElement("tr");
      const cell = document.createElement("td");
      cell.colSpan = 6;
      cell.className = "table-empty";
      cell.textContent = "当前范围暂无记录，先添加一条吧。";
      row.appendChild(cell);
      body.appendChild(row);
      return;
    }
    records.forEach((record) => {
      const row = document.createElement("tr");
      [record.model, formatNumber(record.input_tokens), formatNumber(record.output_tokens), formatNumber(record.total_tokens), record.timestamp].forEach((value, index) => {
        const cell = document.createElement("td");
        cell.textContent = value;
        if (index > 0 && index < 4) cell.className = "table-number";
        row.appendChild(cell);
      });
      const source = document.createElement("td");
      source.className = "table-source";
      source.textContent = `${record.source === "proxy" ? "自动采集" : "补录"}${record.note ? ` · ${record.note}` : ""}`;
      row.appendChild(source);
      body.appendChild(row);
    });
  }

  async function loadSummary() {
    const analysis = byId("analysis");
    setMotionState(analysis, "loading");
    setDashboardStatus("正在刷新用量……", false);
    try {
      const payload = await api(`/api/summary?period=${encodeURIComponent(state.period)}`);
      animateNumber(byId("total-tokens"), payload.totals.total_tokens, 950, formatNumber, activeAnimations);
      animateNumber(byId("input-tokens"), payload.totals.input_tokens, 800, formatNumber, activeAnimations);
      animateNumber(byId("output-tokens"), payload.totals.output_tokens, 800, formatNumber, activeAnimations);
      animateNumber(byId("model-count"), payload.by_model.length, 650, formatNumber, activeAnimations);
      const rangeLabel = { day: "today", week: "this week", month: "this month", all: "all time" }[state.period] || state.period;
      byId("call-count").textContent = `${formatNumber(payload.totals.calls)} calls / ${rangeLabel}`;
      charts.renderTrend(payload.trend);
      charts.renderModelMix(payload.by_model);
      renderRecords(payload.records);
      byId("export-button").href = `/api/export?period=${encodeURIComponent(state.period)}`;
      setMotionState(analysis, payload.totals.calls ? "success" : "empty");
      setDashboardStatus(`已更新：${state.period === "day" ? "今天" : state.period === "week" ? "本周" : state.period === "month" ? "本月" : "全部"}`, false);
    } catch (error) {
      setMotionState(analysis, "error");
      throw error;
    }
  }

  function setupRangeButtons() {
    document.querySelectorAll(".range-button").forEach((button) => {
      button.addEventListener("click", async () => {
        document.querySelectorAll(".range-button").forEach((item) => item.classList.remove("is-active"));
        button.classList.add("is-active");
        state.period = button.dataset.period;
        try { await loadSummary(); } catch (error) { setDashboardStatus(error.message, true); }
      });
    });
  }

  function setupManualForm() {
    const form = byId("manual-form");
    const card = form.closest(".form-card");
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      const message = byId("manual-message");
      setMotionState(card, "loading");
      setMotionState(form, "loading");
      setMessage(message, "保存中……", false);
      const data = Object.fromEntries(new FormData(form).entries());
      try {
        const payload = await api("/api/records", { method: "POST", body: JSON.stringify(data) });
        form.reset();
        setMessage(message, `已记录 ${formatNumber(payload.record.total_tokens)} tokens。`, false);
        setMotionState(card, "success");
        setMotionState(form, "success");
        await loadSummary();
      } catch (error) {
        setMotionState(card, "error");
        setMotionState(form, "error");
        setMessage(message, error.message, true);
      }
    });
  }

  function setupProxyForm() {
    const form = byId("proxy-form");
    const baseUrl = form.elements.base_url;
    const apiKey = form.elements.api_key;
    const model = form.elements.model;
    const detectButton = byId("detect-models");
    const modelOptions = byId("provider-model-options");
    const status = byId("provider-status");
    const card = form.closest(".auto-card");
    const clearKeyButton = byId("clear-provider-key");
    let sessionKey = "";
    baseUrl.value = localStorage.getItem("ai-tracker-base-url") || "";
    model.value = localStorage.getItem("ai-tracker-model") || "";

    function currentKey() {
      const entered = apiKey.value.trim();
      if (entered) sessionKey = entered;
      return entered || sessionKey;
    }

    function setProviderStatus(message, kind) {
      status.textContent = "";
      const dot = document.createElement("i");
      status.appendChild(dot);
      status.append(` ${message}`);
      status.classList.toggle("is-ready", kind === "ready");
      status.classList.toggle("is-error", kind === "error");
    }

    function renderDetectedModels(models) {
      state.detectedModels = models;
      modelOptions.replaceChildren();
      models.forEach((modelId) => {
        const option = document.createElement("option");
        option.value = modelId;
        modelOptions.appendChild(option);
      });
      if (!model.value && models[0]) model.value = models[0];
    }

    async function detectModels() {
      const key = currentKey();
      if (!baseUrl.value.trim() || !key) {
        setMessage(byId("proxy-message"), "先填写 Base URL 和 API Key，再检测模型。", true);
        setProviderStatus("等待连接", "error");
        return;
      }
      detectButton.disabled = true;
      setMotionState(card, "loading");
      setMotionState(form, "loading");
      setProviderStatus("正在检测模型", "");
      setMessage(byId("proxy-message"), "正在读取上游模型列表……", false);
      try {
        const payload = await api("/api/provider/models", {
          method: "POST",
          body: JSON.stringify({ provider: "auto", base_url: baseUrl.value, api_key: key })
        });
        renderDetectedModels(payload.models || []);
        localStorage.setItem("ai-tracker-base-url", baseUrl.value);
        localStorage.setItem("ai-tracker-model", model.value);
        setProviderStatus(`已发现 ${formatNumber(payload.models.length)} 个模型`, "ready");
        setMessage(byId("proxy-message"), `已发现 ${formatNumber(payload.models.length)} 个模型，选好后即可自动记账。`, false);
        setMotionState(card, "success");
        setMotionState(form, "success");
      } catch (error) {
        setProviderStatus("检测失败", "error");
        setMessage(byId("proxy-message"), error.message, true);
        setMotionState(card, "error");
        setMotionState(form, "error");
      } finally {
        detectButton.disabled = false;
      }
    }

    detectButton.addEventListener("click", detectModels);
    clearKeyButton.addEventListener("click", () => {
      sessionKey = "";
      apiKey.value = "";
      setProviderStatus("未连接", "");
      setMessage(byId("proxy-message"), "Key 已从当前页面内存清除。", false);
    });
    baseUrl.addEventListener("change", () => setProviderStatus("等待检测", ""));
    model.addEventListener("change", () => localStorage.setItem("ai-tracker-model", model.value));
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      const message = byId("proxy-message");
      const responseBox = byId("proxy-response");
      const submit = byId("proxy-submit");
      const data = Object.fromEntries(new FormData(form).entries());
      data.api_key = currentKey();
      localStorage.setItem("ai-tracker-base-url", data.base_url);
      localStorage.setItem("ai-tracker-model", data.model);
      setMotionState(card, "loading");
      setMotionState(form, "loading");
      setProviderStatus("正在采集", "");
      setMessage(message, "正在调用上游模型……", false);
      responseBox.hidden = true;
      submit.disabled = true;
      try {
        const payload = await api("/api/proxy/chat/completions", {
          method: "POST",
          body: JSON.stringify({
            provider: "auto",
            base_url: data.base_url,
            api_key: data.api_key,
            model: data.model,
            messages: [{ role: "user", content: data.prompt }],
            note: data.note
          })
        });
        const usage = payload.usage;
        setMessage(message, payload.recorded ? `已自动记录 ${formatNumber(payload.record.total_tokens)} tokens。` : (payload.warning || "调用成功，但没有记录。"), !payload.recorded);
        setProviderStatus(payload.recorded ? `已自动归档 · ${payload.record.model}` : "未找到 usage", payload.recorded ? "ready" : "error");
        setMotionState(card, payload.recorded ? "success" : "error");
        setMotionState(form, payload.recorded ? "success" : "error");
        const answer = payload.response?.choices?.[0]?.message?.content || JSON.stringify(payload.response, null, 2);
        responseBox.textContent = answer;
        responseBox.hidden = false;
        if (usage) responseBox.textContent += `\n\nusage: ${JSON.stringify(usage)}`;
        await loadSummary();
      } catch (error) {
        setProviderStatus("请求失败", "error");
        setMotionState(card, "error");
        setMotionState(form, "error");
        setMessage(message, error.message, true);
      }
      finally { submit.disabled = false; }
    });
  }

  document.addEventListener("DOMContentLoaded", async () => {
    setupRangeButtons();
    setupManualForm();
    setupProxyForm();
    setupReveal();
    setupPointerFollower();
    setupBackdropMotion();
    setupSurfaceMotion();
    try { await loadSummary(); } catch (error) { setDashboardStatus(error.message, true); }
  });
})();
