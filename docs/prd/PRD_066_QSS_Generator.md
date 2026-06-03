# PRD-066: QSS主题生成器 — JSON颜色定义自动生成三套主题

## 背景
当前项目维护3套QSS主题文件(light.qss/dark.qss/onedark.qss)，每套约1700行，总计约5100行。三套文件结构相同，仅颜色值不同，导致~3300行重复代码。修改样式时需同步修改3个文件，极易遗漏。通过JSON定义颜色变量 + Mustache模板，用Python脚本自动生成3套QSS，消除手动维护的重复和遗漏风险。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | theme_definitions.json: 3套主题颜色定义(~100行) | P0 | tools/themes/ |
| R2 | qss_template.mustache: 含{{color}}占位符的QSS模板(~1700行) | P0 | tools/themes/ |
| R3 | generate_themes.py: 读取JSON+模板 → 生成3个QSS文件 | P0 | tools/themes/ |
| R4 | 验证: 生成的QSS与现有文件diff一致 | P0 | tools/themes/ |
| R5 | CI集成: build前自动运行generate_themes.py | P2 | CMakeLists.txt |

## 接口设计

### theme_definitions.json结构
```json
{
  "themes": {
    "light": {
      "name": "EmbedDebug Light",
      "BgPrimary": "#FFFFFF",
      "BgSecondary": "#F5F5F5",
      "BgTertiary": "#EAEAEA",
      "BgHover": "#E0E0E0",
      "TextPrimary": "#1A1A1A",
      "TextSecondary": "#666666",
      "TextMuted": "#999999",
      "Accent": "#2563EB",
      "AccentHover": "#1D4ED8",
      "AccentPressed": "#1E40AF",
      "Border": "#E0E0E0",
      "BorderFocus": "#2563EB",
      "Success": "#16A34A",
      "Warning": "#D97706",
      "Error": "#DC2626",
      "Scrollbar": "#C0C0C0",
      "ScrollbarHover": "#A0A0A0"
    },
    "dark": { "..." : "..." },
    "onedark": { "..." : "..." }
  }
}
```

### qss_template.mustache(片段)
```css
/* === EmbedDebug {{theme_name}} Theme === */
QWidget {
    background-color: {{BgPrimary}};
    color: {{TextPrimary}};
    font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
    font-size: 13px;
}

QLineEdit {
    background-color: {{BgTertiary}};
    border: 1px solid {{Border}};
    border-radius: 6px;
    padding: 6px 10px;
    color: {{TextPrimary}};
}

QLineEdit:focus {
    border-color: {{BorderFocus}};
}

QPushButton {
    background-color: {{Accent}};
    color: #FFFFFF;
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
}

QPushButton:hover {
    background-color: {{AccentHover}};
}

QPushButton:pressed {
    background-color: {{AccentPressed}};
}
/* ... 后续约1700行QSS规则 ... */
```

### generate_themes.py
```python
#!/usr/bin/env python3
"""QSS主题生成器 -- 从JSON颜色定义+Mustache模板生成3套QSS"""
import json, sys
from pathlib import Path
try:
    import chevron  # Mustache渲染库
except ImportError:
    print("pip install chevron", file=sys.stderr)
    sys.exit(1)

def main():
    base = Path(__file__).parent
    output_dir = base / ".." / ".." / "resources" / "themes"

    with open(base / "theme_definitions.json") as f:
        defs = json.load(f)

    with open(base / "qss_template.mustache") as f:
        template = f.read()

    for theme_key, colors in defs["themes"].items():
        qss = chevron.render(template, colors)
        out_path = output_dir / f"{theme_key}.qss"
        out_path.write_text(qss, encoding="utf-8")
        print(f"Generated: {out_path} ({len(qss)} bytes)")

if __name__ == "__main__":
    main()
```

## 依赖的公共组件
- chevron (Python Mustache库) — 模板渲染
- 无C++依赖，纯工具链改进

## 设计模式
- **模板方法模式**: JSON定义数据变化，Mustache模板定义结构不变部分
- **代码生成**: 从单一数据源+模板生成多个输出文件
- **验证模式**: diff比对确保生成结果与手写版本一致

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| tools/themes/theme_definitions.json | 新增 | 无 |
| tools/themes/qss_template.mustache | 新增 | 无 |
| tools/themes/generate_themes.py | 新增 | 无 |
| resources/themes/light.qss | 替换(生成) | 中(需diff验证) |
| resources/themes/dark.qss | 替换(生成) | 中(需diff验证) |
| resources/themes/onedark.qss | 替换(生成) | 中(需diff验证) |
| CMakeLists.txt | 修改(可选:自定义命令) | 低 |

## 验收标准
1. `python tools/themes/generate_themes.py` 成功生成3个QSS文件
2. 生成的light.qss与原文件diff一致(先从原文件提取模板)
3. JSON中包含~100行颜色定义，覆盖ThemeManager::SemanticColor所有枚举值
4. qss_template.mustache包含所有QSS选择器规则(约1700行)
5. 修改JSON中一个颜色值，重新生成后对应QSS文件仅该颜色变化
6. EmbedDebug使用生成的QSS文件启动正常，视觉效果与原文件一致
7. 编译零错误
