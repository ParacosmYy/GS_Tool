# SerialForge UI-1 presentation 基础层交接

日期：2026-08-09  
范围：统一视觉 token、主窗口信息层级、稳定连接状态和低干扰装饰动效  
状态：in progress  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；当前未检测到 `.git`，未创建/操作 worktree

## 用户目标

持续美化 SerialForge UI 组件和动态效果，所有验收要点交付成功后再停止。

本轮用户/仓库限制：只在当前 checkout 工作；presentation-only；不启动 GUI/EXE/后台服务；不连接真实设备；不创建、修改或运行测试专用资产；二次元视觉候选尚未由用户选择。

## 已完成

- 新增 `src/serialforge/presentation/theme.py`：集中式深色工程控制台 token/QSS；不加载外置资源，当前静态背景即资源失败回退。
- 新增 `src/serialforge/presentation/widgets.py`：`StatusIndicator`，以 widget 自己持有低频 timer；支持生命周期色、RX 短脉冲、独立暂停、低动效、`SERIALFORGE_REDUCED_MOTION` 默认开关和显式 stop。
- `src/serialforge/presentation/main_window.py`：
  - 顶部增加稳定连接上下文、连接状态文字和装饰指示器；
  - 连接配置、协议/数据、历史/批量改为滚动分页，终端和发送栏保持主路径可见；
  - 增加 section 语义样式、错误/危险/主按钮层级、终端和暂停滚屏可访问描述；
  - 增加独立“低动效”和“暂停动效”控件；动画不会暂停终端、接收、记录或回放；
  - `Ctrl+Enter` 限定在发送输入控件上下文，避免配置表单误触发送；
  - 隐藏、最小化和关闭时停止装饰 timer。
- `src/serialforge/presentation/dataset_curve.py`：曲线网格、背景、实时/历史线色改用主题 token；保留有限点数、latest-wins 刷新和文字来源/最新值状态。
- `src/serialforge/presentation/command_batch_editor.py`：接入主题、错误语义和主按钮样式；未改批量命令的领域校验/执行。

## 六角色记录

| 角色 | agent id | 结果 | 父代理处理 |
|---|---|---|---|
| 产品 | `019fe681-683f-7fb2-a5a9-012e597c3baa` | revise | 产出 P0/P1 DoD；本轮关闭方向无关的基础层，保留视觉方向选择与 GUI 门。 |
| 架构 | `019fe681-6887-7931-aeaf-8d2e5a5158c1` | revise | 建议容器重组、独立 visual timer、资源无关；采用 QTabWidget/QScrollArea/StatusIndicator。 |
| UI 设计 | `019fe681-68ce-7081-a44f-dde9f4742866` | revise | 指出纵向堆叠、缺少稳定状态区和动效合同；采用状态文字、section role、低动效/暂停开关。 |
| 开发 | `019fe681-6916-70b2-8f33-6a32744171ea` | revise | 建议 theme/widget、分页、曲线 token、dialog 主题；未触碰 ViewModel 或业务层。 |
| 验证 | `019fe681-6956-7300-bbba-fc657deac724` | revise | 允许静态/导入门；GUI/offscreen/打包未授权；本轮不将它们写成通过。 |
| 打包/流程 | `019fe681-6992-7a30-b30c-efd09dcbf0a6` | revise | 确认无外置资源所以无需改 PyInstaller；未来加入资源必须重跑四矩阵。 |

六角色均为只读，父代理是本轮唯一写入者，使用 `luna_max / max / Fast`。

## 决策与简化

- 视觉方向：未选定二次元候选；本轮采用可替换、无资源的工程控制台基础层，不代表最终美术方向。
- 依赖方向：只改 presentation；`theme.py`/`widgets.py` 不导入 ViewModel、domain 或 transport。
- 动效：仅连接状态 indicator 的低频装饰脉冲；邻接文字表达真实状态，动效不是业务真值。
- reduced-motion：应用内“低动效”可随时切换，环境变量 `SERIALFORGE_REDUCED_MOTION=1/true/yes/on` 可默认打开；“暂停动效”独立存在，不复用“暂停滚屏”或回放暂停。
- 生命周期：timer parent 为 `StatusIndicator`；MainWindow 的 hide/minimize/close 路径停止 timer。
- 行为保持：保留所有 ViewModel 信号、连接/发送/记录/回放/协议/曲线数据语义；唯一快捷键调整是把 Ctrl+Enter 收窄到发送输入控件范围。
- 嵌入式 applicability：N/A；没有固件 C/C++ 变更，不能据此声称 MISRA/ISO/认证合规。

## 验证证据

| 命令/动作 | 结果 | 证据 |
|---|---|---|
| `.venv\Scripts\python.exe -B -m compileall -q src` | verified | `compileall: passed` |
| `.venv\Scripts\ruff.exe check src/serialforge/presentation` | verified | `All checks passed!` |
| `python -B` 导入新增/修改 presentation 模块 | verified | `presentation imports: passed` |
| PySide6 `QEvent.Type.WindowStateChange` 与 reduced-motion helper 导入 | verified | 枚举可用；当前环境变量返回 `False` |
| GUI/offscreen/EXE/打包/硬件 | not-run | 仓库约束未授权启动/打包/设备操作 |
| 测试专用资产 | not-run | 按约束未创建、修改或运行 |

未发现 presentation 对 `serial`、`bleak`、`socket` 的直接导入；当前目录没有 `.git`，没有创建/操作 worktree。

## 未完成与风险

1. 二次元视觉方向、背景资源和具体美术动效未实现，等待用户选择；不能把本轮基础层称为完整 UI-1。
2. 没有运行 GUI/offscreen，最小窗口、高 DPI、焦点/读屏、颜色对比、tab 路径、隐藏/最小化行为仍未验证。
3. 没有重打包，现有历史 `dist/release` 产物不代表本轮源码。
4. 未连接 UART/BLE/网络/J-Link，硬件和真实链路均为 not-run。

## 下一步

1. 用户选择一个二次元候选，记录色板、密度、动效强度、资源来源和静态回退。
2. 下一轮继续走六角色，只写 presentation/assets；把选定方向接入可替换 backdrop，不改变业务层。
3. 取得 GUI/offscreen 授权后验证 `980×680`、HIDPI、键盘/读屏、禁用/错误/空态和 timer 停止。
4. 取得打包授权后，用当前源码重跑 core/BLE × onedir/onefile provenance 矩阵。

## 交接结论

方向无关的 UI 基础层已经完成静态/导入验证；下一位协作者必须先读取根 `handoff.md`、`AGENTS.md`、`docs/WORKFLOW.md`，不能宣称二次元背景、GUI/EXE、硬件或正式发行通过。
