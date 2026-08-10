/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Render trend and model-mix charts without owning dashboard state.
 */

const CHART_COLORS = ["#d9ff78", "#c4c0ff", "#ff9c82", "#8fe3d4", "#f6d77a", "#aeb4c1", "#5f6572"];

function visualToken(name, fallback) {
  const value = getComputedStyle(document.documentElement).getPropertyValue(name).trim();
  return value || fallback;
}

function chartTheme() {
  return {
    ink: visualToken("--ink", "#f7f8fc"),
    soft: visualToken("--ink-soft", "#d7dbe5"),
    muted: visualToken("--ink-muted", "#aeb4c1"),
    line: visualToken("--line-soft", "rgba(230, 235, 245, .17)"),
    accent: visualToken("--signal-lime", "#d9ff78"),
    accentWarm: visualToken("--signal-warm", "#ff9c82"),
    tooltip: visualToken("--panel-strong", "#171a21")
  };
}

function setChartState(canvas, state, title, detail) {
  const wrapper = canvas?.closest(".chart-wrap");
  if (!wrapper) return;
  wrapper.dataset.chartState = state;
  let emptyState = wrapper.querySelector(".chart-empty-state");
  if (state === "ready") {
    emptyState?.remove();
    return;
  }
  if (!emptyState) {
    emptyState = document.createElement("div");
    emptyState.className = "chart-empty-state";
    emptyState.setAttribute("role", "status");
    wrapper.appendChild(emptyState);
  }
  emptyState.replaceChildren();
  const marker = document.createElement("span");
  marker.className = "chart-empty-marker";
  marker.setAttribute("aria-hidden", "true");
  const heading = document.createElement("strong");
  heading.textContent = title;
  const copy = document.createElement("small");
  copy.textContent = detail;
  emptyState.append(marker, heading, copy);
}

function dateLabel(value) {
  const parts = String(value || "").split("-");
  return parts.length === 3 ? `${parts[1]}/${parts[2]}` : value;
}

/**
 * Create chart renderers with injected formatters so localization remains an
 * application concern while this module stays focused on visualization.
 */
export function createChartRenderer(numberFormat, formatNumber) {
  let trendChart = null;
  let modelChart = null;

  function renderTrend(points) {
    const canvas = document.getElementById("trend-chart");
    if (!canvas) return;
    const theme = chartTheme();
    const normalized = Array.isArray(points) ? points : [];
    const hasSignal = normalized.some((point) => Number(point.input_tokens || 0) + Number(point.output_tokens || 0) > 0);
    if (trendChart) trendChart.destroy();
    if (!window.Chart) {
      setChartState(canvas, "unavailable", "图表组件未加载", "检查网络后刷新页面");
      return;
    }
    if (!hasSignal) {
      setChartState(canvas, "empty", "等待第一条用量信号", "完成一次自动采集后，趋势会在这里展开");
      return;
    }
    setChartState(canvas, "ready");
    const context = canvas.getContext("2d");
    trendChart = new Chart(context, {
      type: "line",
      data: {
        labels: normalized.map((point) => dateLabel(point.day)),
        datasets: [
          { label: "输入", data: normalized.map((point) => point.input_tokens), borderColor: theme.accent, backgroundColor: "rgba(217,255,120,.08)", fill: true, tension: .38, borderWidth: 2, pointRadius: 2, pointHoverRadius: 5, pointBackgroundColor: theme.accent },
          { label: "输出", data: normalized.map((point) => point.output_tokens), borderColor: theme.accentWarm, backgroundColor: "rgba(255,156,130,.07)", fill: true, tension: .38, borderWidth: 2, pointRadius: 2, pointHoverRadius: 5, pointBackgroundColor: theme.accentWarm }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 950, easing: "easeOutQuart" },
        interaction: { mode: "index", intersect: false },
        plugins: { legend: { display: false }, tooltip: { padding: 12, displayColors: true, backgroundColor: theme.tooltip, titleColor: theme.ink, bodyColor: theme.soft, borderColor: theme.line, borderWidth: 1 } },
        scales: {
          x: { grid: { display: false }, ticks: { color: theme.muted, maxTicksLimit: 7, padding: 6, font: { size: 11, family: "DM Mono" } } },
          y: { beginAtZero: true, grid: { color: theme.line, drawTicks: false }, ticks: { color: theme.muted, padding: 10, maxTicksLimit: 6, font: { size: 11, family: "DM Mono" }, callback: (value) => numberFormat.format(value) } }
        }
      }
    });
  }

  function renderModelMix(models) {
    const canvas = document.getElementById("model-chart");
    const legend = document.getElementById("model-legend");
    if (!canvas || !legend) return;
    if (modelChart) modelChart.destroy();
    const normalized = Array.isArray(models) ? models : [];
    const labels = normalized.map((model) => model.model);
    const values = normalized.map((model) => model.total_tokens);
    if (!window.Chart) {
      setChartState(canvas, "unavailable", "图表组件未加载", "检查网络后刷新页面");
      legend.replaceChildren();
      return;
    }
    if (!normalized.length) {
      setChartState(canvas, "empty", "还没有模型记录", "连接模型后，这里会显示 token 去向");
      legend.replaceChildren();
      return;
    }
    setChartState(canvas, "ready");
    const theme = chartTheme();
    modelChart = new Chart(canvas.getContext("2d"), {
      type: "doughnut",
      data: { labels: labels, datasets: [{ data: values, backgroundColor: labels.map((_, i) => CHART_COLORS[i % CHART_COLORS.length]), borderWidth: 0, hoverOffset: 4 }] },
      options: { responsive: true, maintainAspectRatio: false, cutout: "70%", animation: { duration: 1100, easing: "easeOutQuart" }, plugins: { legend: { display: false }, tooltip: { padding: 12, backgroundColor: theme.tooltip, titleColor: theme.ink, bodyColor: theme.soft, borderColor: theme.line, borderWidth: 1 } } }
    });
    legend.replaceChildren();
    normalized.forEach((model, index) => {
      const item = document.createElement("span");
      item.className = "legend-item";
      const dot = document.createElement("span");
      dot.className = "legend-color";
      dot.style.backgroundColor = CHART_COLORS[index % CHART_COLORS.length];
      const label = document.createElement("span");
      label.textContent = `${model.model} ${formatNumber(model.total_tokens)}`;
      item.append(dot, label);
      legend.appendChild(item);
    });
  }

  return { renderTrend, renderModelMix };
}
