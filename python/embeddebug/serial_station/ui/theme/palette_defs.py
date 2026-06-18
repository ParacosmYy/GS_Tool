"""Serial Station 色板定义 — 主题变体的单一真相源。

把原 ``palette.py`` 的模块级常量重构为 ``Palette`` 数据类，支持多主题变体：
- ``Palette.dark()``：VOFA+ 精致工业风深色（PRD-071，与原 ``palette.py`` 常量逐字一致）。
- ``Palette.light()``：对齐 EK-OmniProbe 默认浅色玻璃拟态观感（``index.css`` ``:root``）。

设计要点：
- 字段名与原 ``palette.all_tokens()`` 的 key 全小写映射一致（``bg_window`` / ``accent`` …），
  保证 ``test_palette_tokens_contain_all_expected_keys`` 对两个变体都成立。
- ``palette.py`` 保留为深色默认 shim：模块级常量仍是深色值，满足
  ``test_palette_constants_match_modern_dark_industrial_palette`` 的硬断言，
  并提供 ``default_palette()`` 返回与常量一致的 ``Palette``。
- section 函数通过 ``active_palette()`` 解析当前激活色板（调用期求值），
  支持运行时切换；详见 ``manager.py`` 的 ``use_palette``。

约束：本模块只依赖标准库 + dataclasses，不 import PyQt 或其它项目层。
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Palette:
    """单套色板值。字段名对应 QSS 生成器与测试期望的全小写 key。"""

    # ── 背景分层 ──
    bg_window: str
    bg_app: str
    bg_panel: str
    bg_panel_raised: str
    bg_input: str
    bg_input_focus: str
    bg_disabled: str
    bg_selection: str
    bg_overlay: str
    # ── 文本分层 ──
    text_primary: str
    text_secondary: str
    text_muted: str
    text_disabled: str
    text_on_accent: str
    text_inverted: str
    # ── 强调色 ──
    accent: str
    accent_hover: str
    accent_pressed: str
    accent_soft: str
    accent_border: str
    # ── 状态色 ──
    success: str
    success_hover: str
    success_soft: str
    warning: str
    warning_hover: str
    warning_soft: str
    warning_border: str
    error: str
    error_hover: str
    error_soft: str
    # ── 终端/日志通道 ──
    term_rx: str
    term_tx: str
    term_timestamp: str
    term_system: str
    term_background: str
    term_selection: str
    # ── 边框 / 滚动条 / 阴影 ──
    border: str
    border_strong: str
    border_focus: str
    scrollbar: str
    scrollbar_hover: str
    scrollbar_background: str
    shadow: str
    # ── 玻璃卡片 / TopBar 视觉层 ──
    card_highlight_top: str
    card_highlight_bottom: str
    topbar_bg_top: str
    topbar_bg_bottom: str
    topbar_border: str
    brand_chip_bg: str
    brand_chip_border: str

    def as_token_map(self) -> dict[str, str]:
        """返回字段名 -> 值映射，key 与原 ``palette.all_tokens()`` 完全一致。"""

        from dataclasses import fields

        return {f.name: getattr(self, f.name) for f in fields(self)}


# ── 工厂：深色（VOFA+ 工业风，与 palette.py 常量逐字一致） ──────────────
def dark() -> Palette:
    """深色工业风色板（PRD-071，默认主题）。"""

    return Palette(
        bg_window="#0d1118",
        bg_app="#101218",
        bg_panel="#151b24",
        bg_panel_raised="#1b2634",
        bg_input="#0f141d",
        bg_input_focus="#111827",
        bg_disabled="#151b24",
        bg_selection="#203044",
        bg_overlay="rgba(13, 17, 24, 200)",
        text_primary="#d7def3",
        text_secondary="#b7c0d8",
        text_muted="#9aa7bd",
        text_disabled="#5f6f86",
        text_on_accent="#0d1118",
        text_inverted="#f8fafc",
        accent="#22d3ee",
        accent_hover="#67e8f9",
        accent_pressed="#0891b2",
        accent_soft="rgba(34, 211, 238, 0.12)",
        accent_border="rgba(34, 211, 238, 0.35)",
        success="#22c55e",
        success_hover="#4ade80",
        success_soft="rgba(34, 197, 94, 0.10)",
        warning="#f59e0b",
        warning_hover="#fbbf24",
        warning_soft="rgba(245, 158, 11, 0.08)",
        warning_border="rgba(245, 158, 11, 0.35)",
        error="#ef4444",
        error_hover="#f87171",
        error_soft="rgba(239, 68, 68, 0.10)",
        term_rx="#22c55e",
        term_tx="#38bdf8",
        term_timestamp="#5f6f86",
        term_system="#9aa7bd",
        term_background="#0d1118",
        term_selection="#2563eb",
        border="#27313f",
        border_strong="#334155",
        border_focus="#22d3ee",
        scrollbar="#334155",
        scrollbar_hover="#475569",
        scrollbar_background="transparent",
        shadow="rgba(0, 0, 0, 0.35)",
        card_highlight_top="rgba(255, 255, 255, 0.05)",
        card_highlight_bottom="rgba(255, 255, 255, 0.00)",
        topbar_bg_top="#131a25",
        topbar_bg_bottom="#10141d",
        topbar_border="#2a3647",
        brand_chip_bg="rgba(34, 211, 238, 0.14)",
        brand_chip_border="rgba(34, 211, 238, 0.40)",
    )


# ── 工厂：浅色（EK-OmniProbe 默认玻璃拟态，index.css :root 映射） ────────
def light() -> Palette:
    """浅色玻璃色板，对齐 EK-OmniProbe 默认观感。

    映射来源：``EK-OmniProbe/src/index.css`` 的 ``:root`` token（HSL→hex）。
    终端 RX/TX 在浅底上保持高对比 vivid 色（绿/蓝），不照搬 EK-OmniProbe 的
    状态色，保证终端分色可读性。
    """

    return Palette(
        bg_window="#f4f6fb",        # --background
        bg_app="#ffffff",           # 应用容器：白
        bg_panel="#ffffff",         # --card：卡片浮在白上
        bg_panel_raised="#e6ebf4",  # --secondary：悬停/凸起
        bg_input="#ffffff",
        bg_input_focus="#f8faff",
        bg_disabled="#e4e8f1",      # --muted
        bg_selection="#daebfb",     # --accent：选中项
        bg_overlay="rgba(38, 41, 54, 120)",  # 浅色遮罩用深半透明，保证对比
        text_primary="#262936",     # --foreground
        text_secondary="#313749",   # --secondary-foreground
        text_muted="#636c83",       # --muted-foreground
        text_disabled="#9aa2b5",
        text_on_accent="#ffffff",   # --primary-foreground
        text_inverted="#262936",    # 反白文字在浅主题下用前景色
        accent="#3e69e0",           # --primary：蓝（替代深色的青）
        accent_hover="#3355c4",
        accent_pressed="#2847b0",
        accent_soft="rgba(62, 105, 224, 0.12)",
        accent_border="rgba(62, 105, 224, 0.40)",
        success="#30a66b",          # --success
        success_hover="#2a9460",
        success_soft="rgba(48, 166, 107, 0.12)",
        warning="#f5a314",          # --warning
        warning_hover="#d98f10",
        warning_soft="rgba(245, 163, 20, 0.12)",
        warning_border="rgba(245, 163, 20, 0.40)",
        error="#e03e3e",            # --destructive
        error_hover="#c83535",
        error_soft="rgba(224, 62, 62, 0.10)",
        term_rx="#16a34a",          # 浅底高对比绿
        term_tx="#2563eb",          # 浅底高对比蓝
        term_timestamp="#636c83",
        term_system="#313749",
        term_background="#ffffff",
        term_selection="#daebfb",
        border="#ccd3e1",           # --border
        border_strong="#b3bdd0",
        border_focus="#3e69e0",     # 同 accent
        scrollbar="#b3bdd0",
        scrollbar_hover="#9aa2b5",
        scrollbar_background="transparent",
        shadow="rgba(73, 93, 142, 0.12)",  # --shadow-1
        card_highlight_top="rgba(255, 255, 255, 0.45)",  # 浅底玻璃高光更明显
        card_highlight_bottom="rgba(255, 255, 255, 0.00)",
        topbar_bg_top="#ffffff",
        topbar_bg_bottom="#f4f6fb",
        topbar_border="#ccd3e1",
        brand_chip_bg="rgba(62, 105, 224, 0.12)",
        brand_chip_border="rgba(62, 105, 224, 0.40)",
    )


# ── 主题名 -> 工厂 注册表 ─────────────────────────────────────────────
PALETTE_FACTORIES: dict[str, "Palette"] = {}


def register_palette(name: str, palette: Palette) -> None:
    """注册一套命名色板（供 ThemeManager 按名解析）。"""

    PALETTE_FACTORIES[name] = palette


def palette_for(name: str) -> Palette:
    """按主题名返回色板，未注册时回退到深色默认。"""

    return PALETTE_FACTORIES.get(name, dark())
