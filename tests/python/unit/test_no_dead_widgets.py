"""B50-1 死代码守护：控件库 __all__ 符号必须有 production 引用。

防止「定义 + 导出但从未接入生产」的死模块累积（Batch 40-48 历史问题）。
Batch 50 新增机械检测：枚举 ``controls/__init__.py.__all__``，断言每个符号
（除白名单外）在 ``python/embeddebug/`` 下有 ≥1 个非测试、非 ``__init__``、
非 QSS 段（``qss_sections_*``）的引用。

白名单（``_PENDING_WIRE``）列出已知的、尚未接入生产的 widget，每个附理由。
未来 batch wire 后从此移除；若长期不 wire 则应删除模块本身。

约束：纯标准库（pathlib/re），不依赖 PyQt，不在模块顶层 import embeddebug。
"""

from __future__ import annotations

import ast
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
UI_ROOT = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"

# 已知未 wire 的 widget（定义 + 导出但无 production 构造）。
# 每项附理由；wire 后从此移除。长期不 wire 的应考虑删除模块。
_PENDING_WIRE: dict[str, str] = {
    "Badge": "Batch 41 新增，未接入任何面板（待状态徽章场景）",
    "BadgeKind": "Badge 的枚举伴生，随 Badge 一起 wire",
    "BannerKind": "InfoBanner 的枚举伴生，随 InfoBanner 一起 wire",
    "Chip": "Batch 40 新增，未接入（待标签/筛选场景）",
    "Divider": "Batch 48 新增分隔线，未接入面板布局",
    "Drawer": "Batch 41 侧边抽屉，未接入（待设置/详情侧栏场景）",
    "InfoBanner": "Batch 40 通知横幅，未接入（toast 已覆盖通知场景）",
    "SegmentedControl": "Batch 40 分段控件，未接入（待协议/视图切换场景）",
    "ToggleSwitch": "Batch 48 滑动开关，未接入（待设置项场景）",
    # install_tooltip/uninstall_tooltip：install 已 wire（connection_toolbar 等），
    # uninstall 仅由 destroyed 信号内部触发，无显式 production 调用，属设计内。
    "uninstall_tooltip": "仅由 widget destroyed 信号内部触发，无显式调用",
    # RichTooltip 类本身不直接构造，仅通过 install_tooltip 间接实例化
    # （install_tooltip 把 RichTooltip 实例 monkey-patch 到目标 widget）。
    "RichTooltip": "类不直接构造，通过 install_tooltip 间接实例化（设计内）",
}

# 扫描时排除的路径片段（这些引用不算 production wiring）。
_NON_PRODUCTION_MARKERS = (
    "__pycache__",
    "__init__.py",
    "qss_sections_",  # QSS objectName 契约占位，非 wiring
)


def _extract_controls_all_symbols() -> list[str]:
    """从 controls/__init__.py 解析 __all__ 列表。"""

    init_path = UI_ROOT / "controls" / "__init__.py"
    text = init_path.read_text(encoding="utf-8")
    match = re.search(r"__all__\s*=\s*\[(.*?)\]", text, re.DOTALL)
    if match is None:
        return []
    body = match.group(1)
    return re.findall(r'"([A-Za-z_][A-Za-z0-9_]*)"', body)


def _symbol_definition_stems(symbol: str) -> set[str]:
    """推断 symbol 可能的定义文件 stem（用于排除自身定义文件）。

    Badge/BadgeKind → badge；InfoBanner → info_banner；
    install_tooltip/uninstall_tooltip → rich_tooltip（无法从名推断，调用方补充）。
    """

    stems = set()
    # 去 Kind/State 后缀。
    base = symbol
    for suffix in ("Kind", "State"):
        if base.endswith(suffix):
            base = base[: -len(suffix)]
    # CamelCase → snake_case。
    snake = re.sub(r"(?<!^)(?=[A-Z])", "_", base).lower()
    stems.add(snake)
    stems.add(base.lower())
    return stems


def _code_tokens(path: Path) -> set[str]:
    """提取文件中的纯代码 identifier（剥离注释/字符串/docstring）。

    防止 docstring 里的提及（如 badge.py 注释提到 InfoBanner）被误判为引用。
    用 AST 遍历所有 Name/Attribute/keyword 节点。
    """

    text = path.read_text(encoding="utf-8")
    try:
        tree = ast.parse(text)
    except SyntaxError:
        # 解析失败退化为正则（不应发生在生产代码）。
        return set(re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", text))
    tokens: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Name):
            tokens.add(node.id)
        elif isinstance(node, ast.Attribute):
            tokens.add(node.attr)
    return tokens


def _symbol_source_stems() -> dict[str, str]:
    """从 controls/__init__.py 的 import 语句解析每个 symbol 的定义文件 stem。

    返回 {symbol: source_stem}，如 {"Badge": "badge", "BannerKind": "info_banner"}。
    精确排除定义文件，避免逐个硬编码。
    """

    init_path = UI_ROOT / "controls" / "__init__.py"
    text = init_path.read_text(encoding="utf-8")
    tree = ast.parse(text)
    mapping: dict[str, str] = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom):
            # from .controls.badge import Badge → module="controls.badge"
            source = (node.module or "").split(".")[-1]
            for alias in node.names:
                mapping[alias.asname or alias.name] = source
    return mapping


def _count_production_references(symbol: str, source_stems: dict[str, str]) -> list[Path]:
    """统计 symbol 在 production 源码（非测试/非 init/非 QSS/非注释）的引用文件数。"""

    def_stems: set[str] = set()
    # 从 import 解析精确源文件 stem。
    if symbol in source_stems:
        def_stems.add(source_stems[symbol])
    # 兜底：CamelCase→snake_case + 去枚举后缀。
    def_stems |= _symbol_definition_stems(symbol)
    if symbol in ("install_tooltip", "uninstall_tooltip", "install_hover_lift"):
        def_stems.add("rich_tooltip")
        def_stems.add("micro_interactions")
    references: list[Path] = []
    for path in UI_ROOT.rglob("*.py"):
        rel = str(path.relative_to(UI_ROOT)).replace("\\", "/")
        if any(marker in rel for marker in _NON_PRODUCTION_MARKERS):
            continue
        if path.stem in def_stems:
            continue
        if symbol in _code_tokens(path):
            references.append(path)
    return references


def test_controls_all_symbols_have_production_references():
    """controls/__all__ 每个符号（白名单外）必须有 ≥1 production 引用。

    白名单 ``_PENDING_WIRE`` 列出已知未 wire 项；wire 后应移除。
    失败时报告每个死符号 + 0 引用，便于定位。
    """

    symbols = _extract_controls_all_symbols()
    assert symbols, "controls/__init__.py __all__ should not be empty"
    source_stems = _symbol_source_stems()

    dead: list[str] = []
    for symbol in symbols:
        if symbol in _PENDING_WIRE:
            continue
        refs = _count_production_references(symbol, source_stems)
        if not refs:
            dead.append(symbol)

    if dead:
        details = "\n".join(f"  - {s}: 0 production references" for s in dead)
        raise AssertionError(
            f"{len(dead)} control symbol(s) have zero production references "
            f"(add to _PENDING_WIRE with reason, or wire into production):\n{details}"
        )


def test_pending_wire_whitelist_matches_actually_dead():
    """白名单精确性：_PENDING_WIRE 中的符号必须确实 0 引用（防止白名单过时）。

    若某白名单符号已被 wire（有 ≥1 引用），应从白名单移除（否则白名单变成死代码）。
    """

    source_stems = _symbol_source_stems()
    stale = []
    for symbol in _PENDING_WIRE:
        if symbol not in _extract_controls_all_symbols():
            stale.append(f"{symbol}: not in __all__ anymore (remove from whitelist)")
            continue
        refs = _count_production_references(symbol, source_stems)
        if refs:
            stale.append(
                f"{symbol}: now has {len(refs)} reference(s) "
                f"(e.g. {refs[0].name}) — remove from _PENDING_WIRE"
            )
    assert not stale, "stale whitelist entries (wire completed, please clean up):\n" + "\n".join(stale)


def test_whitelist_entries_have_documented_reasons():
    """白名单每项必须有非空理由（防止「先加再说」累积无文档豁免）。"""

    undocumented = [s for s, reason in _PENDING_WIRE.items() if not reason or not reason.strip()]
    assert not undocumented, f"whitelist entries missing reason: {undocumented}"
