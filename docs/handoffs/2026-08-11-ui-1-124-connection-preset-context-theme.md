# UI-1.124 连接快速配置上下文主题 surface 交接

日期：2026-08-11

## 交付内容

- `ConnectionPresetContextSurface.set_preset()` 继续消费既有 preset projection，并以
  `refresh_dynamic_property()` 同步 `source/state`。
- 空态、内置 preset、自定义 preset 分别使用 neutral、info、history semantic surface/border；
  marker/accent 与三套主题保持一致。
- 不新增连接动作、状态源、timer、线程、I/O、资源或 OTA/AES/RTT/J-Link 依赖；apply/connect、
  hint、tooltip、AccessibleDescription、焦点/Tab、NoFocus、鼠标透明和 shared frame/stop 不变。

## 验证

- `scripts/check.ps1`：pass（157 files <= 1000；3 themes；22 semantic tokens；19 selectors；
  `legacy_qss_literals=0`）。
- compileall：pass。
- Ruff：pass。
- UI124 三主题空态/内置/自定义 vector：pass；中心像素均为非白色语义 surface，截图已人工查看。
- package：pass；provenance verify：pass。
- 架构师线程 `019fed48-9164-72c3-91f3-7472ab0cf517` 超时，未计为独立通过；父代理完成架构、
  代码质量、简化、性能与 accessibility 审查。
- 未修改嵌入式 C/C++；embedded applicability=N/A。真实 GUI/EXE startup、硬件验收未运行。

## 产物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.124`  
size：47,941,349 bytes  
SHA-256：`79A366634CF0B87C629416194AC8891F7BFE6BB08CB680F3DE050D9A99D3E3BC`  
archive listing SHA-256：`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`  
root copy：pending-user-close（PID 46108、49236）
