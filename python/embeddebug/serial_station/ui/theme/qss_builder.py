"""Serial Station QSS 生成器（协调入口）。

程序化生成深色工业风完整 QSS，覆盖全部 ``serialStation*`` objectName。
颜色取自 ``palette``、尺寸取自 ``tokens``、分区样式取自 ``qss_sections_*``。

拆分动机：单文件 QSS 自然较长，受仓库运行时文件 ``<= 300 行`` 门禁约束，
按域拆为 ``qss_sections_core``（全局/窗口/标签/输入/下拉）、
``qss_sections_widgets``（按钮/日志/波形/状态/滚动条/快捷键）与
``qss_sections_layout``（三栏 Shell/玻璃卡片/TopBar）。

约束：本模块只依赖 palette/tokens + 本包分区模块，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as _dark_palette
from embeddebug.serial_station.ui.theme import palette_light as _light_palette
from embeddebug.serial_station.ui.theme.accents import AccentVariant

from embeddebug.serial_station.ui.theme.qss_sections_app import (
    nav_rail_section,
    ota_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_domain import (
    domain_panels_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_controls import (
    controls_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_core import (
    combos_section,
    global_section,
    inputs_section,
    labels_section,
    window_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_empty import (
    empty_state_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_layout import (
    cards_section,
    collapsible_section,
    splitter_section,
    topbar_section,
    zones_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_overlays import (
    command_palette_section,
    waveform_overlays_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_widgets import (
    buttons_section,
    log_view_section,
    plaintext_section,
    scrollbar_section,
    shortcut_section,
    status_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_waveform import waveform_section
from embeddebug.serial_station.ui.theme.qss_sections_toast import toast_section
from embeddebug.serial_station.ui.theme.qss_sections_log_options import log_options_section


def build_qss() -> str:
    """生成 Serial Station 深色工业风完整 QSS。"""

    sections = [
        global_section(),
        window_section(),
        labels_section(),
        topbar_section(),
        cards_section(),
        collapsible_section(),
        zones_section(),
        splitter_section(),
        command_palette_section(),
        waveform_overlays_section(),
        controls_section(),
        nav_rail_section(),
        ota_section(),
        domain_panels_section(),
        buttons_section(),
        inputs_section(),
        combos_section(),
        log_view_section(),
        waveform_section(),
        status_section(),
        scrollbar_section(),
        plaintext_section(),
        shortcut_section(),
        empty_state_section(),
        toast_section(),
        log_options_section(),
    ]
    return "\n\n".join(sections) + "\n"


# ── 强调色重着色（多配色） ──────────────────────────────────────────
# cyan 是 palette 的默认强调色，QSS 串里内嵌的是 cyan 字面值。切到其他 accent
# 变体时，把 QSS 里的 cyan 字面值替换为变体对应值。两阶段占位符替换手法与
# theme_switcher.build_light_qss 完全一致（longest-first + \x00 placeholder），
# 避免源值与目标值重叠时的链式替换。


def _cyan_to_accent_pairs(is_light: bool) -> list[tuple[str, str]]:
    """构建「cyan 当前值 -> cyan 对应语义键」的源值清单（按出现顺序）。

    返回的是 (source_value, source_key) 对，供 recolor 时按 key 查 variant 目标值。
    用 key 而非直接 (cyan_value, variant_value) 是因为 cyan 的多个 token 可能
    共享同一字面值（如 base/gradient_from/border_focus 都 == "#22d3ee"），
    按 key 映射保证语义不串。
    """

    palette = _light_palette if is_light else _dark_palette
    # 与 AccentTones 的 7 个语义键一一对应的 palette token 名。
    # 注意：accent_gradient（完整 qlineargradient 串）不单独替换，它由
    # gradient_from/to 拼成，recolor from/to 后 QSS 里的 gradient 串天然更新。
    return [
        (palette.ACCENT, "base"),
        (palette.ACCENT_HOVER, "hover"),
        (palette.ACCENT_PRESSED, "pressed"),
        (palette.ACCENT_SOFT, "soft"),
        (palette.ACCENT_BORDER, "border"),
        (palette.ACCENT_GRADIENT_FROM, "gradient_from"),
        (palette.ACCENT_GRADIENT_TO, "gradient_to"),
    ]


def apply_accent_recolor(qss: str, variant: AccentVariant, *, is_light: bool) -> str:
    """把 QSS 中的当前 cyan 强调色重着色为 ``variant`` 对应值。

    复用 ``build_light_qss`` 的两阶段占位符替换：源值（cyan 字面）按长度降序先
    换成唯一 ``\\x00`` 占位符，再换成变体目标值。降序防止短值是长值子串时误替换。

    当 ``variant`` 是 cyan（默认）时，源值 == 目标值，输出恒等于输入（恒等变换，
    由 ``test_apply_accent_recolor_cyan_is_identity`` 保证）。

    Args:
        qss: ``build_qss`` 产出的深色 QSS（cyan 字面）。
        variant: 目标强调色变体。
        is_light: 当前是否浅色主题（决定取变体的 dark 还是 light 色调，以及
            cyan 源值取深色 palette 还是浅色 palette）。
    """

    tones = variant.tones_for(is_light)
    target_map = tones.as_recolor_map()

    # 收集 source -> target（按 source 字面值去重；同一字面值只取第一个 key 的目标）。
    source_to_target: dict[str, str] = {}
    for source_value, key in _cyan_to_accent_pairs(is_light):
        if source_value in source_to_target:
            continue
        target_value = target_map[key]
        if source_value == target_value:
            continue  # 恒等：跳过（cyan 默认时全部跳过 -> 输出不变）。
        source_to_target[source_value] = target_value

    if not source_to_target:
        return qss  # 无需替换（cyan 默认或全恒等）。

    # 第一阶段：cyan 源值 -> 唯一占位符（longest-first 防子串误替）。
    placeholders: dict[str, str] = {}
    for index, source_val in enumerate(sorted(source_to_target, key=len, reverse=True)):
        placeholder = f"\x00ACCENT{index}\x00"
        placeholders[placeholder] = source_to_target[source_val]
        qss = qss.replace(source_val, placeholder)

    # 第二阶段：占位符 -> 变体目标值。
    for placeholder, target_val in placeholders.items():
        qss = qss.replace(placeholder, target_val)
    return qss


def _target_of(pairs: list[tuple[str, str]], source_val: str) -> str:
    """从 pairs 取 source_val 对应的 target（pairs 已按 source 去重）。"""

    for src, tgt in pairs:
        if src == source_val:
            return tgt
    return source_val  # 不应到达。
