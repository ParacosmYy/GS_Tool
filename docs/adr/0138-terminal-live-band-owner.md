# ADR-0138：终端 live band 的语义行与独立 builder

- 状态：Accepted for ARCH-87 / UI-1.160
- 日期：2026-08-12
- 范围：`presentation/controllers/terminal.py`、`terminal_toolbar_builder.py`、
  `send_bar_builder.py`、`bootstrap.py`

## 背景

实时观测和发送 band 过去各自使用固定多列网格，把标题、选择器、输入、状态和动作压在同一组
水平列中。窄窗口下输入区域被动让位，按钮与状态 rail 的视觉层级不清晰；同时 `terminal.py` 同时
承担错误、live controls、历史和批量命令组合，扩展成本变高。

## 决策

1. `terminal_toolbar_builder.py` 单独拥有实时观测 band，按控制行/活动行组合显示模式、暂停、清空、
   原始记录和数据活动。
2. `send_bar_builder.py` 单独拥有发送 band，第一行优先保护格式、输入和发送动作；第二行承载发送
   状态、摘要、CRLF、快捷命令和保存快捷。
3. `terminal.py` 只保留错误 notice、发送历史和批量命令；`bootstrap.py` 直接导入两个 live builders，
   组合根仍在同一处创建 `TerminalControlBindings`。
4. 不新增 binding 字段、业务状态、timer、scroll owner、线程或 transport backend；所有既有信号和
   window staged refs 原样保留。

## 不变量与风险

- `TerminalControlBindings`、`terminal_bindings_for(window)`、所有 `itemData`、回调、dynamic
  `source/state`、focus/accessibility 和 close/motion lifecycle 不变。
- 发送回车仍显式发送；快捷命令只填入；暂停显示不停止接收/记录；批量命令仍显式执行。
- 本轮只做静态/编译校验，真实窗口几何和主题截图未运行；最小宽度仅作静态预算，不能代替 Qt
  layout geometry evidence。

## 复核与验证

`ruff`、`compileall`、`scripts/check.ps1`、presentation import contract 通过，源文件 174 个均不
超过 1000 行，theme token audit 通过。ARCH-87 架构师及独立 reviewer 调用均超时并关闭，未伪造
独立 PASS；父代理完成 architecture、correctness、readability/simplicity、security、performance
fresh-pass，Required=0。Python/PySide6 presentation-only，embedded C/C++ public vendor source
applicability 为 N/A。
