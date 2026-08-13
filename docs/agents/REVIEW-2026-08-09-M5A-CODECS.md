# M5a schema v2 JSON/TLV codec review

日期：2026-08-09  
范围：旧 component profile v1 兼容、schema v2 JSON Pointer/TLV RX codec、bounded worker/UI、
Windows workflow 和默认 PyInstaller 发行门。  
写入者：父代理；六角色子代理只读。

## 六角色评审

| 角色 | 结论 | 关键证据/建议 |
|---|---|---|
| 产品 | 条件通过 | 先交付 RX 派生预览；TX 编码、曲线、回放、迁移和全传输覆盖拆后续切片。 |
| 架构 | 条件通过 | `domain.codecs` 独立持有 v2 typed config；旧 v1 留在 `domain.components`；`ProtocolConfig` 不承载 codec。 |
| UI 设计 | 条件通过 | 现有组件入口改为显式“Profile / Codec”；标签显示 legacy v1 或 JSON/TLV v2；raw terminal/Hex 保留。 |
| 开发 | 可编码 | `ComponentCodecRouter` 选择内置 codec；JSON Pointer 无表达式；TLV 平面 tag/length/value；worker 解码不持锁。 |
| 验证 | revise 后通过 | 先要求 malformed、bounds、version、duplicate key/tag、worker error 都转 row/field/codec error；本轮已补齐 inline vectors。 |
| 打包/workflow | revise 后通过 | 只使用 Python 标准库 `json`/`struct`；新增 Windows quality workflow；默认包不收集 Bleak/WinRT。 |

父代理冻结的未决项：M5a 只处理 UART/TCP Client RX；UDP、TCP Server、BLE、RTT 维持 raw-only；不做
TX 自动编码、不做动态插件、不做静默 profile 迁移。

## 实现边界

- `src/serialforge/domain/codecs.py`：schema v2 config、JSON Pointer、平面 TLV、显式 router；
- `src/serialforge/domain/components.py`：增加跨 codec `ValueKind`、`codec_error` 和 codec 统计；
- `src/serialforge/application/components.py`：config union、bounded store、generation 快照和
  解码锁边界；
- `src/serialforge/domain/ports.py`、`domain/events.py`、ViewModel/UI：传递 v1/v2 配置并显示派生错误；
- `docs/adr/0009-structured-component-codecs-m5a.md`、`README.md`、`docs/PROTOCOLS.md`、
  `docs/ARCHITECTURE.md`、`docs/DEPENDENCIES.md`、`docs/CONSTRAINTS.md`：记录契约和非目标；
- `.github/workflows/windows-quality.yml`：Windows locked check + 默认 onedir artifact。

## 验证证据

以下均在当前 `D:\Workplace\Agent_Workplace\SerialForge` 完成，未创建或运行测试专用代码/资产：

1. `uv run --locked ruff check --no-cache src`：通过；
2. `uv run --locked ruff format --check --no-cache src`：37 files already formatted；
3. `uv run --locked python -m compileall -q src`：通过；
4. `uv lock --check`：通过；
5. inline vectors：v1 round-trip、JSON nested path/scale、strict UTF-8、duplicate key、
   malformed JSON、missing/type error、schema version/unknown key/path rejection、TLV endian/scale、
   truncation、unknown-tag error、worker event：通过；
6. PySide6 offscreen：窗口启动/配置 v2/标签更新/关闭和 session/protocol/component/recorder
   worker shutdown：通过；仅有已知 `QFontDatabase` 缺少字体目录提示；
7. `.\scripts\check.ps1`：通过；
8. `.\scripts\package.ps1 -Mode onedir`：通过；实际窗口句柄 `WM_CLOSE` 退出码 0；
9. `.\scripts\package.ps1 -Mode onefile`：通过；实际验证 PyInstaller parent/GUI child，
   GUI child `WM_CLOSE` 后 parent/child 均退出。

最终默认产物：

- `dist/SerialForge.exe`：47,428,052 bytes，SHA256
  `DC219D653DA3557321D232D74B0834E047A3C1046538F691D64CC26EDBFD5A01`；
- `dist/SerialForge/SerialForge.exe`：2,933,792 bytes，SHA256
  `99248D6AC10B4D8FC6FC2DBFD7633CD63D98CD844FD5C2E6AD046B496CB7BDD6`；
- 默认 onedir 递归文件中 Bleak/WinRT 名称匹配：0；
- 最终检查时无残留 SerialForge 进程。

## 未运行项目与风险

- 未连接真实 UART/TCP/BLE/目标板；未验证 JSON/TLV 设备吞吐、长时间压力、协议迁移和全传输
  source/channel 语义；
- 未安装/运行 J-Link 驱动和工具；RTT 仍按 M6 attach-only raw bridge 约束；
- 未做 onefile/onedir clean Windows、签名、安装器、许可证审计和 BLE-enabled 发行门；
- TLV 平面方言、JSON Pointer 和 v2 schema 需要后续设备样本反馈再扩展，禁止用任意脚本或动态
  import 绕过边界。
