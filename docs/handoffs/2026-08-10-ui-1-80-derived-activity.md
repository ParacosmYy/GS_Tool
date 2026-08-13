# UI-1.80 派生快照 Activity Pulse 交接

日期：2026-08-10  
工作区：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者  

## 交付结果

Protocol → Component → Dataset → Curve 派生链路收到真实非空 `ComponentFrameRow` 或 `DatasetSample` 快照时，当前可见 Protocol workspace 的既有
status rail、pipeline surface 和相关 signal consumers 会获得一次 360ms shared activity pulse。该反馈帮助用户确认异步派生结果已经进入 UI，
不改变任何业务数据。

空快照、bootstrap hydration、后台页、隐藏/最小化/关闭、暂停和 reduced-motion 保持静态；DTO、stats、表格/预览/曲线内容、renderer throttle、
clear、焦点、Tab/accessibility 语义保持不变。

## 修改文件

- `src/serialforge/presentation/controllers/derived_data.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0067-derived-activity.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 评审与简化

```text
产品角色       019febd2-d13a-7e50-a975-b807b8ab5427  called before source edit; wait timed out twice; closed
架构角色       019febd2-d183-7772-a53e-1bda9ed177b3  called before source edit; wait timed out twice; closed
UI 设计角色    019febd2-d1ce-7272-99c3-f14df1a3655c  called before source edit; wait timed out twice; closed
开发角色       019febd2-d21a-7772-87f6-6c8d803fc423  called before source edit; wait timed out twice; closed
验证角色       019febd2-d267-7cc3-83ac-0285f3bf9bcb  called before source edit; wait timed out twice; closed
打包/流程角色  019febd2-d2b7-73f0-969d-10e7a2e242de  called before source edit; wait timed out twice; closed
独立质量复核   019febd7-1d96-79c0-a54e-44f1b1923ad9  called after implementation; wait timed out; closed
```

角色与独立复核没有返回完整报告，未被当作通过。父代理五轴审查确认：helper 只在非空、类型正确、可见 Protocol 页请求 activity；隐藏/最小化/其他页/关闭
不请求；没有新 timer、DTO、事件总线、输入面、依赖或常驻资源。简化结论是复用两个真实快照回调与唯一 MotionController 的 360ms 请求，保持
Component/Dataset worker、lifecycle、renderer throttle 和 surface owner 不变。

嵌入式 C/C++ 适用性：N/A。

## 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src          PASS
UI180_HIDDEN_HYDRATION_GUARD_PASS
UI180_VISIBLE_ACTIVITY_PASS duration=360
UI180_LIFECYCLE_GUARD_PASS hidden=minimized=other_tab
UI180_SHARED_CONTROLLER_PASS no_local_timer
UI180_DERIVED_ACTIVITY_VECTOR_PASS themes=default-composition hidden=static visible-probe=360
UI180_THEME_APPLY_PASS theme=star_trail
UI180_THEME_APPLY_PASS theme=moonlit_ocean
UI180_THEME_APPLY_PASS theme=sakura_night
UI180_THEME_VECTOR_PASS themes=3 main_window_hidden
```

验证使用真实 composition root 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；缺失 PySide6 虚拟字体目录的 Qt warning 已记录，未据此宣称真实
Windows 字体/HIDPI 通过。未运行完整 GUI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。

## 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.80` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.80`
- size：`47,890,929` bytes
- SHA-256：`E47A06C61064CC934B0812FEDCAD462AD61E3DFB664A3593C37705CEEE76365E`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
