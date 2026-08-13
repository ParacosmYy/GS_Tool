# ADR 0080：嵌入式扩展能力展示页

日期：2026-08-10

状态：已接受

## 背景

SerialForge 已经为 OTA、OTA security、RTT/J-Link 建立独立 contract-only/attach-only 目录，但如果用户看不到这些边界，后续很容易把“预留槽位”误解成已支持的刷写、加密或探针控制能力。同时，把这些状态硬编码进 `MainWindow` 会让未来实现与 UI 形成反向依赖。

## 决策

- application 新增 `extension_capabilities.py`，只提供不可变、有限、带 group/reference/state 的能力 DTO 和静态 catalog；它不打开文件、网络、设备、密钥或 vendor 工具。
- presentation 新增 `embedded_extension_panel.py`，只渲染 application DTO；卡片明确展示“契约预留”或“仅附着”，不提供启动、传输、验证、激活和探针动作。
- 工作区新增第四个“扩展 / 工具站”Tab；原生 Tab、焦点和键盘导航仍由 `QTabWidget` 拥有，route beacon 与 theme-aware icon 只同步 bounded index/ThemeSpec。
- XMODEM/YMODEM/TFTP、AES-256-GCM/AES-128-CCM、RTT/J-Link 的实现和授权边界仍由 `ota/*`/`debug/*` contract owner 管理，presentation 不导入 adapter、socket、pyserial、SDK/DLL 或密钥。

## 结果

用户可以在独立工具站页面看到后续扩展的准确状态和前置条件；未来实现只需替换 application catalog/projection，不必把协议、安全或 vendor 细节塞进主窗口。三主题、四节点路线和第四个无障碍 Tab 通过 offscreen vector；真实 OTA、加密、J-Link、GUI/HIDPI 和硬件验收仍未执行。

