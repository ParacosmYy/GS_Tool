# ADR-0158：扩展能力详情按需展开

日期：2026-08-12  
状态：accepted  
范围：`presentation/embedded_extension_panel.py`

## 决策

扩展工具站首屏不自动展示第一张能力卡的详情。`ExtensionCapabilityDetail` 仍由扩展页
presentation owner 组合，但初始隐藏；用户首次通过 Tab 聚焦或点击能力卡时才显示，并同步当前卡片
的只读详情。这样首屏保留接入概览、分组标题和能力卡，不再重复展示 XMODEM。

`ExtensionCapabilityDetail` 继续只负责单一能力投影；能力选择仍由
`embedded_extension_panel.py` 的本地 presentation selection closure 管理。应用层 catalog、OTA
XMODEM/YMODEM/TFTP、AES-128-CCM/AES-256-GCM、RTT/J-Link attach-only/contract-only 边界不变，
不创建真实后端、密钥读取、vendor 工具、timer、thread 或设备 I/O。

## 交互与可访问性

- 首屏详情为隐藏状态，不产生空白卡片。
- Tab 聚焦任一能力卡会显示详情并更新选择状态。
- Enter/Space 点击能力卡会显示详情并更新对应能力。
- 卡片的 accessible description 继续说明“按 Tab 聚焦，按 Enter 或空格选择并查看详情”；详情的
  accessible name/description 继续由 `ExtensionCapabilityDetail.set_capability` 投影。

## 验证边界

真实 Qt offscreen 已覆盖 `980×720`、`1240×820`、`star_trail`、`moonlit_ocean`、`sakura_night`：
初始 `detail_visible=False`；第一张卡聚焦后显示 XMODEM；点击最后一张卡后显示 J-Link Telnet；
7 张卡 `overlap=False`，滚动提示保持 top，横向无新增滚动。EXE 启动、真实 Windows 可见窗口、
HIDPI、高负载、硬件连接和 OTA/RTT 实连仍未运行或未授权。

本轮无嵌入式 C/C++ 修改，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。

## 交付

`local-arch-107` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,017,630` bytes，SHA-256 为
`0264B14093A62DD296B3DF9C753BFFBD9FB66941D75C51227B17727098BFDDEC`。archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
