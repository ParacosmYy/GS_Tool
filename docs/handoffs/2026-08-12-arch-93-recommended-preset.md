# ARCH-93 / UI-1.166 推荐连接 preset 交接

日期：2026-08-12  
范围：首屏连接配置 hydration、stale preset/context 清理、用户可用性回归。  
状态：源码、离屏验证与 onefile 构建完成；根目录旧包已覆盖。

## 用户可感知变化

- 首次打开默认显示并填入 `UART · 115200 8N1`，不再要求用户先打开快速配置下拉框。
- 端口仍要求用户选择/输入，连接按钮不会被 hydration 自动触发。
- 手动切换 TCP、UDP、BLE、RTT 时，旧 UART preset/context 自动清掉；选择任意其他 profile 后
  transport、字段、摘要和 accessibility 文案重新同步。

## 架构与审查

- stable key 位于 `presentation/connection_presets.py`；一次性 hydration 位于
  `controllers/connection_presets.py`；bootstrap 只触发；stale guard 位于
  `controllers/connection_runtime.py`。
- 没有新增业务状态源、timer、线程、硬件访问、自动连接或密钥持久化；所有 profile 仍复用既有
  typed bindings/controller。
- 架构师调用与独立 reviewer 调用超时关闭，未伪造外部 PASS；父代理完成 fresh 五轴 review 和
  简化评估，无 Required finding。
- Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability 为 N/A，
  不作固件或认证合规声明。

## 验证

- pass：Ruff、compileall、`scripts/check.ps1`、source-limit/theme audit。
- pass：启动推荐 profile、accessibility、无自动 session、TCP stale clear、TCP apply、显式 clear。
- pass：7 builtin profiles 全部切换，4 workspace visible scroll 无横向溢出；980×720 连接带 138px、
  UART panel 217px。
- 未运行：EXE 启动、真实显示器/HIDPI/FPS、UART/网络/BLE/RTT 实连、OTA/AES 和签名验收。

## 交付物

构建命令：`./scripts/package.ps1 -Mode onefile -SourceRevision local-arch-93`。  
已验证 canonical、根目录 `SerialForge.exe`、根目录 `SerialForge-latest.exe` 三者字节一致，并运行
provenance verify：size `48,008,840` bytes，SHA-256
`14B679D1F92E03BCB1F189E3DD9012DF8D7C0ACE3C0934D75B5C285F0DC13E9A`，archive listing SHA-256
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，source revision
`local-arch-93`，signature `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
