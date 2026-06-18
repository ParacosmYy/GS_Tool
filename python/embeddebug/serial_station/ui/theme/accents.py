"""强调色变体（多配色）—— 运行时 accent override 真相源。

本模块提供 7 套强调色（青/蓝/紫/绿/琥珀/玫瑰/青绿），每套含 dark + light
两套色调。基础深/浅主题（背景/文本/状态色等）由 ``palette`` / ``palette_light``
固定提供，**只有强调色调可由用户运行时切换**。

设计要点（为什么是 runtime override 而非新增 palette 模块）：

1. ``palette.py`` 的 ``ACCENT == "#22d3ee"`` 被
   ``test_theme_manager.test_palette_constants_match_modern_dark_industrial_palette``
   硬断言锁定；``palette_light.py`` 的 ``ACCENT.startswith("#0")`` 被
   ``test_theme_switcher.test_light_palette_accent_is_consistent_brand`` 锁定。
   → 模块常量保持「青色 = 默认」不动，**零视觉回归、零测试改动**。
2. 强调色不仅出现在 QSS 字符串，还在代码里被消费：
   ``micro_interactions._HoverLiftFilter._animate_shadow`` 读 accent 喂
   ``QGraphicsDropShadowEffect``。纯 QSS 字符串替换够不到这条路径。
   → 需要一个运行时访问器（``get_active_accent``）让代码侧也能取到当前色。
3. QSS 侧重着色复用 ``theme_switcher.build_light_qss`` 已验证的两阶段占位符
   替换手法（longest-first + ``\\x00`` placeholder），避免链式替换。

cyan 变体的 dark/light 色调与 ``palette`` / ``palette_light`` 现有常量严格一致，
是恒等性测试（``apply_accent_recolor(qss, cyan) == qss``）的依据。

约束：本模块只依赖标准库 + 同包 palette，不 import PyQt、不访问 controller/transport。
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class AccentTones:
    """单一明度下的强调色 7 元组（base/hover/pressed/soft/border/gradient_from/to）。

    ``soft`` / ``border`` 存 ``rgba(r, g, b, alpha)`` 串，alpha 与 cyan 默认一致
    （soft=0.12/0.10，border=0.35），仅 RGB 随 base 变化，保持透明度节奏统一。
    """

    base: str
    hover: str
    pressed: str
    soft: str
    border: str
    gradient_from: str
    gradient_to: str

    def as_recolor_map(self) -> dict[str, str]:
        """返回该色调的「语义键 -> 值」映射，键名与 ``AccentVariant.recolor_pairs`` 对齐。"""

        return {
            "base": self.base,
            "hover": self.hover,
            "pressed": self.pressed,
            "soft": self.soft,
            "border": self.border,
            "gradient_from": self.gradient_from,
            "gradient_to": self.gradient_to,
        }


@dataclass(frozen=True)
class AccentVariant:
    """一套强调色变体：dark + light 双明度色调。

    ``id`` 是稳定标识（存档/测试用），``label_key`` 是 settings 面板 ``tr()``
    翻译键（中文展示名）。
    """

    id: str
    label_key: str
    dark: AccentTones
    light: AccentTones

    def tones_for(self, is_light: bool) -> AccentTones:
        """按当前主题明度取对应色调。"""

        return self.light if is_light else self.dark


def _soft(rgb: tuple[int, int, int], alpha: float) -> str:
    # alpha 格式化为 2 位小数（与 palette/palette_light 的 rgba 写法一致，
    # 保证 cyan 变体恒等性测试 string-equal 通过）。
    return f"rgba({rgb[0]}, {rgb[1]}, {rgb[2]}, {alpha:.2f})"


# ── 7 套强调色变体 ────────────────────────────────────────────────
# cyan 的 dark/light 色调与 palette.py / palette_light.py 现有常量严格一致，
# 保证默认状态下 recolor 为恒等变换（零视觉回归）。
# 其余 6 套按「base -> hover(提亮) / pressed(加深)」的同一派生规律构造，
# gradient_to 沿用跨色相品牌渐变思路（每套选一个同冷暖调的辅助色）。
_ACCENTS: tuple[AccentVariant, ...] = (
    AccentVariant(
        id="cyan",
        label_key="青",
        dark=AccentTones(
            base="#22d3ee", hover="#67e8f9", pressed="#0891b2",
            soft=_soft((34, 211, 238), 0.12), border=_soft((34, 211, 238), 0.35),
            gradient_from="#22d3ee", gradient_to="#3b82f6",
        ),
        light=AccentTones(
            base="#0891b2", hover="#06b6d4", pressed="#0e7490",
            soft=_soft((8, 145, 178), 0.10), border=_soft((8, 145, 178), 0.35),
            gradient_from="#0891b2", gradient_to="#2563eb",
        ),
    ),
    AccentVariant(
        id="blue",
        label_key="蓝",
        dark=AccentTones(
            base="#3b82f6", hover="#60a5fa", pressed="#1d4ed8",
            soft=_soft((59, 130, 246), 0.12), border=_soft((59, 130, 246), 0.35),
            gradient_from="#3b82f6", gradient_to="#06b6d4",
        ),
        light=AccentTones(
            base="#2563eb", hover="#3b82f6", pressed="#1d4ed8",
            soft=_soft((37, 99, 235), 0.10), border=_soft((37, 99, 235), 0.35),
            gradient_from="#2563eb", gradient_to="#0891b2",
        ),
    ),
    AccentVariant(
        id="purple",
        label_key="紫",
        dark=AccentTones(
            base="#a78bfa", hover="#c4b5fd", pressed="#7c3aed",
            soft=_soft((167, 139, 250), 0.14), border=_soft((167, 139, 250), 0.35),
            gradient_from="#a78bfa", gradient_to="#ec4899",
        ),
        light=AccentTones(
            base="#7c3aed", hover="#8b5cf6", pressed="#6d28d9",
            soft=_soft((124, 58, 237), 0.10), border=_soft((124, 58, 237), 0.35),
            gradient_from="#7c3aed", gradient_to="#db2777",
        ),
    ),
    AccentVariant(
        id="green",
        label_key="绿",
        dark=AccentTones(
            base="#22c55e", hover="#4ade80", pressed="#15803d",
            soft=_soft((34, 197, 94), 0.12), border=_soft((34, 197, 94), 0.35),
            gradient_from="#22c55e", gradient_to="#14b8a6",
        ),
        light=AccentTones(
            base="#16a34a", hover="#22c55e", pressed="#15803d",
            soft=_soft((22, 163, 74), 0.10), border=_soft((22, 163, 74), 0.35),
            gradient_from="#16a34a", gradient_to="#0d9488",
        ),
    ),
    AccentVariant(
        id="amber",
        label_key="琥珀",
        dark=AccentTones(
            base="#f59e0b", hover="#fbbf24", pressed="#b45309",
            soft=_soft((245, 158, 11), 0.14), border=_soft((245, 158, 11), 0.35),
            gradient_from="#f59e0b", gradient_to="#ef4444",
        ),
        light=AccentTones(
            base="#d97706", hover="#f59e0b", pressed="#b45309",
            soft=_soft((217, 119, 6), 0.10), border=_soft((217, 119, 6), 0.35),
            gradient_from="#d97706", gradient_to="#dc2626",
        ),
    ),
    AccentVariant(
        id="rose",
        label_key="玫瑰",
        dark=AccentTones(
            base="#f43f5e", hover="#fb7185", pressed="#be123c",
            soft=_soft((244, 63, 94), 0.14), border=_soft((244, 63, 94), 0.35),
            gradient_from="#f43f5e", gradient_to="#a78bfa",
        ),
        light=AccentTones(
            base="#e11d48", hover="#f43f5e", pressed="#be123c",
            soft=_soft((225, 29, 72), 0.10), border=_soft((225, 29, 72), 0.35),
            gradient_from="#e11d48", gradient_to="#7c3aed",
        ),
    ),
    AccentVariant(
        id="teal",
        label_key="青绿",
        dark=AccentTones(
            base="#14b8a6", hover="#2dd4bf", pressed="#0f766e",
            soft=_soft((20, 184, 166), 0.12), border=_soft((20, 184, 166), 0.35),
            gradient_from="#14b8a6", gradient_to="#22d3ee",
        ),
        light=AccentTones(
            base="#0d9488", hover="#14b8a6", pressed="#0f766e",
            soft=_soft((13, 148, 136), 0.10), border=_soft((13, 148, 136), 0.35),
            gradient_from="#0d9488", gradient_to="#0891b2",
        ),
    ),
)

# id -> variant 索引（启动期一次性构建，O(1) 查找）。
_ACCENTS_BY_ID: dict[str, AccentVariant] = {a.id: a for a in _ACCENTS}

# 公开只读元组，供 settings 面板与测试消费（顺序即展示顺序）。
ACCENTS: tuple[AccentVariant, ...] = _ACCENTS

DEFAULT_ACCENT_ID = "cyan"

# 运行时活动 accent（None == cyan 默认，等价于「未 override」）。
# 模块级单例：整个进程共享一个 accent 状态，与 QSS 生成器/微交互一致。
_active_override: AccentVariant | None = None


def get_accent_by_id(accent_id: str) -> AccentVariant:
    """按 id 取变体；未知 id 回退到 cyan 默认。"""

    return _ACCENTS_BY_ID.get(accent_id, _ACCENTS_BY_ID[DEFAULT_ACCENT_ID])


def get_active_accent() -> AccentVariant:
    """返回当前生效的 accent 变体（未 override 时返回 cyan）。"""

    return _active_override if _active_override is not None else _ACCENTS_BY_ID[DEFAULT_ACCENT_ID]


def get_active_accent_id() -> str:
    """返回当前生效 accent 的 id（未 override 时返回 "cyan"）。"""

    return get_active_accent().id


def set_active_accent(accent_id: str) -> AccentVariant:
    """切换运行时 accent。返回实际生效的变体（未知 id 回退 cyan）。

    仅更新模块级状态；不触发 QSS 重生成（由调用方 ``theme_switcher`` 负责）。
    """

    global _active_override
    variant = get_accent_by_id(accent_id)
    _active_override = variant if variant.id != DEFAULT_ACCENT_ID else None
    return variant


def reset_active_accent() -> None:
    """重置 accent 状态为 cyan 默认（仅测试用）。"""

    global _active_override
    _active_override = None


def current_theme_is_light() -> bool:
    """查询当前生效主题是否浅色（供代码侧 accent 消费者如 micro_interactions 用）。

    读 ``ThemeManager`` 单例的 ``current_theme``；未应用过主题（None）按深色处理。
    放在这里（而非 micro_interactions 内联）是因为 micro_interactions 不应直接
    依赖 ThemeManager，而 accents 是 theme 子包成员，引用同包 manager 合理。
    """

    from embeddebug.serial_station.ui.theme.manager import ThemeManager

    return ThemeManager().current_theme == "serial_station_light"


def active_accent_border() -> str:
    """返回当前 accent 在当前主题明度下的 border 色（代码侧 hover 辉光用）。

    micro_interactions._HoverLiftFilter 把此值喂 ``QGraphicsDropShadowEffect.color``，
    让 hover 辉光跟随用户选中的 accent（而非写死 cyan）。
    """

    return get_active_accent().tones_for(current_theme_is_light()).border


def active_accent_base() -> str:
    """返回当前 accent 在当前主题明度下的 base 色（代码侧 focus 光环用）。

    micro_interactions.install_focus_ring 把此值喂 ``QGraphicsDropShadowEffect.color``，
    让 focus 光环跟随用户选中的 accent。
    """

    return get_active_accent().tones_for(current_theme_is_light()).base
