# Batch 54 — tr() + objectName 合规审计（对标铁律 16/19）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 54/∞
> 来源：CLAUDE.md §七 待完成项目 #13/#14（tr() 合规审计 + objectName 审计）
> 前置：Batch 53 已提交（19a06b1a4），死代码治理完成
> 评分目标：720 → 723+

---

## 1. 目标

修复 iron rule 16（所有 QWidget 必须有 objectName）+ iron rule 19（用户可见文字走 tr()）
违规，并加守护测试防止回潮。

### 子任务

| 子任务 | 内容 | 文件 |
|---|---|---|
| **B54-1** | 修复 ~30 个 objectName 违规 | ui/tools/{byte_frequency,crc_calculator,hex_viewer,timestamp_converter}.py |
| **B54-2** | 修复 1 个 tr() 违规 | ble_panel.py:78 "handle" 加 widget.tr() |
| **B54-3** | 新建 test_objectname_coverage.py 守护测试 | tests/python/unit/（clone test_no_hardcoded_colors 模式）|
| **B54-4** | 新建 test_tr_compliance.py 守护测试 | tests/python/unit/（AST + STRICT_TR env flag）|
| **B54-5** | 补 QSS（新 objectName）| qss_sections_tools.py 或现有段 |

### 约束
- 不改用户工作区已改动文件（animations/*、connection_toolbar、main_window、panels/* 等）
- 新 objectName 命名：serialStation + 模块域 + 角色（如 serialStationCrcInputModeLabel）
- 守护测试软模式默认（print 违规），STRICT_* env flag 硬模式

---

> **下一步**：用户持续授权 → 直接执行
