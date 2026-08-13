# SerialForge 项目约束

## 目标与范围

- 目标平台：Windows x64；Python 3.12；输出可分发的 PyInstaller EXE。
- 默认技术栈：PySide6、pyserial、Python 标准库 `socket`、PyInstaller；bleak 只作为可选 BLE extra。
- 不引入 WSL、Rust、MinGW 或 Tauri 作为首版前置条件。
- 第一版优先完成 UART 终端和日志；TCP/UDP、BLE、RTT 按独立适配器逐步接入。

## 分层与依赖方向

```text
Presentation ──> Application ──> Domain
      │               │
      └──────> Ports <┘
                    ▲
             Infrastructure
```

- `domain`：值对象、事件、协议端口和不依赖 Qt/设备库的规则；
- `application`：会话编排、命令、重连策略、记录器协调；
- `infrastructure`：pyserial、bleak、socket、RTT Telnet 等具体适配器；
- `presentation`：Qt 窗口、视图模型、信号槽和用户交互；
- `plugins`：首版只保留明确的内部接口，不加载任意第三方 Python 代码。

UI 不得直接导入 pyserial/bleak/socket；基础设施不得导入 PySide6。新增传输只能实现已有端口和适配器注册，不修改终端、记录器和协议解析器。

## 高内聚、低耦合规则

1. 一个模块只有一个主要变化理由；连接、解析、记录、显示和配置分别拥有自己的边界。
2. 依赖抽象端口，不依赖具体设备库；具体库只能出现在 `infrastructure`。
3. 公共对象使用不可变 dataclass、枚举或显式 DTO；不把 Qt widget 暴露给应用层。
4. 不使用隐式全局单例；组合根统一创建依赖并注入会话服务。
5. 传输语义必须保留：stream、datagram、BLE characteristic、RTT channel 不能都压成 `send(bytes)`。
6. 用事件和命令连接模块；禁止跨层访问私有状态或互相持有 UI 控件。
7. 任何新依赖必须说明职责、许可证、Windows 行为和替代方案。

## 线程与性能

- Qt 主线程只操作控件和短状态转换；串口、网络、BLE 扫描、文件写入和 RTT 轮询运行在 worker/QThread 或受控后台任务中；
- 设备输入通过有界队列进入应用层；终端按批次刷新，不能每字节刷新；
- 记录队列和 UI 预览队列分离，丢弃、背压、磁盘错误必须可观测；
- 关闭会话必须可取消，并先停止 worker 再释放底层句柄；
- 默认不记录密码、密钥、完整敏感 payload 和本地隐私路径。

## 敏捷六角色工作流

每个功能开发周期必须先经过一次六角色只读评审，再由父代理整合。六个角色的职责固定为：

1. 产品：用户结果、范围、非范围、验收和优先级；
2. 架构：分层、端口、依赖方向、并发、生命周期和扩展边界；
3. UI 设计：布局、交互、可访问性、动效、状态和键盘路径；
4. 开发：给出最小完整实现建议；只有父代理或明确指定的一个写入者可以修改源码；
5. 验证：静态、编译、现有构建、手工向量、打包和授权硬件证据；
6. 打包/流程：依赖、许可证、版本、PyInstaller 产物隔离、交付和 handoff 完整性。

六个角色默认使用 `luna_max`（gpt-5.6-luna、max、Fast）；只有复杂调用链/并发/安全分析或高风险
复核无法可靠解决时，才升级 `terra_max`；仍无法解决的系统级或安全关键问题才升级 `sol_medium`。
子代理必须收到不重叠的范围和只读/写入权限，返回证据、假设、未决风险；父代理负责整合、最终
diff 检查和验证，不能把子代理意见直接当成通过证据。

每轮执行以下门：定位（读取本文件与相关 docs）→定义（结果/范围/恢复/性能/验证）→六角色评审
→唯一写入者实现→独立复核与简化→非破坏性验证→交接。高风险 RTT、BLE、网络监听、动态代码
加载和嵌入式代码不得凭直觉放行。

## 协作、工作区与授权边界

- 只在当前共享 checkout 工作；不创建、切换、删除或操作 Git/Codex worktree，不 clone 到其他目录；
- 保留用户已有修改；不使用破坏性 reset/checkout；文件删除或覆盖前必须确认精确目标和范围；
- 每个周期最多一个源码写入代理，其他角色只读；不允许多个代理同时写相同文件；
- 不向用户发送外部消息、创建 PR、发布包或操作真实设备，除非用户明确授权；
- 用户已要求“后面不允许一直启动这个软件”：后续默认不启动 GUI、EXE 或后台服务；只做静态检查、
  编译/导入检查和短时进程内纯数据向量。需要 GUI/offscreen、打包启动、真实 UART/BLE/Wi-Fi/
  TCP/UDP/J-Link 验证时，必须在 handoff 中列为待授权项目，不能默认为已通过。

## UI 快速迭代约束

- UI 改动优先限定在 `presentation` 和资源层，业务、transport、protocol、recorder 不因视觉调整
  直接耦合；
- 二次元背景/GIF/粒子等动效必须可暂停、支持 reduced-motion、默认低干扰，并有资源加载失败的
  静态回退；动效状态不能成为业务状态或连接状态的唯一表达；
- 视觉方案已生成多个候选时，必须先记录用户选定方向再编码；没有选择时只能做不依赖视觉方向的
  文档/后端工作，不能擅自固化美术风格。

## 嵌入式 C/C++ assurance

如果修改 MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、driver、protocol、boot/OTA、Flash/NVM、power、
motor-control 或其他固件 C/C++，必须先加载并执行 `$mcu`、`$embedded-enterprise-workflow`、
`$embedded-code-review-simplifier`：先记录公开的一手、版本化、目标适用的 vendor source，再做
最小修改、独立复核、行为保持的简化评估和风险级 R&D 验证。没有项目规则、工具覆盖、批准和证据，
不得声称 MISRA、ISO 26262、ASIL、ASPICE、汽车或认证合规。Python desktop 变更不适用这些固件
要求，但 handoff 必须记录 applicability=N/A，不能借此推导固件合规。

## 验证与修改边界

- 不创建或修改 unit test、mock、fixture、test harness 或其他测试专用资产；
- 默认不运行现有单元测试；优先使用静态检查、编译/启动检查、现有构建、手工回环和授权硬件验证；
- 不声称硬件、许可证或性能兼容，除非有实际证据；
- 每次开发最多一个源码写入代理，其他角色只读评审；父代理负责整合、最终 diff 检查和验证。

## 交接文档（强制）

每次开发周期、暂停、阻塞或交由下一位协作者继续时，都必须写交接文档：

- `docs/handoffs/current.md` 始终保存最新交接入口；交接文件不得放在仓库根目录；
- 每轮还必须归档到 `docs/handoffs/YYYY-MM-DD-<scope>.md`，历史文件不覆盖；
- 交接必须写清：日期/范围、用户结果、已完成与未完成、实际改动文件、六角色结果及 agent id、
  架构和简化决策、验证命令与真实输出、未运行项目及原因、授权/硬件状态、风险、下一步和需要
  用户选择的事项；
- “建议/计划/静态通过/真实设备通过/正式发行通过”必须使用不同措辞；没有证据不得写成通过；
- 若本轮没有代码或文档变更，也要写 handoff，记录原因和下一步，不能用口头消息代替。
