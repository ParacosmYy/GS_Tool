"""QSS 覆盖率守护测试 — 保证程序化 QSS 覆盖全部 UI objectName。

扫描 ``serial_station/ui/`` 源码中的 ``setObjectName(...)`` 调用，
提取所有 ``serialStation*`` 名称，断言生成的 QSS 含对应选择器。
新增控件若未补 QSS 会被本测试拦截，杜绝主题与 UI 漂移。
"""

from __future__ import annotations

import re
from pathlib import Path

from embeddebug.serial_station.ui.theme.qss_builder import build_qss

REPO_ROOT = Path(__file__).resolve().parents[3]
UI_DIR = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"

# QShortcut objectName 在源码中通过 setObjectName 设置，但 QShortcut 无可视样式，
# 仅用于测试与可发现性，不要求 QSS 覆盖。
STYLE_EXEMPT_SUFFIXES = ("Shortcut",)

_OBJECTNAME_PATTERN = re.compile(r'setObjectName\(\s*["\']([A-Za-z0-9_]+)["\']\s*\)')


def _extract_serial_station_objectnames() -> set[str]:
    """扫描 ui/ 源码，提取所有 setObjectName(...) 中的 serialStation* 名称。"""

    names: set[str] = set()
    for path in UI_DIR.rglob("*.py"):
        if "theme" in path.parts:
            continue
        text = path.read_text(encoding="utf-8")
        for match in _OBJECTNAME_PATTERN.finditer(text):
            name = match.group(1)
            if name.startswith("serialStation"):
                names.add(name)
    return names


def _is_style_exempt(name: str) -> bool:
    return any(name.endswith(suffix) for suffix in STYLE_EXEMPT_SUFFIXES)


def test_ui_source_contains_serial_station_objectnames():
    """守卫：扫描必须命中非空集合，否则扫描逻辑失效。"""

    names = _extract_serial_station_objectnames()
    assert names, "expected serialStation* objectNames in ui/ source"


def test_build_qss_covers_all_styled_serial_station_objectnames():
    """每个非豁免的 serialStation* objectName 必须在 QSS 中有对应选择器。"""

    qss = build_qss()
    names = _extract_serial_station_objectnames()
    missing = sorted(n for n in names if not _is_style_exempt(n) and f"#{n}" not in qss)
    assert not missing, (
        f"QSS does not cover {len(missing)} serialStation* objectName(s): {missing}"
    )


def test_build_qss_documents_exempt_objectnames():
    """豁免 objectName（如 Shortcut）必须在 QSS 中有契约占位，保证可发现性。"""

    qss = build_qss()
    names = _extract_serial_station_objectnames()
    exempt = sorted(n for n in names if _is_style_exempt(n))
    for name in exempt:
        assert f"#{name}" in qss, (
            f"exempt objectName {name} must still have a contract placeholder in QSS"
        )
