# PRD-199 Python Governance 测试行为域拆分

## 目标

把默认 Python/PyQt 切换测试中的治理文档审计拆到独立文件，避免 `test_python_default_cutover.py` 继续承担脚本入口、native 清理、架构文档和治理文档多种职责。

## 范围

- 新增 `test_python_governance_docs.py`。
- 将活跃架构文档、PRD/Specs、治理文档和已删除脚本文案审计迁移到独立测试文件。
- 将测试文件体积门禁从 300 行收紧到 250 行。

## 非目标

- 不改变产品运行时代码。
- 不改变 uv 脚本、bat 入口或 PyInstaller 打包链路。
- 不放宽旧原生构建清理门禁。

## 架构边界

- 测试仍位于 `tests/python/unit/`。
- 该拆分只调整测试组织，不改变被审计文档的语义。
- 新增门禁继续保护 Python/PyQt-only 主线。

## 验收标准

- `test_python_default_cutover.py` 低于 250 行。
- `test_python_governance_docs.py` 存在并承载治理文档审计。
- `uv run test-embeddebug-py` 和启动 smoke 继续通过。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，测试治理门禁可复现 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
