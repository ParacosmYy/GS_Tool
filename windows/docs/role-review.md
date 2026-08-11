# 六角色集成复核记录

**作者：** AI Token Tracker Engineering Team  
**复核对象：** AI Token Tracker 当前 checkout  
**复核日期：** 2026-08-10  
**方法：** 代码边界审查、Python/JavaScript 编译诊断、本地 HTTP、真实浏览器 DOM/截图/控制台复核；未创建或运行测试专用资产。

## 复核结果

| 角色 | 关注点 | 结果 | 证据与遗留风险 |
|---|---|---|---|
| UI-1 视觉系统 | Material 3 语义 token、Moonshot-inspired 黑底大排版、首屏/图表/表单/空状态一致性 | 通过 | `docs/design-tokens.md`、`style.css`、`ui-polish.css`、`scene-motion.css`；v14 御姐风嵌入式工程师、MacBook Pro/Mac Studio 风格工作站、Rust/RL 屏幕语义、显式图像场景层、认证玻璃卡片和可读性遮罩已落地；v28 补充半透明玻璃层、独立表单表面、focus-within 和信号轨道降级，v29 修正左上场景遮罩并完成导航玻璃层运行观察，v30 补充图表空态行动入口和非空回归，v31 修复登录首帧黑幕并收紧 Admin 数据轨道；v13/v12/v11/v10/v9/v8 仍保留为回滚资产；合成边界见 ADR-057/065/070/074/076/080/086/090/095。 |
| UI-2 动效交互 | reveal、轨道、scanline、glitch、count-up、pointer follower、异步状态 | 通过 | `docs/motion-contract.md`、`static/modules/motion.js`、Android `ui/MotionPreferences.kt`；Web pointer follower 只在移动后按需 requestAnimationFrame，Web reduced-motion 与 Android 系统动画缩放均不启动对应无限装饰循环，详见 ADR-079。 |
| UI-3 响应式与可访问性 | 语义标题、label、live region、表格 caption、focus-visible、空/错误状态 | 条件通过 | 登录/注册页与受保护 Dashboard/Admin 已完成隔离合法会话下 320/768/1024/1440 viewport、横向溢出、焦点回流、ARIA、动态错误播报、空态、历史非空、脱敏日志详情、服务端 CSV 200、Provider 502 错误态；认证页 v23 已将唯一语义 h1 与 aria-hidden glitch 视觉层分离；个人 Activity 表单/最近表格已完成 v19 隔离会话下成功、校验错误、恢复、锚点留白和默认视口无溢出证据；v26 补充 v14 默认浏览器首屏，v27 补齐当前 v14 320/768/1024/1440 viewport、唯一 h1、焦点、文字颜色和清洁控制台证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、reduced-motion/高对比度真实环境仍待部署级复核，详见 `docs/ui-accessibility-evidence.md`。 |
| 前端工程师 | API client、CSRF、XSS 边界、Key 生命周期、状态编排、静态 UI 契约 | 通过 | `app.js` 仅编排；API/图表/motion 已拆模块；渲染记录统一使用 `textContent`；Key 仅页面内存；`token_tracker audit` 已检查 label、table caption/scope、图片 alt、跳过链接和动效降级。 |
| 后端工程师 | 鉴权、用户隔离、参数化 SQL、SSRF、provider usage、响应上限、限流、日志与缓存边界 | 通过（源码边界）；真实 provider 联调待授权 | `web.py`/`providers.py`/`provider_projection.py`/`ingest_auth.py`/`gateway.py`/`gateway_reporting.py`/`gateway_contracts.py`/`gateway_queue.py`/`backup.py`/`db.py`/`events.py`/`csv_export.py`；health 200、未登录 summary 401、统一错误包络、安全响应头、凭据脱敏、provider 原始响应投影、Usage Ingest digest/到期/撤销/固定 source、Gateway loopback/独立访问令牌、上游 URL/大小/超时、JSON/SSE usage 解析、有界退避、DPAPI 跨重启恢复、响应读取异常映射、幂等键控制字符校验、非回环 HTTP 拒绝、队列元数据 fail-closed 和运行时队列故障熔断、动态中心端口只读发现、备份清单只读策略和完整性边界已通过源码与隔离 smoke，真实 provider Key 联调仍待授权，详见 ADR-037/039/040/052/053/054/056/058/059/060/068/075/077。个人入口 host 解析由架构边界收敛，详见 ADR-078。 |
| 架构师 | 依赖方向、API/ADR、六角色边界、根入口、版本控制、依赖锁定和演进风险 | 通过（源码边界）；正式环境门禁进行中 | `docs/architecture.md`、`docs/api-contract.md`、ADR-003/032/033/034/035/036/037/038/039/040/041/042/043/044/045/046/047/048/049/050/051/052/053/054/055/056/057/058/059/060/061/062/063/064/065/066/067/068/069/070/071/072/073/074/075/076/077/078/079/080/081/082/083/084/085/086/087/088/089/090/091/092/093/094/095/096/097/098/099、`roles/`、`start.bat`/`run.py`；访问日志、Caddy 边缘预检、敏感 artifact 隔离、本地 Git 保存点、provider 响应投影、runtime lock、统一有界 CSV、代码/文本 1000 行门禁、production/Caddy/LAN 部署契约、Android Release HTTPS 门禁、liveness/readiness 探针、跨端 v14 资产、稳定场景主图层、场景可见度构图、Web UI 静态契约、Kimi provider 预设、Kimi 官方 endpoint 上下文与 Key 隔离、EXE 构建工具链锁定、只读发布审计、本地 Chart.js 供应链、Usage Ingest、Gateway 契约、响应可靠性、DPAPI 重试队列完整性、运行时队列故障边界、备份 schema/身份完整性、只读备份清单与保留策略边界、生产 loopback 绑定、动态 live-region 契约、分享 handoff 预检、个人 Activity 写入边界、动态中心发现、个人入口 host fail-closed、CLI Waitress/显式共享绑定、签名 fail-closed、Android 系统动效策略、用户授权 SDK 入口和 LAN/示例 Production 预检证据均有记录；Gateway 真实上游联调、正式 HTTPS/Android/签名/正式分发/真实设备和真实备份责任人仍是门禁。 |

## 集成闸门

- 共享运行代码仍集中在当前 checkout，没有创建 worktree 或角色复制源代码。
- Provider adapter 当前实现 `auto → openai-compatible`；新增厂商应新增 adapter，不应在 Flask 路由增加分支。
- 交付层通过 `release-doctor.ps1/.bat` 编排单项门禁；它只报告源码、工具链和部署状态，不安装软件或改变主机状态（ADR-071）。
- 自动采集不猜测缺失的 usage；没有 usage 的 provider 仍提示异常补录。
- 当前未用真实 provider Key 做上游调用验证，因此“真实 usage 入库”依赖用户提供合法 Base URL/Key，不能用演示数据冒充通过。
- 外部客户端的 per-user Usage Ingest Token、固定 `/api/v1/ingest/usage` 边界和本地 Gateway 已实现；Kimi Code 等 stock 客户端的真实 provider 联调仍需授权 Key，provider Key 隔离遵守 ADR-054/056/058/059/075。
