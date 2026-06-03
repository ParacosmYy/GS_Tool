#!/usr/bin/env python3
"""QSS Theme Generator — 从模板 + JSON 颜色定义生成 QSS 主题文件。

用法:
    python generate.py --all                  # 生成所有主题
    python generate.py dark_terminal          # 生成单个主题
    python generate.py modern_dark light      # 生成多个主题
"""

import json
import re
import sys
from pathlib import Path

# ── 路径配置 ──────────────────────────────────────────────────────────
SCRIPT_DIR = Path(__file__).resolve().parent
THEMES_DIR = SCRIPT_DIR / "themes"
TEMPLATE_FILE = SCRIPT_DIR / "template.qss"
LAYOUT_FILE = SCRIPT_DIR / "layout.json"
OUTPUT_DIR = SCRIPT_DIR.parent.parent / "resources" / "themes"

# ── 语义色板键列表 (ThemeManager 从注释块中解析这些键) ──────────────────
SEMANTIC_KEYS = [
    "BgPrimary", "BgSecondary", "BgTertiary", "BgHover",
    "TextPrimary", "TextSecondary", "TextMuted",
    "Accent", "AccentHover", "AccentPressed",
    "Border", "BorderFocus",
    "Success", "Warning", "Error",
    "Scrollbar", "ScrollbarHover", "Shadow",
    "TermBackground", "TermRxText", "TermTxText",
    "TermTimestamp", "TermSelection",
    "TermSearchHighlight", "TermCurrentMatch",
]

# ── 语义色板注释块模板 ────────────────────────────────────────────────
SEMANTIC_BLOCK_TEMPLATE = """\
/*
 * 语义色板定义 — ThemeManager 从此块解析自绘控件颜色
 * 格式: --semantic-<枚举名>: <颜色值>;
 * TerminalWidget/ChartWidget 等自绘控件通过 ThemeManager::color() 查询
 *
 * 注意: 这些自定义属性声明必须在CSS注释内!
 * Qt的QSS引擎不支持CSS自定义属性(--)语法,
 * 裸露的声明会导致QSS解析器出错，后续样式规则全部失效。
 * ThemeManager使用正则表达式从此注释块中提取颜色定义。
{semantic_lines} */"""


def load_json(path: Path) -> dict:
    """加载 JSON 文件并返回字典。"""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def build_semantic_block(colors: dict) -> str:
    """根据颜色字典生成语义色板注释块。"""
    lines = []
    for key in SEMANTIC_KEYS:
        value = colors.get(key, "")
        lines.append(f"--semantic-{key}: {value};")
    semantic_lines = "\n".join(lines)
    return SEMANTIC_BLOCK_TEMPLATE.format(semantic_lines=semantic_lines)


def generate_theme(theme_name: str, template: str, layout: dict) -> bool:
    """生成单个主题的 QSS 文件。

    Args:
        theme_name: 主题名称 (对应 themes/<name>.json)
        template: 模板内容字符串
        layout: layout.json 字典

    Returns:
        True 生成成功, False 失败
    """
    theme_file = THEMES_DIR / f"{theme_name}.json"
    if not theme_file.exists():
        print(f"  [ERR] Theme not found: {theme_file}")
        return False

    theme_data = load_json(theme_file)
    colors = theme_data["colors"]
    description = theme_data.get("description", theme_name)

    # 1) 替换语义色板块
    semantic_block = build_semantic_block(colors)
    result = template.replace("{{SEMANTIC_BLOCK}}", semantic_block)

    # 2) 替换颜色占位符 {{color.Xxx}}
    def replace_color(match: re.Match) -> str:
        key = match.group(1)
        if key not in colors:
            print(f"  [WARN] Missing color key: {key} (theme: {theme_name})")
            return match.group(0)
        return colors[key]

    result = re.sub(r"\{\{color\.(\w+)\}\}", replace_color, result)

    # 3) 替换布局占位符 {{layout.Xxx}}
    def replace_layout(match: re.Match) -> str:
        key = match.group(1)
        if key not in layout:
            print(f"  [WARN] Missing layout key: {key}")
            return match.group(0)
        return layout[key]

    result = re.sub(r"\{\{layout\.(\w+)\}\}", replace_layout, result)

    # 4) 替换描述占位符
    result = result.replace("{{description}}", description)

    # 5) 写入输出文件
    output_file = OUTPUT_DIR / f"{theme_name}.qss"
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(result)

    line_count = result.count("\n") + 1
    rel = output_file.relative_to(SCRIPT_DIR.parent.parent)
    print(f"  [OK] {theme_name} -> {rel} ({line_count} lines)")
    return True


def get_available_themes() -> list[str]:
    """返回 themes/ 目录中可用的主题名称列表。"""
    return sorted(p.stem for p in THEMES_DIR.glob("*.json"))


def main():
    # 加载模板和布局
    if not TEMPLATE_FILE.exists():
        print(f"Error: Template not found: {TEMPLATE_FILE}")
        sys.exit(1)

    template = TEMPLATE_FILE.read_text(encoding="utf-8")
    layout = load_json(LAYOUT_FILE) if LAYOUT_FILE.exists() else {}

    # 确定要生成的主题
    available = get_available_themes()
    if not available:
        print("Error: No theme definitions in themes/ directory")
        sys.exit(1)

    if "--all" in sys.argv:
        themes = available
    elif len(sys.argv) > 1:
        themes = [a for a in sys.argv[1:] if not a.startswith("-")]
    else:
        print("Usage: python generate.py --all | <theme_name> [theme_name ...]")
        print(f"Available themes: {', '.join(available)}")
        sys.exit(1)

    print(f"QSS Theme Generator - {len(themes)} theme(s)")
    print(f"  Template: {TEMPLATE_FILE.name}")
    print(f"  Output:   {OUTPUT_DIR}\n")

    success = 0
    for name in themes:
        if name not in available:
            print(f"  [ERR] Unknown theme: {name} (available: {', '.join(available)})")
            continue
        if generate_theme(name, template, layout):
            success += 1

    print(f"\nDone: {success}/{len(themes)} themes generated")
    if success < len(themes):
        sys.exit(1)


if __name__ == "__main__":
    main()
