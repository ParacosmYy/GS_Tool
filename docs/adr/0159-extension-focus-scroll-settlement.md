# ADR-0159：扩展能力焦点滚动结算

日期：2026-08-12  
状态：accepted  
范围：`presentation/embedded_extension_panel.py`

## 问题

扩展页详情按需显示后，窄窗口首次焦点事件会改变 scroll content 的最终高度。若只调用一次原生
`ensureWidgetVisible`，首张能力卡可能仍有约 5px 位于 viewport 外；键盘焦点存在但用户看不到它。

## 决策

由扩展页组合 owner 在现有 focus/click 路径中同步结算：

1. 对现有 `QScrollArea.widget()` 调用 `adjustSize()`，并激活其 layout；
2. 调用原生 `ensureWidgetVisible(card, 12, 12)`；
3. 读取卡片映射到 viewport 的真实 top/bottom，仅在超出 12px安全边界时同步调整 vertical scrollbar。

这段逻辑只处理 presentation 几何，不创建新的 scroll owner、timer、singleShot、事件循环、业务
状态、后端或设备 I/O。能力卡仍只负责发出 focus/click 选择信号；应用层 OTA、AES、RTT/J-Link
catalog 和 contract-only/attach-only 边界不变。

## 验证边界

真实 Qt offscreen 已覆盖 `star_trail`、`moonlit_ocean`、`sakura_night` × `980×720`、`1240×820`：
7 张能力卡逐一 Tab 聚焦后均 `visible=True` 且 `focus=True`；980px 首卡为 `319..462`（viewport
height `475`），1240px 首卡为 `408..551`（viewport height `563`）；横向 scrollbar maximum
始终为 `0`，scroll hint 中段状态正确。EXE 启动、真实 Windows 可见窗口、HIDPI、高负载、硬件连接
和 OTA/RTT 实连仍未运行或未授权。

本轮无嵌入式 C/C++ 修改，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。

## 交付

`local-arch-108` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,020,617` bytes，SHA-256 为
`2300FFD431657EA96E67A43F1470199671883CAC9AA5FC711F0CA1AB6330A217`。archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
