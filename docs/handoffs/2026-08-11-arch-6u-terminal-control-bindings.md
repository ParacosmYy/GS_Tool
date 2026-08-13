# ARCH-6u：终端/发送控件组合绑定

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

新增 `presentation/terminal_bindings.py` 的 `TerminalControlBindings` 与
`terminal_bindings_for()`。`bootstrap.py` 是组合根组装 owner；终端 runtime、连接 gate、命令
发送/选择、发送上下文、生命周期、Tab 顺序和专注模式不再直接读取终端/发送/历史 Qt 动态字段。
bundle 只持有 Qt 引用，业务状态、preview buffer、history/quick snapshot、timer、ViewModel、
MotionController 与 callbacks 留在原 owner。

## 行为保持

保留实时观测、暂停显示、原始记录、Hex/文本发送、CRLF、Ctrl+Enter、快捷命令、发送历史、
批量命令 Tab 顺序、发送 gate、空态、主题、动效和关闭生命周期；缺失 bundle 时 projection 安全
返回，发送配置错误显式提示。

## 验证证据

- `TERMINAL_BINDINGS_COMPILE_PASS`
- `TERMINAL_BINDINGS_COMMAND_SLICE_PASS`
- `TERMINAL_BINDINGS_RUNTIME_SLICE_PASS`
- `TERMINAL_BINDINGS_LIFECYCLE_SLICE_PASS`
- `TERMINAL_BINDINGS_WIDGET_OWNER_ONLY_PASS`
- `TERMINAL_DYNAMIC_WIDGET_OWNER_ONLY_PASS`
- `ARCH6U_TERMINAL_VECTOR_PASS 24 themes=3 terminal=1 send=1 hmax=0 exact_white=0 near_white=0`
- `PACKAGE_ARCH6U_FINAL`: onefile/provenance verified; canonical/root/root-latest all `47,969,870` bytes
  with SHA-256 `485AFE4B7A581E27154BFA1043290D872FDB862CA61F35219945D88FA72839A8`
- archive listing SHA-256: `E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`
- `ROOT_EXE_OVERWRITE_PASS`: canonical, `SerialForge.exe` and `SerialForge-latest.exe` byte-identical
- PySide6 仍报告既有 fonts 目录 warning，但系统字体离屏渲染正常；GUI/EXE 启动、读屏、真实设备、
  硬件、刷写和部署验收未运行。

## 角色与简化审查

架构师线程 `019ff129-d626-78b0-9852-fe743f2b7d99` 已调用但超时关闭，未计为独立通过；父代理
完成五轴架构与简化审查。复用 UART/网络/BLE typed accessor 模式，不新增业务状态、timer、
backend 或测试资产。

嵌入式 C/C++/固件适用性：N/A；无 public vendor source applicability；无硬件授权操作。
