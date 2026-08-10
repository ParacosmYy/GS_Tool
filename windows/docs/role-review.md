# 七槽位集成复核记录

**作者：** AI Token Tracker Engineering Team  
**复核对象：** AI Token Tracker 当前 checkout  
**复核日期：** 2026-08-10  
**方法：** 代码边界审查、Python/JavaScript 编译诊断、本地 HTTP、真实浏览器 DOM/截图/控制台复核；未创建或运行测试专用资产。

## 复核结果

| 角色 | 关注点 | 结果 | 证据与遗留风险 |
|---|---|---|---|
| UI-1 视觉系统 | Material 3 语义 token、Moonshot-inspired 黑底大排版、首屏/图表/表单/空状态一致性 | 通过 | `docs/design-tokens.md`、`style.css`、`ui-polish.css`；v6 工作站背景、认证玻璃卡片和 RUST/RL 场景签名已落地，v5 仍保留为回滚资产。 |
| UI-2 动效交互 | reveal、轨道、scanline、glitch、count-up、pointer follower、异步状态 | 通过 | `docs/motion-contract.md`、`static/modules/motion.js`；pointer follower 只在移动后按需 requestAnimationFrame，reduced-motion 不启动。 |
| UI-3 响应式与可访问性 | 语义标题、label、live region、表格 caption、focus-visible、空/错误状态 | 条件通过 | 登录页已完成 320/768/1024/1440 截图、320px 横向溢出修复、焦点、ARIA、reduced-motion 和运行时颜色证据；仪表盘/管理员页面已完成单一 h1、表格 scope/caption 和详情焦点回流静态修正，受保护页面仍需合法会话复核，详见 `docs/ui-accessibility-evidence.md`。 |
| 前端工程师 | API client、CSRF、XSS 边界、Key 生命周期、状态编排 | 通过 | `app.js` 仅编排；API/图表/motion 已拆模块；渲染记录统一使用 `textContent`；Key 仅页面内存。 |
| 后端工程师 | 鉴权、用户隔离、参数化 SQL、SSRF、provider usage、响应上限、限流、日志与缓存边界 | 通过 | `web.py`/`providers.py`/`db.py`/`events.py`/`csv_export.py`；health 200、未登录 summary 401、统一错误包络、安全响应头、凭据脱敏、认证/API `no-store` 和个人/管理员有界导出边界已验证，详见 ADR-037/039/040。 |
| 架构师 | 依赖方向、API/ADR、七槽位边界、根入口、版本控制、依赖锁定和演进风险 | 通过（源码边界） | `docs/architecture.md`、`docs/api-contract.md`、ADR-003/032/033/034/035/036/037/038/039/040/041/042/043/044/045/046、`roles/`、`start.bat`/`run.py`；访问日志、Caddy 边缘预检、LAN 分享确认、敏感 artifact 隔离、本地 Git 保存点、敏感响应边界、runtime lock、统一有界 CSV、Android Release HTTPS 门禁、liveness/readiness 探针、加密会话同步持久性、运行时 HTTPS 拒绝、无副作用配置预检和显式 edge log retention 均有独立记录，外部客户端自动采集明确以 gateway/adapter 为边界。正式 HTTPS/Android 仍是环境门禁。 |

## 集成闸门

- 共享运行代码仍集中在当前 checkout，没有创建 worktree 或角色复制源代码。
- Provider adapter 当前实现 `auto → openai-compatible`；新增厂商应新增 adapter，不应在 Flask 路由增加分支。
- 自动采集不猜测缺失的 usage；没有 usage 的 provider 仍提示异常补录。
- 当前未用真实 provider Key 做上游调用验证，因此“真实 usage 入库”依赖用户提供合法 Base URL/Key，不能用演示数据冒充通过。
- 外部客户端 gateway 尚未实现；这不是当前页面自动采集链路的阻塞项，未来必须单独设计 per-user ingest token 和 Key 隔离 ADR。
