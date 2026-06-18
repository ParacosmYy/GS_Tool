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

from embeddebug.serial_station.ui.theme.qss_sections_app import (
    nav_rail_section,
    ota_section,
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
from embeddebug.serial_station.ui.theme.qss_sections_layout import (
    cards_section,
    collapsible_section,
    command_palette_section,
    splitter_section,
    topbar_section,
    waveform_overlays_section,
    zones_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_widgets import (
    buttons_section,
    log_view_section,
    plaintext_section,
    scrollbar_section,
    shortcut_section,
    status_section,
    waveform_section,
)


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
        buttons_section(),
        inputs_section(),
        combos_section(),
        log_view_section(),
        waveform_section(),
        status_section(),
        scrollbar_section(),
        plaintext_section(),
        shortcut_section(),
    ]
    return "\n\n".join(sections) + "\n"
