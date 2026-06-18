"""Serial Station UI 尺寸/间距/字号 token。

与 ``palette.py`` 配合，作为 QSS 生成器的唯一尺寸真相源，
杜绝散落硬编码像素值。数值取自 VOFA+ 工业风观感基线。
"""

from __future__ import annotations

# ── 圆角 ───────────────────────────────────────────────────────────
RADIUS_NONE = "0px"
RADIUS_SM = "4px"   # 小控件（徽章、标签）
RADIUS_MD = "6px"   # 输入框、按钮、下拉
RADIUS_LG = "8px"   # 卡片、面板
RADIUS_PILL = "12px"  # 状态药丸

# ── 间距 ───────────────────────────────────────────────────────────
SPACING_NONE = "0px"
SPACING_XS = "4px"
SPACING_SM = "6px"
SPACING_MD = "8px"
SPACING_LG = "12px"
SPACING_XL = "16px"
SPACING_2XL = "24px"

# ── 内边距 ─────────────────────────────────────────────────────────
PADDING_SM = "4px 8px"     # 紧凑按钮
PADDING_MD = "6px 12px"    # 普通按钮
PADDING_LG = "8px 16px"    # 主按钮、强调按钮
PADDING_INPUT = "5px 8px"  # 输入框

# ── 边框宽度 ───────────────────────────────────────────────────────
BORDER_NONE = "0px"
BORDER_THIN = "1px"
BORDER_THICK = "2px"

# ── 字号 ───────────────────────────────────────────────────────────
FONT_XS = "11px"
FONT_SM = "12px"
FONT_BASE = "13px"
FONT_MD = "14px"
FONT_LG = "16px"
FONT_XL = "18px"

FONT_FAMILY = '"Microsoft YaHei UI", "Segoe UI", sans-serif'
FONT_FAMILY_MONO = 'Consolas, "Cascadia Code", "JetBrains Mono", monospace'

# ── 控件高度 ───────────────────────────────────────────────────────
CONTROL_HEIGHT_SM = "24px"
CONTROL_HEIGHT_MD = "28px"
CONTROL_HEIGHT_LG = "32px"


def all_tokens() -> dict[str, str]:
    """返回尺寸 token 名 -> 值的有序映射，供 QSS 生成与测试校验。"""

    return {
        "radius_none": RADIUS_NONE,
        "radius_sm": RADIUS_SM,
        "radius_md": RADIUS_MD,
        "radius_lg": RADIUS_LG,
        "radius_pill": RADIUS_PILL,
        "spacing_none": SPACING_NONE,
        "spacing_xs": SPACING_XS,
        "spacing_sm": SPACING_SM,
        "spacing_md": SPACING_MD,
        "spacing_lg": SPACING_LG,
        "spacing_xl": SPACING_XL,
        "spacing_2xl": SPACING_2XL,
        "padding_sm": PADDING_SM,
        "padding_md": PADDING_MD,
        "padding_lg": PADDING_LG,
        "padding_input": PADDING_INPUT,
        "border_none": BORDER_NONE,
        "border_thin": BORDER_THIN,
        "border_thick": BORDER_THICK,
        "font_xs": FONT_XS,
        "font_sm": FONT_SM,
        "font_base": FONT_BASE,
        "font_md": FONT_MD,
        "font_lg": FONT_LG,
        "font_xl": FONT_XL,
        "font_family": FONT_FAMILY,
        "font_family_mono": FONT_FAMILY_MONO,
        "control_height_sm": CONTROL_HEIGHT_SM,
        "control_height_md": CONTROL_HEIGHT_MD,
        "control_height_lg": CONTROL_HEIGHT_LG,
    }
