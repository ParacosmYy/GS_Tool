"""B50-2 死代码守护：动画模块 + 动画 token 必须有 production 引用。

两部分（DC-1 + DC-2，Batch 43 spec 预分配给 Batch 50）：
1. 动画模块 wiring：``animations/__init__.py.__all__`` 每个符号（白名单外）
   在 production 有 ≥1 引用。
2. 动画 token 覆盖：``AnimationTokens`` 的 DURATION_* / EASE_* 常量每个被
   ≥1 个 factory 或 widget 引用（防止孤儿 token）。

白名单 ``_PENDING_WIRE`` 列出已知未 wire 项（如 elevation_effect 待 B50-3 wire）。

约束：纯标准库（pathlib/re/ast），不依赖 PyQt。
"""

from __future__ import annotations

import ast
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
UI_ROOT = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"
ANIM_INIT = UI_ROOT / "animations" / "__init__.py"
TOKENS_PATH = UI_ROOT / "animations" / "tokens.py"

# 已知未 wire 的动画符号（wire 后移除）。
# Batch 50-3 已 wire elevation_effect 到 micro_interactions.install_card_shadow
# → layout_cards.build_card，白名单现为空。
_PENDING_WIRE: dict[str, str] = {}

# Batch 51-53 已消费 DURATION_CONTAINER/EASE_OUT_QUART/DURATION_FLYOUT/EASE_OUT_QUINT/
# DURATION_DRAWER/DURATION_PROGRESS/EASE_IN_QUART/EASE_OUT_QUAD。
# 剩 DURATION_SCROLL 待「滚动到视图」场景实现（如列表项点击定位）。
_RESERVED_TOKENS: dict[str, str] = {
    "DURATION_SCROLL": "滚动到视图动画预留（500ms，待列表项点击定位场景实现）",
}

_NON_PRODUCTION_MARKERS = (
    "__pycache__",
    "__init__.py",
    "qss_sections_",
)


def _extract_animations_all_symbols() -> list[str]:
    """从 animations/__init__.py 解析 __all__。"""

    text = ANIM_INIT.read_text(encoding="utf-8")
    match = re.search(r"__all__\s*=\s*\[(.*?)\]", text, re.DOTALL)
    if match is None:
        return []
    return re.findall(r'"([A-Za-z_][A-Za-z0-9_]*)"', match.group(1))


def _count_production_references(symbol: str) -> list[Path]:
    """统计 symbol 在 production 源码的引用文件（排除测试/init/QSS/自身定义）。"""

    pattern = re.compile(r"\b" + re.escape(symbol) + r"\b")
    refs: list[Path] = []
    for path in UI_ROOT.rglob("*.py"):
        rel = str(path.relative_to(UI_ROOT)).replace("\\", "/")
        if any(marker in rel for marker in _NON_PRODUCTION_MARKERS):
            continue
        # 跳过 animations/ 自身（定义模块，不算引用）。
        if "animations/" in rel:
            continue
        text = path.read_text(encoding="utf-8")
        if pattern.search(text):
            refs.append(path)
    return refs


def _extract_animation_token_names() -> list[str]:
    """用 AST 从 tokens.py 提取 AnimationTokens 类的 DURATION_*/EASE_* 属性名。"""

    tree = ast.parse(TOKENS_PATH.read_text(encoding="utf-8"))
    tokens: list[str] = []
    for node in ast.walk(tree):
        if isinstance(node, ast.ClassDef) and node.name == "AnimationTokens":
            for item in node.body:
                if isinstance(item, ast.Assign):
                    for target in item.targets:
                        if isinstance(target, ast.Name) and (
                            target.id.startswith("DURATION_")
                            or target.id.startswith("EASE_")
                        ):
                            tokens.append(target.id)
    return tokens


def test_animation_modules_have_production_references():
    """animations/__all__ 每个符号（白名单外）必须有 ≥1 production 引用。"""

    symbols = _extract_animations_all_symbols()
    assert symbols, "animations/__init__.py __all__ should not be empty"

    dead: list[str] = []
    for symbol in symbols:
        if symbol in _PENDING_WIRE:
            continue
        refs = _count_production_references(symbol)
        if not refs:
            dead.append(symbol)

    if dead:
        details = "\n".join(f"  - {s}: 0 production references" for s in dead)
        raise AssertionError(
            f"{len(dead)} animation symbol(s) have zero production references:\n{details}"
        )


def test_animation_tokens_have_factory_references():
    """每个 DURATION_*/EASE_* token 必须被 ≥1 production 文件代码引用。

    防止孤儿 token（定义但无 factory 消费）。用 AST 提取代码 token
    （剥离注释/字符串），避免 docstring 里的提及误判。
    引用形式：``AnimationTokens.DURATION_X`` 或局部别名。
    """

    token_names = _extract_animation_token_names()
    assert token_names, "AnimationTokens should define DURATION_*/EASE_* constants"

    orphan_tokens: list[str] = []
    for token in token_names:
        if token in _RESERVED_TOKENS:
            continue  # 有意预留，待场景 wire 后移除
        found = False
        for path in UI_ROOT.rglob("*.py"):
            rel = str(path.relative_to(UI_ROOT)).replace("\\", "/")
            if any(marker in rel for marker in _NON_PRODUCTION_MARKERS):
                continue
            if rel == "animations/tokens.py":
                continue  # 定义文件自身不算引用
            # 用 AST 提取代码 token（排除注释/docstring）。
            try:
                tree = ast.parse(path.read_text(encoding="utf-8"))
            except SyntaxError:
                continue
            code_tokens = {
                n.id for n in ast.walk(tree) if isinstance(n, ast.Name)
            } | {
                n.attr for n in ast.walk(tree) if isinstance(n, ast.Attribute)
            }
            if token in code_tokens:
                found = True
                break
        if not found:
            orphan_tokens.append(token)

    if orphan_tokens:
        details = "\n".join(f"  - {t}: 0 references" for t in orphan_tokens)
        raise AssertionError(
            f"{len(orphan_tokens)} animation token(s) have zero references "
            f"(remove or wire into a factory):\n{details}"
        )


def test_pending_wire_whitelist_matches_actually_dead():
    """白名单精确性：_PENDING_WIRE 中的符号必须确实 0 引用。"""

    stale = []
    for symbol in _PENDING_WIRE:
        if symbol not in _extract_animations_all_symbols():
            stale.append(f"{symbol}: not in __all__ anymore (remove from whitelist)")
            continue
        refs = _count_production_references(symbol)
        if refs:
            stale.append(
                f"{symbol}: now has {len(refs)} reference(s) — remove from _PENDING_WIRE"
            )
    assert not stale, "stale whitelist entries:\n" + "\n".join(stale)


def test_reserved_tokens_whitelist_is_accurate():
    """预留 token 白名单精确性：已被引用的应移除（防止白名单变死代码）。"""

    token_names = _extract_animation_token_names()
    stale = []
    for token in _RESERVED_TOKENS:
        if token not in token_names:
            stale.append(f"{token}: not defined in AnimationTokens anymore")
            continue
        # 检查是否有代码引用（AST）。
        found = False
        for path in UI_ROOT.rglob("*.py"):
            rel = str(path.relative_to(UI_ROOT)).replace("\\", "/")
            if any(marker in rel for marker in _NON_PRODUCTION_MARKERS):
                continue
            if rel == "animations/tokens.py":
                continue
            try:
                tree = ast.parse(path.read_text(encoding="utf-8"))
            except SyntaxError:
                continue
            code_tokens = {
                n.id for n in ast.walk(tree) if isinstance(n, ast.Name)
            } | {
                n.attr for n in ast.walk(tree) if isinstance(n, ast.Attribute)
            }
            if token in code_tokens:
                found = True
                break
        if found:
            stale.append(f"{token}: now referenced — remove from _RESERVED_TOKENS")
    assert not stale, "stale reserved-token whitelist:\n" + "\n".join(stale)


def test_reserved_tokens_have_documented_reasons():
    """预留 token 每项必须有非空理由。"""

    undocumented = [t for t, reason in _RESERVED_TOKENS.items() if not reason.strip()]
    assert not undocumented, f"reserved tokens missing reason: {undocumented}"
