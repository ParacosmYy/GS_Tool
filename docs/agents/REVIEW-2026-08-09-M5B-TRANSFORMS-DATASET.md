# M5b 六角色复核记录：typed transform 与 bounded dataset

日期：2026-08-09  
范围：`ComponentFieldValue.value`、有限 transform chain、Dataset schema v1、Dataset worker、
事件总线、ViewModel/UI 预览与 CSV 导出。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 |
|---|---|---|
| 产品 | `019fe445-91bd-79f1-8340-590c24158142` | 通过；M5b 只做 typed RX 派生数据，不扩大到 TX/曲线/回放 |
| 架构 | `019fe445-91f2-7960-93fc-d235ae3500c1` | 通过；DatasetPipelinePort 与 component/protocol/transport 分离 |
| UI 设计 | `019fe445-9232-7a23-84c2-d56715a4ec24` | 通过；状态、容量、错误和 bounded preview 可见 |
| 开发 | `019fe445-9271-75d2-a568-a8b5b4ce8272` | 通过；提出 typed value、有限 chain 和 generation worker |
| 验证 | `019fe445-92ae-75f1-80ae-41e516f3bb0d` | 通过；指出 root pointer、非有限值和 worker fallback 风险，均已修复 |
| 打包/流程 | `019fe445-92e9-7ff1-b602-4971e5b2e879` | 通过；无新增运行时依赖，默认 BLE-free 包门通过 |

六个子代理已关闭；父代理负责整合、最终 diff 复核和验证。

## 设计边界

- Component field 同时保留 `raw`、有界 `display` 和有限 scalar `value`；后续 transform 不解析 display；
- Dataset schema v1 只接受严格的 `name/schema_version/capacity/series`；series 只声明 field、unit 和
  `scale/offset/clamp/enum`；每条 chain 最多 8 项，enum 只能是最后一项；
- Dataset worker 输入最多 128 项/256 KiB，窗口最多 1024 条，默认 256 条；configure/reset 使用
  generation 丢弃旧任务；
- 只观察 UART/TCP Client RX 产生的 ComponentFramesDecodedEvent；UDP、TCP Server、BLE、RTT
  继续 raw-only；raw recorder、终端、framing、TX 和会话生命周期不依赖 Dataset 成功。

## 复核发现与修复

1. root JSON Pointer 的空路径必须序列化为 `""`，不能错误生成 `/`；已修复 `_encode_pointer`，并用
   root number/JSON inline vector 验证。
2. Float32、scale 和 transform 的 NaN/Infinity 必须在边界拒绝；已加入有限值校验，坏值保持可见
   error，不穿透到 Dataset。
3. component worker 的单帧异常不能丢掉整个批次；已生成 bounded `codec_error` row。
4. Dataset 必须处理上游 typed value，不能从展示文本重新解析；`sample_from_row` 与 worker 已按
   typed boundary 实现。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `uv run --locked ruff format --check --no-cache src`：通过，40 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `.\scripts\check.ps1`：通过；
- inline vectors：M5a v1/v2 JSON/TLV 兼容、root pointer、坏 JSON、TLV 边界、Float32/scale 非有限
  值、scale/offset/clamp、enum known/unknown、Dataset JSON roundtrip、worker event：通过；
- composition lifecycle：通过；protocol/component/dataset/recorder/session 均在关闭预算内退出；
- Qt offscreen startup/config/close：通过；保留已知 `QFontDatabase` 缺少字体目录提示，不影响启动；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit：通过；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit：通过；
- 默认 onedir 中 Bleak/WinRT 文件名匹配数：`0`。

最终主线包：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 2,963,321 B | `E16F476E2A1C3DD0F699202912C5F672C1547EE0E4E70DF368207DAD0D53D980` |
| `dist/SerialForge.exe` | 47,459,084 B | `269EB19A21DC4719225343BD885B7CB8958CE7F1A1A637E1D0DB44B9DE99D0B0` |

## 未运行/未宣称

- 真实 UART USB 回环、真实 TCP/UDP 高吞吐、BLE 扫描/配对/通知、J-Link/目标板、干净 Windows、
  代码签名和第三方许可证发布审查仍未在当前环境完成；
- RTT 仍是 attach-only，J-Link 驱动和工具未纳入当前包，也没有 memory/halt/flash/SDK/DLL 能力；
- 本轮没有创建或修改单元测试、mock、fixture、test harness 或其他 test-only asset。

## 简化评估

- 没有引入通用插件运行时、表达式解释器、数据库或图表依赖；
- 通过 `DatasetPipelinePort` 和 immutable DTO 隔离变换与 UI，避免为每种传输复制一套控件逻辑；
- 固定 transform registry、严格配置和有界队列使错误/资源预算可审计；后续曲线、回放只需订阅
  DatasetBatchEvent，不必侵入 transport 或 component codec。
