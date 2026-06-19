"""SvdPanel 的树/详情构建 helper（从 ``svd_panel.py`` 拆出，保持单文件 ≤300 行）。

把 ``SvdDevice`` → ``QTreeWidget`` 三级节点（外设/寄存器/位域），以及把
``SvdRegister`` 格式化成详情字典（地址/复位/位域表行）。仅依赖 PyQt6 + svd 公共
API，不 import transport，不反向依赖 UI 面板内部状态。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QTreeWidget, QTreeWidgetItem, QWidget

from embeddebug.serial_station.svd import (
    Access,
    SvdDevice,
    SvdPeripheral,
    SvdRegister,
)


def _hex(value: int, width: int = 8) -> str:
    """格式化为 ``0x`` 前缀的定宽十六进制（默认 8 位）。"""

    return f"0x{value:0{width}X}"


def _access_label(access: Access) -> str:
    """access 枚举 → 简短中文标签（详情表用）。"""

    return {
        Access.READ_ONLY: "只读",
        Access.WRITE_ONLY: "只写",
        Access.READ_WRITE: "读写",
    }.get(access, "读写")


def populate_tree(tree: QTreeWidget, device: SvdDevice, tr) -> None:
    """把 ``device`` 的外设/寄存器/位域填进 ``tree``（三级）。

    节点列：名称 | 地址/偏移 | 描述（截断）。
    节点 ``data(0, Qt.UserRole)`` 存对象本体，供选中时回查。
    """

    tree.clear()
    for periph in device.peripherals:
        p_item = QTreeWidgetItem([
            periph.name,
            _hex(periph.base_address),
            _trunc(periph.description),
        ])
        p_item.setData(0, 0x0100, periph)  # Qt.UserRole = 0x0100
        for reg in periph.registers:
            r_item = QTreeWidgetItem([
                reg.name,
                _hex(reg.address_offset, 3),
                _trunc(reg.description),
            ])
            r_item.setData(0, 0x0100, reg)
            for field in reg.fields:
                f_item = QTreeWidgetItem([
                    field.name,
                    f"bit {field.bit_offset}-{field.bit_end}"
                    if field.bit_width > 1
                    else f"bit {field.bit_offset}",
                    _trunc(field.description),
                ])
                f_item.setData(0, 0x0100, field)
                r_item.addChild(f_item)
            p_item.addChild(r_item)
        tree.addTopLevelItem(p_item)
    tree.expandItem(tree.topLevelItem(0)) if tree.topLevelItem(0) else None


def register_detail_rows(reg: SvdRegister, periph: SvdPeripheral, tr) -> list[tuple]:
    """把 ``reg`` 格式化成详情表行（供 ``QFormLayout`` / ``QTableWidget``）。

    返回 [(标签, 值), ...]，调用方决定渲染方式。位域单独成子表（见下方）。
    """

    return [
        (tr("寄存器"), reg.name),
        (tr("所属外设"), periph.name),
        (tr("偏移"), _hex(reg.address_offset, 3)),
        (tr("绝对地址"), _hex(reg.absolute_address(periph.base_address))),
        (tr("位宽"), f"{reg.size} bit"),
        (tr("访问"), _access_label(reg.access)),
        (tr("复位值"), _hex(reg.reset_value)),
    ]


def field_table_rows(reg: SvdRegister) -> list[list[str]]:
    """``reg`` 的位域 → [[名称, 位范围, 访问, 复位值], ...]（QTableWidget 行）。"""

    rows: list[list[str]] = []
    for f in reg.fields:
        rng = f"bit {f.bit_offset}-{f.bit_end}" if f.bit_width > 1 else f"bit {f.bit_offset}"
        rows.append([f.name, rng, _access_label(f.access), _hex(f.reset_value, 2)])
    return rows


def _trunc(text: str, limit: int = 40) -> str:
    """描述超长截断 + 省略号（树列窄，避免横向溢出）。"""

    if not text:
        return ""
    return text if len(text) <= limit else text[: limit - 1] + "…"


__all__ = [
    "field_table_rows",
    "populate_tree",
    "register_detail_rows",
]
