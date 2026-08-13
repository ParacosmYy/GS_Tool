# UI-1.125 链路连接状态轨道终态 marker 交接

日期：2026-08-11

## 交付内容

- `ConnectionStatusRail` 在既有四节点路径上增加 `open` 完成勾和 `error` 叉号。
- opening/closing/open 继续由共享 MotionController frame 驱动彗尾/脉冲；closed/discovered、
  stop、暂停、低动效、隐藏/最小化/关闭保持静态。
- 不新增状态源、连接动作、timer、线程、I/O、外部资源或 OTA/AES/RTT/J-Link 依赖；
  NoFocus、鼠标透明、状态文本、焦点/Tab 与 controller owner 不变。

## 验证

- `scripts/check.ps1`：pass（157 files <= 1000；3 themes；22 semantic tokens；19 selectors；
  `legacy_qss_literals=0`）。
- compileall：pass。
- Ruff：pass。
- UI125 production-host vector：pass（三主题、六状态、animated transition、stop、exact-white=0）。
- 首轮普通 QWidget 子控件抓图出现白色背景，已定位为错误验证宿主；修正为生产 `appRoot`/
  `connectionControlBand` 后重新通过。
- package：pass；provenance verify：pass。
- 架构师线程 `019fed4f-d2a8-7d33-9cdd-b536efe3e106` 超时，未计为独立通过；父代理完成架构、
  代码质量、简化、性能与 accessibility 审查。
- 未修改嵌入式 C/C++；embedded applicability=N/A。真实 EXE startup、硬件验收未运行。

## 产物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.125`  
size：47,942,017 bytes  
SHA-256：`ACC5FCA833E17EF0D648FCBE7CB7DB1ECB8557B43BD4C920371EF2CCDEF49142`  
archive listing SHA-256：`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`  
root copy：pending-user-close（PID 46108、49236）
