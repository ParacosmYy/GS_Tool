"""仪表盘布局自动持久化 helper（Batch 25）。

DashboardPanel 此前只有手动「保存/加载布局」按钮，关掉应用后布局丢失。
本 helper 提供自动持久化：build 时从应用数据目录恢复上一次布局，add/remove
控件时自动写回。布局存为 JSON 到 ``QStandardPaths.AppDataLocation`` 下的
``embeddebug/dashboard_layout.json``。

约束：只依赖 PyQt6 + dashboard canvas + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import json
import os
from pathlib import Path

LAYOUT_FILENAME = "dashboard_layout.json"


def _app_data_dir() -> Path:
    """解析应用数据目录（QStandardPaths.AppDataLocation）。"""

    from PyQt6.QtCore import QStandardPaths

    locations = QStandardPaths.standardLocations(QStandardPaths.StandardLocation.AppDataLocation)
    for loc in locations:
        if loc:
            return Path(loc)
    # 兜底：用户家目录下的 .embeddebug。
    return Path.home() / ".embeddebug"


def layout_path() -> Path:
    """仪表盘布局 JSON 文件路径。"""

    return _app_data_dir() / "embeddebug" / LAYOUT_FILENAME


def load_layout_dict() -> dict:
    """读取已保存的布局 dict；不存在或损坏返回空 dict。"""

    p = layout_path()
    if not p.exists():
        return {}
    try:
        raw = json.loads(p.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    return raw if isinstance(raw, dict) else {}


def save_layout_dict(layout: dict) -> bool:
    """原子写入布局 dict，返回是否成功。"""

    p = layout_path()
    try:
        p.parent.mkdir(parents=True, exist_ok=True)
        tmp = p.with_suffix(p.suffix + ".tmp")
        tmp.write_text(json.dumps(layout, ensure_ascii=False, indent=2), encoding="utf-8")
        os.replace(tmp, p)
    except OSError:
        return False
    return True


def restore_to_canvas(canvas) -> int:
    """把已保存布局恢复到画布，返回恢复的控件数。

    用 canvas.load_layout 接口（接受 JSON 文件路径）的内存变体：先写到临时路径
    再调 load_layout，避免重复解析逻辑。
    """

    layout = load_layout_dict()
    items = layout.get("items")
    if not isinstance(items, list) or not items:
        return 0
    # 写到临时文件再用 canvas.load_layout（复用其 type/x/y/width/height/config 解析）。
    import tempfile

    try:
        fd, tmp_name = tempfile.mkstemp(suffix=".json", prefix="dashboard_restore.")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as fh:
                json.dump(layout, fh)
            return canvas.load_layout(tmp_name)
        finally:
            try:
                os.remove(tmp_name)
            except OSError:
                pass
    except OSError:
        return 0


def persist_from_canvas(canvas) -> bool:
    """把画布当前布局持久化（add/remove 后自动保存），返回是否成功。"""

    try:
        layout = canvas.to_layout_dict()
    except Exception:
        return False
    return save_layout_dict(layout)


def load_tabs_layout() -> dict:
    """读取多标签页布局 dict（{tab_name: layout_dict}）。

    Batch 27：兼容旧单画布格式（顶层含 ``items`` 列表时归入默认标签页 "Dashboard 1"）。
    """

    raw = load_layout_dict()
    # 旧格式：顶层是 {"items": [...]}（单画布）。归入默认标签页。
    if isinstance(raw.get("items"), list):
        return {"Dashboard 1": raw}
    # 新格式：{tab_name: {"items": [...]}}。
    if isinstance(raw, dict):
        return {k: v for k, v in raw.items() if isinstance(v, dict)}
    return {}


def save_tabs_layout(tabs_layout: dict) -> bool:
    """写入多标签页布局 dict（{tab_name: layout_dict}）。"""

    return save_layout_dict(tabs_layout)


def persist_all_tabs(tabs) -> bool:
    """把 DashboardTabs 所有标签页布局持久化（按 tab 名为 key），返回是否成功。

    Batch 27：修复旧版仅持久化 current_canvas 的数据丢失（其他标签页布局丢失）。
    激活 DashboardTabs.tab_names（此前零消费者）。
    """

    try:
        names = tabs.tab_names()
    except Exception:
        return False
    tabs_layout: dict[str, dict] = {}
    for index, name in enumerate(names):
        canvas = tabs.widget(index) if hasattr(tabs, "widget") else None
        if canvas is None:
            continue
        try:
            tabs_layout[name] = canvas.to_layout_dict()
        except Exception:
            continue
    return save_tabs_layout(tabs_layout)


def restore_all_tabs(tabs) -> int:
    """把已保存的多标签页布局恢复到 DashboardTabs，返回恢复的总控件数。

    按保存的 tab 名匹配当前标签页（同名恢复）；保存里有但当前无的标签页跳过。
    """

    tabs_layout = load_tabs_layout()
    if not tabs_layout:
        return 0
    try:
        names = tabs.tab_names()
    except Exception:
        return 0
    restored = 0
    for index, name in enumerate(names):
        layout = tabs_layout.get(name)
        if not layout or not isinstance(layout.get("items"), list):
            continue
        canvas = tabs.widget(index) if hasattr(tabs, "widget") else None
        if canvas is None:
            continue
        restored += _apply_layout_to_canvas(canvas, layout)
    return restored


def _apply_layout_to_canvas(canvas, layout: dict) -> int:
    """把单个 layout_dict 应用到画布（复用 canvas.load_layout 的文件接口）。"""

    import tempfile

    try:
        fd, tmp_name = tempfile.mkstemp(suffix=".json", prefix="dashboard_tab_restore.")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as fh:
                json.dump(layout, fh)
            return canvas.load_layout(tmp_name)
        finally:
            try:
                os.remove(tmp_name)
            except OSError:
                pass
    except OSError:
        return 0


# ── 手动 save/load 对话框 helper（Batch 49：从 dashboard_panel 抽出） ─────
def save_layout_via_dialog(panel) -> None:
    """弹 QFileDialog 选保存路径，把当前画布布局写入（带状态反馈）。"""

    from PyQt6.QtWidgets import QFileDialog

    tabs = getattr(panel, "_tabs", None)
    widget = getattr(panel, "_widget", None)
    if tabs is None or widget is None:
        return
    canvas = tabs.current_canvas()
    if canvas is None:
        return
    path, _ = QFileDialog.getSaveFileName(
        widget, widget.tr("保存仪表盘布局"), "",
        widget.tr("Dashboard layout (*.json)"),
    )
    if not path:
        return
    try:
        canvas.save_layout(path)
        panel._status.setText(widget.tr("布局已保存：{path}").format(path=path))
    except OSError as exc:
        panel._status.setText(widget.tr("保存失败：{err}").format(err=exc))


def load_layout_via_dialog(panel) -> None:
    """弹 QFileDialog 选加载路径，把布局应用到当前画布（带状态反馈）。"""

    from PyQt6.QtWidgets import QFileDialog

    tabs = getattr(panel, "_tabs", None)
    widget = getattr(panel, "_widget", None)
    if tabs is None or widget is None:
        return
    canvas = tabs.current_canvas()
    if canvas is None:
        return
    path, _ = QFileDialog.getOpenFileName(
        widget, widget.tr("加载仪表盘布局"), "",
        widget.tr("Dashboard layout (*.json)"),
    )
    if not path:
        return
    try:
        count = canvas.load_layout(path)
        panel._status.setText(widget.tr("已加载 {n} 个控件").format(n=count))
    except (OSError, ValueError, KeyError) as exc:
        panel._status.setText(widget.tr("加载失败：{err}").format(err=exc))


# ── Batch 49：autosave 入口（从 dashboard_panel 抽出） ───────────────
def restore_panel_layout(panel) -> None:
    """build 时从应用数据目录恢复上一次仪表盘布局（Batch 25/27/49）。"""

    if not _autosave_enabled_env():
        return
    try:
        tabs = getattr(panel, "_tabs", None)
        if tabs is None:
            return
        restore_all_tabs(tabs)
    except Exception:
        pass  # 恢复失败不阻塞面板构建（用户可手动加载）。


def persist_panel_layout(panel) -> None:
    """add/remove 后把全部标签页布局写回应用数据目录（Batch 25/27/49）。"""

    if not _autosave_enabled_env():
        return
    try:
        tabs = getattr(panel, "_tabs", None)
        if tabs is None:
            return
        persist_all_tabs(tabs)
    except Exception:
        pass  # 自动保存失败静默（手动保存按钮仍可用）。


def _autosave_enabled_env() -> bool:
    """是否启用仪表盘布局自动持久化（env EMBEDDEBUG_DASHBOARD_AUTOSAVE=1）。"""

    import os

    return os.environ.get("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "") == "1"
