# 多模式应用框架 + OTA 升级模式 设计

> 日期：2026-06-18
> 范围：把单一串口窗口重构为左侧图标导航的多模式应用 Shell，首批接入 OTA（X/YMODEM）模式。
> 参考：EK-OmniProbe 的 `ModeSwitch` + `panelRegistry` + 各 `modes/` 子目录。

## 一、目标与范围

把当前「打开即一个串口窗口」的应用，重构为「左侧图标导航栏 + 右侧可切换工作区」的多模式应用。
首批导航入口：**串口（现有）/ OTA（X/YMODEM）/ RTT（占位）/ 设置（占位）**。
本轮重点实现 **AppShell 框架 + 串口模式接入 + OTA 引擎与 UI 壳**；RTT/设置先做占位面板。

### 非目标
- 不做 RTT/设置的真实功能（占位面板，后续独立 spec）。
- 不做真实设备 OTA 升级验证（D2 替身/loopback 口径，真实硬件留待有设备时）。
- 不改串口模式现有三栏布局的业务逻辑（作为面板整体接入）。

## 二、架构决策（已与用户确认）

1. **导航形态**：图标窄导航栏（≈56px），每个图标=一个功能模式，点图标切换右侧整个工作区。
2. **框架衔接**：**新建 AppShell 包裹现有串口窗口**（不改造 SerialStationMainWindow 内部）。
3. **TopBar 不共享**：每个模式拥有自己的完整布局（串口模式保留其三栏+TopBar，OTA 有自己的布局）。
4. **模式组织**：AppShell + PanelRegistry（方案 A）。每个模式实现 `ModePanel` 协议，通过 registry 注册。
5. **OTA 通道**：**复用已连接的串口**（共享 transport）。串口模式连上设备后，切到 OTA 直接用该连接发包。
6. **OTA 协议**：XMODEM / XMODEM-CRC / YMODEM / YMODEM-g 全套。
7. **验收深度**：引擎 + 协议状态机 + pytest 单测（fake/loopback transport）+ UI 壳。D2 口径。

## 三、目录结构（新增）

```text
python/embeddebug/app/
  app_shell.py          # AppShell(QMainWindow): NavRail + QStackedWidget 装配
  mode_panel.py         # ModePanel 协议 + PanelRegistry + register_panel
  app_controller.py     # AppController: 持有 SerialWorkbenchController + 共享 transport 访问
  main.py               # 改为构建 AppShell（不再直接是串口窗口）

python/embeddebug/ota/                       # OTA 子系统（独立包，不依赖 ui）
  __init__.py
  protocols/
    __init__.py
    base.py             # OtaProtocol 协议 + OtaBlock/TransferResult 数据类
    xmodem.py           # XMODEM (CKSUM) + XMODEM-CRC 状态机
    ymodem.py           # YMODEM + YMODEM-g 状态机（批量 + 1K 块）
  transfer_engine.py    # TransferEngine: 协议无关的块发送/ACK/NAK/超时/重传编排
  transport_adapter.py  # 把 SerialTransport 适配为 OtaByteChannel(读/写/超时)

python/embeddebug/serial_station/ui/
  panels/
    serial_panel.py     # 串口模式面板：包现有 build_main_layout 三栏布局
    ota_panel.py        # OTA 模式面板：文件选择 + 协议选择 + 进度 + 日志
    placeholder_panel.py # RTT/设置占位面板
```

测试落 `tests/python/unit/ota/`（协议状态机 + 引擎）和 `tests/python/ui_smoke/`（shell 切换）。

## 四、核心接口

### ModePanel 协议（mode_panel.py）
```python
class ModePanel(Protocol):
    def build(self, app_controller: AppController) -> QWidget: ...
    def on_enter(self) -> None: ...   # 切入时调用（如 OTA 检查连接）
    def on_leave(self) -> None: ...   # 切出时调用（可中止任务）

PanelFactory = Callable[[AppController], ModePanel]

@dataclass(frozen=True)
class PanelRegistration:
    mode_id: str; icon: str; label: str; factory: PanelFactory

PANEL_REGISTRY: list[PanelRegistration] = []
def register_panel(mode_id, icon, label, factory) -> None: ...
def registered_panels() -> tuple[PanelRegistration, ...]: ...
```

### AppController（app_controller.py）
```python
class AppController:
    """应用级控制器：持有串口 controller，提供共享 transport 访问。"""
    def __init__(self) -> None:
        self._serial_controller = SerialWorkbenchController()
    @property
    def serial_controller(self) -> SerialWorkbenchController: ...
    def active_transport(self) -> SerialTransport | None:
        """返回当前已连接的串口 transport，未连接返回 None。"""
        return self._serial_controller.active_transport  # 新增访问器
    def is_connected(self) -> bool: ...
```

### SerialWorkbenchController 改动（最小）
新增一个公共属性 `active_transport -> SerialTransport | None`（返回 `self._transport_runtime.transport` 若 `is_connected` 否则 `None`）。零行为改动，只是暴露已有对象。

### OtaByteChannel（transport_adapter.py）
```python
class OtaByteChannel(Protocol):
    def write(self, data: bytes) -> int: ...
    def read(self, timeout_ms: int) -> bytes: ...  # 阻塞读，超时返回 b""
```
`SerialTransport` 有 `write` 但没有阻塞 `read`（它是 callback 推送）。适配器在 `on_bytes_received` 回调基础上加一个带超时的 `read()`（用 queue 收集回调字节）。

## 五、OTA 协议设计

### XMODEM 家族
- **块大小**：128B（标准）/ 1024B（YMODEM 可选）。
- **校验**：CKSUM（XMODEM）/ CRC-16（XMODEM-CRC、YMODEM）。
- **握手**：XMODEM 用 NAK（CKSUM）或 'C'（CRC）启动；YMODEM 由接收方发 'C'。
- **控制字节**：SOH(0x01,128B)/STX(0x02,1KB)/EOT(0x04)/ACK(0x06)/NAK(0x15)/CAN(0x18)。
- **重传**：NAK 或超时重发当前块，达到最大重试（默认 10）中止。

### YMODEM
- 在 XMODEM-CRC 基础上：块 0 传文件信息（文件名 + 大小 + 时间戳），之后传数据块，结束发空块名。
- **YMODEM-g**：流式发送无 ACK 等待（发送方不等待 ACK 连续发），出错用 CAN 中止；速度最快。

### TransferEngine（transfer_engine.py）
协议无关编排：给定 `OtaProtocol`（提供 `next_block()`/`handle_response()`）+ `OtaByteChannel`，
驱动 发送块→等待 ACK/NAK→超时重传→EOT 收尾→返回 `TransferResult(success, blocks_sent, retries, errors)`。

## 六、数据流

```
用户在串口模式连接设备 (SerialWorkbenchController.connect_*)
   → SerialTransport 绑定到 controller._transport_runtime
用户点 OTA 图标 → AppShell 切到 OTA 面板 (on_enter 检查 is_connected)
用户选固件文件 + 协议 → 点"开始升级"
   → OtaPanel 调 AppController.active_transport() 拿 SerialTransport
   → SerialTransportAdapter 包成 OtaByteChannel
   → TransferEngine 用选定 OtaProtocol 驱动发送
   → 进度/日志通过 signal 回 OtaPanel 更新 UI
```

## 七、UI 布局

### AppShell
```
┌────┬──────────────────────────────────────┐
│ 📡 │                                      │
│ ⬆️ │   QStackedWidget                     │
│ 📊 │   (serial / ota / rtt / settings)    │
│ ⚙️ │                                      │
│56px│                                      │
└────┴──────────────────────────────────────┘
```
NavRail objectName: `serialStationNavRail`；图标按钮 `serialStationNavSerialBtn` /
`serialStationNavOtaBtn` / `serialStationNavRttBtn` / `serialStationNavSettingsBtn`。
激活项用强调青软底高亮（QSS）。

### OTA 面板
```
┌─ OTA 升级 ────────────────────────┐
│ 协议: [XMODEM-CRC ▼]              │
│ 文件: [__________] [选择]          │
│ ───────────────────────────────── │
│ [████████░░░░░░] 60% (块 24/40)   │
│ ───────────────────────────────── │
│ 日志:                              │
│ > 发送块 24 (CRC OK)              │
│ > 收到 ACK                        │
│ ───────────────────────────────── │
│ 状态: ● 已连接 COM3   [开始升级]   │
└──────────────────────────────────┘
```
objectName 统一 `serialStationOta*` 前缀。

## 八、测试策略

- **协议状态机单测**（`tests/python/unit/ota/test_xmodem.py` / `test_ymodem.py`）：
  fake OtaByteChannel 回放 ACK/NAK/超时，断言块序、重传、CRC、EOT 收尾、TransferResult。
- **引擎单测**（`test_transfer_engine.py`）：跨协议的重传上限、错误中止、成功路径。
- **Shell 切换 smoke**（`tests/python/ui_smoke/test_app_shell.py`）：4 图标 findChild 可达，
  点击切换 QStackedWidget 当前页，on_enter/on_leave 触发。
- **OTA UI smoke**：文件选择/协议选择/进度/日志控件 findChild 可达；
  连接态 gating（未连接时"开始升级"禁用）。

## 九、约束遵守

- 所有新 `.py` ≤300 行；新 `serialStation*` objectName 必须在 `build_qss()` 覆盖（QSS 覆盖率守护）。
- `ota/` 包不 import ui（协议引擎与展示分离）；`panels/` 只通过 AppController 访问 controller。
- 不改串口模式现有 objectName 与 action 模块契约。
- `EmbedDebug.bat → uv run start-embeddebug` 启动链路保持可用；收口验证 `uv run start-embeddebug --smoke` + `cmd /c EmbedDebug.bat --smoke`。

## 十、三轴状态

| 轴 | 本轮 | 说明 |
|---|---|---|
| 工程 | E4 | AppShell + 4 面板 + OTA 引擎全落地，pytest 覆盖协议/引擎/shell |
| 用户 | U3 | 导航切换 + OTA 文件选择/进度/日志闭环（替身传输） |
| 设备 | D2 | OTA 用 fake/loopback 验证，真实 bootloader 设备未验证 |
