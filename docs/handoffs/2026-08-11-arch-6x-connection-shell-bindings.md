# ARCH-6x / UI-1.140 链路控制外壳绑定边界

日期：2026-08-11  
状态：源码、静态检查、真实组合根 vector、onefile/root 覆盖完成

## 变更

新增 `ConnectionShellBindings`（`frozen=True, slots=True`），由
`presentation/controllers/connection_builder.py` 在创建连接外壳后、刷新 preset
combo 前唯一组装。bundle 包含 control band、transport mode surface、传输/快速配置
combo、preset context、保存/删除按钮、connection status rail 和 connect button。

以下消费者已改为 typed accessor：

- `connection.py`、`connection_runtime.py`、`connection_presets.py`；
- `commands.py`、`composition.py`、`derived_data.py`；
- `protocol_config.py`、`replay.py`、`protocol_scope.py`、`lifecycle.py`。

preset catalog/store、session/ViewModel、连接/协议策略、callbacks、timer、transport
handle 和 peer/device snapshot 未进入 bundle。构建器动态字段仅保留组装阶段使用。

## 已验证

```text
CONNECTION_SHELL_STATIC_PASS
scripts/check.ps1: pass
compileall: pass
ruff: pass
source line limit: 164 files <= 1000
theme token audit: 3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0
CONNECTION_SHELL_RUNTIME_PASS ConnectionShellBindings
CONNECTION_SHELL_LAYOUT_PASS 980 720
CONNECTION_SHELL_LAYOUT_PASS 1240 820
MOTION_TARGET_PASS 120 [8, 9]
THEME_CONNECTION_SHELL_PASS 0/1/2
```

## assurance gate

- public source applicability：N/A；本轮只修改 Python/Qt presentation，不涉及 MCU、
  embedded C/C++、vendor SDK、RTOS、ISR/DMA、OTA firmware 或硬件。
- independent review：已请求 Luna/max/Fast 只读审查；该线程超时并已关闭，未返回独立报告。
  父代理随后按 correctness、可读性/简化、架构边界、安全、性能五轴复核实际调用点、初始化
  顺序和错误回退，并记录未覆盖项；没有把超时误报为通过。
- simplification assessment：复用既有 `connection_bindings.py`，没有新增通用 facade、
  状态源、timer 或动态 registry；只移除跨 controller 动态 Qt 读取，行为保持型简化通过。
- authorized non-destructive validation：compileall、Ruff、`scripts/check.ps1` 已执行；
  GUI/EXE 启动、HIDPI、读屏、真实 UART/网络/BLE/J-Link/目标板和硬件验收未运行，原因是
  当前未获得对应授权，且本轮无设备操作需求。

## 父代理独立五轴复核

- correctness：bundle 在 preset combo refresh 前组装；所有新消费者都有缺失安全回退，
  需要动作的路径给出明确错误；transport `itemData`、preset signal 和 replay refresh
  的既有顺序未变。
- readability/simplification：复用既有 `connection_bindings.py`，每个消费者只在入口取得
  一次 typed bundle；没有为了减少行数移动状态或隐藏副作用。
- architecture：bundle 只持有 Qt 引用，catalog/store、ViewModel、policy、timer、handle
  仍由原 owner 持有；静态 owner-only 搜索确认跨 controller 的链路控件动态读取已清零。
- security：没有新增外部输入、凭据、网络、加密、文件格式或设备动作；OTA/AES/RTT/J-Link
  仍是原有 contract-only/attach-only 边界。
- performance/lifecycle：只增加一次 `getattr` + `isinstance` accessor 读取，没有新增 timer、
  signal fan-out、线程或持续动画；MotionController 120Hz/PreciseTimer/8–9ms 目标 vector 通过。

残余风险：独立 Luna 审查线程未返回报告；Windows 真机 GUI/EXE 启动、HIDPI、读屏和真实
设备链路尚未执行，需授权环境补验。

## 包交付

`local-arch-6x` onefile 已生成并覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`。canonical/root/root-latest 均为 `47,975,290` bytes，SHA-256
`AE11EE6B8BED8F04CD844994B2FA60820232391084DDD57978B2CB2FB4D4BCE8`，archive listing
SHA-256 `60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C`；签名
`NotSigned`，`release_eligible=false`，硬件验收 `not_run`。
