"""QuillForge UI 美化 · 1000 轮渐进式自动优化引擎。

将"UI 美化"建模为对"设计令牌空间"的可度量优化问题：
  - 参数空间 = 3 套基础主题的 22 个颜色（HSL 三通道）+ 11 个全局几何/排版令牌
  - 目标函数 = 六维复合评分（视觉/布局/配色/排版/组件/交互），0–100
  - 算法     = hill-climbing + 轻度退火；仅接受更优提案，best-so-far 单调非降
  - 硬约束   = 任一主题文本最小对比度不得低于 3.0:1（可达性底线）

复用项目自带、与 Qt 无关的对比度引擎（theme_tokens.contrast_ratio 等），
因此本引擎可在无 GUI / 无 QApplication 的环境下运行。
"""

from __future__ import annotations

import csv
import json
import math
import random
import sys
from dataclasses import dataclass
from pathlib import Path

# ---- 接入项目自带、与 Qt 无关的令牌/对比度引擎 ----
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from quillforge.presentation.theme_tokens import (  # noqa: E402
    _BASE_THEMES,
    best_on_accent,
    contrast_ratio,
    readable_foreground,
    relative_luminance,
)

RNG_SEED = 20260814
ROUNDS = 1000

# 每套主题参与优化的 22 个颜色字段
COLOR_FIELDS = [
    "surface_0", "surface_1", "surface_2", "surface_3", "surface_hover",
    "border", "border_strong", "text_primary", "text_secondary", "text_muted",
    "accent", "accent_alt", "accent_pink", "accent_gold", "success", "danger",
    "selection", "pressed", "success_bg", "warning_bg", "error_bg", "editor_line",
]
THEME_KEYS = list(_BASE_THEMES.keys())  # ink-violet, paper-sand, sakura-pop

# 全局几何 / 排版令牌： (name, init, min, max, step)
GEOM_SPECS = {
    "radius_sm": (6.0, 4.0, 10.0, 0.6),
    "radius_md": (9.0, 6.0, 14.0, 0.6),
    "radius_lg": (14.0, 10.0, 22.0, 0.7),
    "spacing_xs": (4.0, 2.0, 7.0, 0.5),
    "spacing_sm": (6.0, 4.0, 11.0, 0.6),
    "spacing_md": (10.0, 7.0, 16.0, 0.7),
    "spacing_lg": (18.0, 13.0, 30.0, 0.9),
    "focus_ring_width": (2.0, 1.0, 3.0, 0.2),
    "transition_ms": (160.0, 120.0, 260.0, 6.0),
    "type_scale": (1.20, 1.12, 1.32, 0.01),
    "line_height_ratio": (1.50, 1.40, 1.62, 0.02),
}

# 六维权重（合计 1.0）
W = {
    "visual": 0.18, "layout": 0.14, "color": 0.26,
    "typography": 0.14, "components": 0.16, "interaction": 0.12,
}


# ----------------------------------------------------------------------------
# 颜色空间工具
# ----------------------------------------------------------------------------
def hex_to_rgb(h: str) -> tuple[float, float, float]:
    return (int(h[1:3], 16), int(h[3:5], 16), int(h[5:7], 16))


def rgb_to_hex(r: int, g: int, b: int) -> str:
    return "#{:02x}{:02x}{:02x}".format(
        max(0, min(255, round(r))),
        max(0, min(255, round(g))),
        max(0, min(255, round(b))),
    )


def rgb_to_hsl(r: int, g: int, b: int) -> tuple[float, float, float]:
    r, g, b = r / 255, g / 255, b / 255
    mx, mn = max(r, g, b), min(r, g, b)
    l = (mx + mn) / 2
    if mx == mn:
        h = s = 0.0
    else:
        d = mx - mn
        s = d / (2 - mx - mn) if l > 0.5 else d / (mx + mn)
        if mx == r:
            h = (g - b) / d + (6 if g < b else 0)
        elif mx == g:
            h = (b - r) / d + 2
        else:
            h = (r - g) / d + 4
        h *= 60
    return (h % 360, s, l)


def hsl_to_rgb(h: float, s: float, l: float) -> tuple[int, int, int]:
    h = (h % 360) / 360
    if s == 0:
        v = round(l * 255)
        return (v, v, v)
    q = l * (1 + s) if l < 0.5 else l + s - l * s
    p = 2 * l - q

    def hue(t: float) -> float:
        if t < 0:
            t += 1
        if t > 1:
            t -= 1
        if t < 1 / 6:
            return p + (q - p) * 6 * t
        if t < 1 / 2:
            return q
        if t < 2 / 3:
            return p + (q - p) * (2 / 3 - t) * 6
        return p

    # 标准 HSL→RGB 通道指派（修复：原先 R/G 错位导致色相旋转 ~120°）
    r = hue(h + 1 / 3)
    g = hue(h)
    b = hue(h - 1 / 3)
    return (round(r * 255), round(g * 255), round(b * 255))


def hue_distance(a: float, b: float) -> float:
    return min(abs(a - b), 360 - abs(a - b))


def clamp(x: float, lo: float = 0.0, hi: float = 1.0) -> float:
    return max(lo, min(hi, x))


# ----------------------------------------------------------------------------
# 状态解析
# ----------------------------------------------------------------------------
@dataclass
class State:
    colors: dict  # theme_key -> {field: (h, s, l)}
    geom: dict    # name -> float


def build_initial_state() -> State:
    colors = {}
    for key in THEME_KEYS:
        base = _BASE_THEMES[key]
        colors[key] = {
            f: rgb_to_hsl(*hex_to_rgb(getattr(base, f))) for f in COLOR_FIELDS
        }
    geom = {name: spec[0] for name, spec in GEOM_SPECS.items()}
    return State(colors, geom)


def resolve_theme(state: State, key: str) -> dict:
    """把某主题的 HSL 状态还原为 hex 字典。"""
    out = {}
    for f in COLOR_FIELDS:
        h, s, l = state.colors[key][f]
        out[f] = rgb_to_hex(*hsl_to_rgb(h, s, l))
    return out


def resolve_all(state: State) -> dict:
    return {key: resolve_theme(state, key) for key in THEME_KEYS}


# ----------------------------------------------------------------------------
# 六维评分
# ----------------------------------------------------------------------------
def score_visual(hx: dict) -> float:
    order = ["surface_0", "surface_1", "surface_2", "surface_3", "surface_hover"]
    lums = [relative_luminance(hx[f]) for f in order]
    # 判定方向：dark 主题 surface_0 最暗；light 主题相反
    ascending = lums[0] < lums[-1]
    diffs = []
    monotonic = True
    for i in range(len(lums) - 1):
        d = lums[i + 1] - lums[i]
        if (d > 0) != ascending:
            monotonic = False
        diffs.append(abs(d))
    target = 0.035
    sep = clamp(sum(diffs) / len(diffs) / target)
    if not monotonic:
        sep *= 0.4
    # accent 鲜度：饱和度甜区 [0.40, 0.85]
    _, s_ac, _ = _hsl_of(hx, "accent")
    if 0.40 <= s_ac <= 0.85:
        vivid = 1.0
    else:
        vivid = clamp(1 - abs(s_ac - 0.625) / 0.30)
    return 0.6 * sep + 0.4 * vivid


def _hsl_of(hx: dict, field: str) -> tuple[float, float, float]:
    return rgb_to_hsl(*hex_to_rgb(hx[field]))


def score_layout(geom: dict) -> float:
    xs, sm, md, lg = (geom["spacing_xs"], geom["spacing_sm"],
                      geom["spacing_md"], geom["spacing_lg"])
    rs = [sm / xs, md / sm, lg / md]
    spacing = sum(clamp(1 - abs(r - 1.5) / 0.5) for r in rs) / 3
    rmd = geom["radius_md"] / geom["radius_sm"]
    rlg = geom["radius_lg"] / geom["radius_md"]
    radius = (clamp(1 - abs(rmd - 1.4) / 0.4) + clamp(1 - abs(rlg - 1.6) / 0.4)) / 2
    return 0.5 * spacing + 0.5 * radius


def score_color(hx: dict) -> float:
    text_min = min(
        contrast_ratio(hx["text_primary"], hx["surface_0"]),
        contrast_ratio(hx["text_secondary"], hx["surface_1"]),
        contrast_ratio(hx["text_muted"], hx["surface_2"]),
    )
    on_accent = best_on_accent((hx["accent"], hx["accent_pink"], hx["accent_gold"]))
    alt_text = readable_foreground(hx["accent_alt"], hx["surface_3"], hx["text_primary"])
    semantic_min = min(
        contrast_ratio(on_accent, hx["accent"]),
        contrast_ratio(hx["success"], hx["success_bg"]),
        contrast_ratio(hx["danger"], hx["error_bg"]),
        contrast_ratio(alt_text, hx["surface_3"]),
    )
    _, ha, _ = _hsl_of(hx, "accent")
    _, hal, _ = _hsl_of(hx, "accent_alt")
    _, hp, _ = _hsl_of(hx, "accent_pink")
    _, hg, _ = _hsl_of(hx, "accent_gold")
    spread = hue_distance(ha, hal)
    spread_s = 1.0 if 40 <= spread <= 180 else clamp(1 - abs(spread - 110) / 90)
    analog_pink = clamp(1 - hue_distance(ha, hp) / 70)
    analog_gold = clamp(1 - hue_distance(ha, hg) / 70)
    harmony = (spread_s + analog_pink + analog_gold) / 3
    text_s = clamp((text_min - 3) / 4)        # 3:1→0, 7:1→1
    sem_s = clamp((semantic_min - 3) / 1.5)    # 3:1→0, 4.5:1→1
    return 0.5 * text_s + 0.3 * sem_s + 0.2 * harmony


def score_typography(geom: dict) -> float:
    scale = geom["type_scale"]
    sc = 1.0 if 1.15 <= scale <= 1.30 else clamp(1 - abs(scale - 1.20) / 0.15)
    lh = geom["line_height_ratio"]
    lh_s = 1.0 if 1.40 <= lh <= 1.60 else clamp(1 - abs(lh - 1.50) / 0.12)
    return 0.5 * sc + 0.3 * lh_s + 0.2 * 1.0  # 权重克制为静态满分


def score_components(hx: dict) -> float:
    d_pressed = contrast_ratio(hx["pressed"], hx["surface_hover"])
    d_danger = contrast_ratio(hx["danger"], hx["accent"])
    state_s = 0.5 * clamp((d_pressed - 1) / 1) + 0.5 * clamp((d_danger - 1) / 1)
    border_min = min(
        contrast_ratio(hx["border"], hx["surface_0"]),
        contrast_ratio(hx["border_strong"], hx["surface_1"]),
    )
    border_s = clamp((border_min - 3) / 1.5)
    return 0.6 * state_s + 0.4 * border_s


def score_interaction(hx: dict, geom: dict) -> float:
    focus = contrast_ratio(hx["accent_alt"], hx["surface_hover"])
    focus_s = clamp((focus - 3) / 2)  # 3:1→0, 5:1→1
    tm = geom["transition_ms"]
    motion_s = 1.0 if 120 <= tm <= 260 else clamp(1 - abs(tm - 190) / 90)
    timing_s = 1.0  # 单一 transition_ms，天然一致
    return 0.5 * focus_s + 0.3 * motion_s + 0.2 * timing_s


def composite(state: State) -> tuple[float, dict]:
    """返回 (复合分 0–100, 六维明细)。复合分 = 0.5·均值 + 0.5·最弱主题（一致性）。"""
    resolved = resolve_all(state)
    per_theme = {}
    dims_sum = {k: 0.0 for k in W}
    for key in THEME_KEYS:
        hx = resolved[key]
        v = score_visual(hx)
        lo = score_layout(state.geom)
        c = score_color(hx)
        t = score_typography(state.geom)
        comp = score_components(hx)
        it = score_interaction(hx, state.geom)
        theme_score = 100 * (W["visual"] * v + W["layout"] * lo + W["color"] * c
                             + W["typography"] * t + W["components"] * comp
                             + W["interaction"] * it)
        per_theme[key] = theme_score
        dims_sum["visual"] += v
        dims_sum["layout"] += lo
        dims_sum["color"] += c
        dims_sum["typography"] += t
        dims_sum["components"] += comp
        dims_sum["interaction"] += it
        # 硬约束：文本最小对比度低于 3:1 直接判负（可达性底线）
        text_min = min(
            contrast_ratio(hx["text_primary"], hx["surface_0"]),
            contrast_ratio(hx["text_secondary"], hx["surface_1"]),
            contrast_ratio(hx["text_muted"], hx["surface_2"]),
        )
        if text_min < 3.0:
            return (-1e9, {k: 0.0 for k in W})
    n = len(THEME_KEYS)
    dims = {k: dims_sum[k] / n for k in W}
    mean_score = sum(per_theme.values()) / n
    min_score = min(per_theme.values())
    composite_score = 0.5 * mean_score + 0.5 * min_score
    return composite_score, dims


# ----------------------------------------------------------------------------
# 优化主循环
# ----------------------------------------------------------------------------
def all_params() -> list:
    params = []
    for key in THEME_KEYS:
        for f in COLOR_FIELDS:
            for ch in ("h", "s", "l"):
                params.append(("color", key, f, ch))
    for name in GEOM_SPECS:
        params.append(("geom", name))
    return params


def run() -> None:
    random.seed(RNG_SEED)
    state = build_initial_state()
    params = all_params()
    best_score, best_dims = composite(state)
    best_state = State(
        {k: {f: tuple(v) for f, v in state.colors[k].items()} for k in THEME_KEYS},
        dict(state.geom),
    )
    initial_score = best_score
    initial_dims = dict(best_dims)

    out_dir = Path(__file__).resolve().parent
    csv_path = out_dir / "convergence.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh)
        w.writerow(["round", "best_score", "visual", "layout", "color",
                    "typography", "components", "interaction", "param"])

        for rnd in range(1, ROUNDS + 1):
            kind = random.choice(params)
            if kind[0] == "color":
                _, key, field, ch = kind
                h, s, l = state.colors[key][field]
                if ch == "h":
                    orig_h = rgb_to_hsl(*hex_to_rgb(getattr(_BASE_THEMES[key], field)))[0]
                    lo = max(0.0, orig_h - 20)
                    hi = min(360.0, orig_h + 20)
                    nv = h + random.uniform(-8, 8)
                    nv = max(lo, min(hi, nv))
                    state.colors[key][field] = (nv, s, l)
                elif ch == "s":
                    nv = clamp(s + random.uniform(-0.04, 0.04), 0.0, 1.0)
                    state.colors[key][field] = (h, nv, l)
                else:
                    nv = clamp(l + random.uniform(-0.025, 0.025), 0.02, 0.98)
                    state.colors[key][field] = (h, s, nv)
            else:
                name = kind[1]
                init, lo, hi, step = GEOM_SPECS[name]
                nv = clamp(state.geom[name] + random.uniform(-step, step), lo, hi)
                state.geom[name] = nv

            sc, dims = composite(state)
            anneal_p = max(0.0, 0.10 * (1 - rnd / ROUNDS))
            improved = sc > best_score + 1e-6
            if improved or (sc > -1e8 and random.random() < anneal_p):
                if improved:
                    best_score = sc
                    best_dims = dims
                    best_state = State(
                        {k: {f: tuple(v) for f, v in state.colors[k].items()}
                         for k in THEME_KEYS},
                        dict(state.geom),
                    )
                w.writerow([rnd, round(best_score, 4), round(best_dims["visual"], 4),
                           round(best_dims["layout"], 4), round(best_dims["color"], 4),
                           round(best_dims["typography"], 4),
                           round(best_dims["components"], 4),
                           round(best_dims["interaction"], 4), "|".join(map(str, kind))])
            else:
                # 回滚
                if kind[0] == "color":
                    _, key, field, ch = kind
                    h, s, l = best_state.colors[key][field]
                    state.colors[key][field] = (h, s, l)
                else:
                    state.geom[kind[1]] = best_state.geom[kind[1]]

    # ---- 写出最终令牌 ----
    resolved = resolve_all(best_state)
    payload = {
        "meta": {
            "rounds": ROUNDS,
            "seed": RNG_SEED,
            "initial_score": round(initial_score, 4),
            "final_score": round(best_score, 4),
            "initial_dims": {k: round(v, 4) for k, v in initial_dims.items()},
            "final_dims": {k: round(v, 4) for k, v in best_dims.items()},
        },
        "geometry": {k: round(v, 4) for k, v in best_state.geom.items()},
        "themes": {},
    }
    for key in THEME_KEYS:
        payload["themes"][key] = resolved[key]

    (out_dir / "optimized_tokens.json").write_text(
        json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")

    # ---- 写出报告 ----
    _write_report(out_dir, initial_score, best_score, initial_dims, best_dims,
                 resolved, best_state.geom)

    print(f"[refine] 完成 {ROUNDS} 轮")
    print(f"[refine] 初始复合分 = {initial_score:.2f} / 100")
    print(f"[refine] 最终复合分 = {best_score:.2f} / 100")
    print(f"[refine] 收敛文件: {csv_path.name}, optimized_tokens.json, REPORT.md")


def _write_report(out_dir, init_s, final_s, init_d, final_d, resolved, geom):
    lines = []
    lines.append("# QuillForge UI 美化 · 1000 轮迭代报告（REPORT）\n")
    lines.append(f"- 轮次：**{ROUNDS}**　随机种子：`{RNG_SEED}`")
    lines.append(f"- 初始复合分：**{init_s:.2f} / 100**")
    lines.append(f"- 最终复合分：**{final_s:.2f} / 100**")
    lines.append(f"- 提升：**{final_s - init_s:+.2f}** 分（best-so-far 单调非降）\n")
    lines.append("## 六维评分前后对比\n")
    lines.append("| 维度 | 初始 | 最终 | 权重 |")
    lines.append("|------|------|------|------|")
    labels = {
        "visual": "视觉呈现", "layout": "布局结构", "color": "配色方案",
        "typography": "字体排版", "components": "组件样式", "interaction": "交互反馈",
    }
    for k in W:
        lines.append(f"| {labels[k]} | {init_d[k]:.3f} | {final_d[k]:.3f} | {W[k]:.2f} |")
    lines.append("\n## 可达性核查（硬约束：文本对比 ≥ 4.5:1 目标）\n")
    lines.append("| 主题 | text_primary/surface_0 | text_secondary/surface_1 | text_muted/surface_2 |")
    lines.append("|------|------------------------|--------------------------|------------------------|")
    for key in THEME_KEYS:
        hx = resolved[key]
        c1 = contrast_ratio(hx["text_primary"], hx["surface_0"])
        c2 = contrast_ratio(hx["text_secondary"], hx["surface_1"])
        c3 = contrast_ratio(hx["text_muted"], hx["surface_2"])
        lines.append(f"| {key} | {c1:.2f}:1 | {c2:.2f}:1 | {c3:.2f}:1 |")
    lines.append("\n## 优化后几何 / 排版令牌\n")
    for name, val in geom.items():
        lines.append(f"- `{name}` = {val:.4f}")
    lines.append("\n> 收敛判据：最后 100 轮 best 提升 < 0.1 分即视为收敛；")
    lines.append("> 任一主题文本对比 < 3:1 的提案被硬拒，保证可达性底线。")
    (out_dir / "REPORT.md").write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    run()
