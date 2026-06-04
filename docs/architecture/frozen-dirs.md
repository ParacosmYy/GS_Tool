# 冻结目录

> 这里列出允许保留、但不再继续扩张的历史分叉目录。

## 冻结清单

- `src/core/animation2/`
- `src/core/widgets2/`
- `src/plugin/loader2/`
- `src/core/font/`
- `src/core/fonts/`
- `src/core/icon/`
- `src/core/icons/`
- `src/core/responsive/`

## 冻结原则

- 可以保留旧 include 和兼容转发。
- 可以修复编译或链接问题。
- 不允许再往冻结目录新增功能实现。
- 如果要继续扩张能力，必须进入 canonical 目录。
