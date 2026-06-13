# PRD-109 - Tools Test Direct Entry

## 背景

`uv run test-embeddebug-tools` 可以运行 Python 工具自测，但 `python tools/test_embeddebug_tools.py` 直接运行时失败：

```text
ModuleNotFoundError: No module named 'tools'
```

原因是脚本以文件路径直接执行时，Python 将 `tools/` 目录放入 `sys.path`，而仓库根目录不在导入路径中，导致 `from tools import ...` 失败。PRD-107 Specs 中已经把直接 Python 命令列为可用验证入口，因此需要修复该入口。

## 目标

1. 让 `python tools/test_embeddebug_tools.py` 可直接从仓库根目录运行。
2. 保持 `uv run test-embeddebug-tools` 入口继续可用。
3. 不改变被测工具行为。

## 非目标

1. 不修改生产 C++。
2. 不修改构建系统。
3. 不新增 Python 依赖。
4. 不改变启动、打包、校验工具业务逻辑。

## 验收标准

1. `python tools/test_embeddebug_tools.py` 通过。
2. `uv run test-embeddebug-tools` 通过。
3. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以双入口工具测试通过为证据。
- 用户状态：不提升，产品行为无变化。
- 设备状态：不提升，真实硬件未验证。
