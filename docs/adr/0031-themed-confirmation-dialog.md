# ADR 0031：协议确认弹窗的主题化 presentation 边界

日期：2026-08-10

状态：已接受（UI-1.44）

## 背景

协议配置应用/重置前的 QMessageBox 已经使用暗色应用样式，但弹窗没有统一的确认 surface 角色、按钮文案和
辅助技术语义。确认操作会清空协议帧、Component 和 Dataset 派生缓存，用户必须在不改变既有协议行为的情况下
清楚区分“确认清理”和“取消保留”。

## 决策

- `controllers/protocol_config.py:confirm_protocol_change()` 继续拥有确认触发条件、影响范围文案和 `exec()` 返回值；
  不把 ViewModel、协议 worker 或 transport 状态复制到 presentation helper。
- 新增 `presentation/dialog_surface.py:configure_confirmation_dialog()`，只配置 QMessageBox 的
  `surfaceRole="confirmation"`、标准 Ok/Cancel 按钮文案、`dangerButton` 确认语义和
  AccessibleName/AccessibleDescription。
- 默认 controls stylesheet 与 `theme_variant_controls.py` 都覆盖
  `QMessageBox[surfaceRole="confirmation"]`、信息文本和按钮尺寸；颜色只消费现有 warning/surface/text 语义 token。
- 默认按钮继续是 Cancel；确认按钮不改变 QMessageBox 的 StandardButton role，因此调用方的 `exec()` 判定保持不变。
- 不新增弹窗专用动画、timer、自绘 popup、外部资源或设备/协议依赖。

## 放弃的选项

- 不在 `MainWindow` 增加弹窗 facade，避免继续扩大窗口 shell。
- 不用全局 `QMessageBox` 样式覆盖所有信息/错误弹窗，避免把危险确认语义扩散到普通提示。
- 不关闭原生标准按钮和键盘焦点，也不改成自绘对话框；这会增加跨 Windows Qt style 的风险。
- 不在本切片修正“仅有未完成帧是否需要确认”或“重置协议”业务文案覆盖范围；这些属于后续协议状态切片。

## 依赖与生命周期

```text
protocol_config.confirm_protocol_change
            ↓ protocol-specific copy + exec result
dialog_surface.configure_confirmation_dialog
            ↓ presentation-only Qt metadata
Qt QMessageBox + theme QSS
```

helper 不持有窗口、ViewModel、worker、timer 或 session；对话框 parent 和生命周期仍由当前 controller/Qt modal flow
管理。动态 `surfaceRole` 使用现有 `refresh_dynamic_property()` 触发 QSS re-polish。

## 验证

```text
scripts/check.ps1                         PASS 127 files <= 1000; 3 themes; 22 tokens; 13 selectors; Ruff
UI144_IMPORT/COMPILE                     PASS 126 modules; compileall
UI144_CONFIRM                             PASS 3 themes; surfaceRole; default_cancel=True; standard button text;
                                              AccessibleName/Description; near_white=0
```

离屏环境报告缺少 PySide6 fonts directory；中文方框不代表 Windows 字体结论。已生成
`build/ui_review_ui144_confirmation_star_trail.png`、`..._moonlit_ocean.png` 和 `..._sakura_night.png`。

## 复核与限制

- 架构师角色已调用；两次等待超时，父级完成边界审查并保持最小 diff。
- 独立只读复核确认 helper 职责清晰、未引入业务/设备/线程耦合；建议保留现有复用，不再继续抽象。
- 简化评估：只抽取稳定的 presentation 配置重复点，没有为减少行数改变消息框行为；未修改测试专用资产。
- 未运行持续 GUI、Windows 原生 Qt style、真实键盘/读屏/HIDPI、EXE 启动、硬件、OTA 或 RTT/J-Link 验收。
- 嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用厂商要求；不声称
  MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

## 后续

协议状态切片应单独决定未完成 parser buffer/仅统计状态的确认门，并校准“重置”与 editor 实际行为的文案；不能由
本 helper 隐式扩大确认范围。
