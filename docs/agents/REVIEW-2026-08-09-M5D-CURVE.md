# M5d 六角色复核记录：有界 Dataset 曲线监视

日期：2026-08-09  
范围：Dataset 数值 series 选择、历史/实时来源标识、相对时间曲线、有限容量、异常值过滤、
Qt offscreen 绘制、PyInstaller 交付。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议 |
|---|---|---|---|
| 产品 | `019fe476-b6fe-7b83-91dc-6e03af4f5f52` | pass conditionally → 已整合 | 单页先支持一个数值 series；历史/实时必须明确区分；错误、字符串、bool 不进入曲线；回放停止后的迟到事件必须隔离 |
| 架构 | `019fe476-b745-75b1-80aa-23e9a94c0a92` | pass conditionally → 已整合 | 曲线保持 presentation-only immutable snapshot；不向 domain/application 引入图表依赖；固定 512 点上限和来源隔离 |
| UI 设计 | `019fe476-b787-7951-8f25-8854c3ef73cd` | pass conditionally → 已整合 | 提供 series 选择、空态/错误/历史文案；使用 100 ms 合并刷新，避免高频重绘；保留可访问名称和描述 |
| 开发 | `019fe476-b7c2-7bb2-9f6b-b42ffca4b21d` | pass conditionally → 已整合 | 只新增 `curve.py`、`dataset_curve.py` 和必要的 Qt/MainWindow/ViewModel 接线；复用现有 Dataset 信号，不新建 ViewModel |
| 验证 | `019fe476-b800-79a2-86a3-bac57531272f` | pass conditionally → 已整合 | 覆盖有限容量、finite/error/bool/string 过滤、历史来源、offscreen 绘制、刷新/关闭和双模式打包 |
| 打包/流程 | `019fe476-b83f-7142-8c00-d032aeca7ccf` | pass conditionally → 已整合 | 不新增运行时依赖或资源；默认包继续不带 BLE/J-Link/RTT 可选运行时；保留许可证、签名和 clean Windows 交付门 |

六个子代理已关闭；父代理负责整合、最终源码检查和验证。

## 变更摘要

- `presentation/curve.py` 将有限的 `DatasetSample` 序列投影为 immutable `CurveSnapshot`；只选择一个精确 `ProtocolSource`，
  以第一条保留样本为时间零点，并限制为最多 512 个点；
- 曲线只接受有限实数，跳过字段错误、bool、字符串、非有限值和负 elapsed；保留 sample、skipped、truncated 计数供 UI 解释；
- `presentation/dataset_curve.py` 使用 Qt Widgets/QPainter 自绘网格、坐标轴、折线和采样点，历史数据使用独立颜色，
  不引入 QtCharts、WebEngine 或其他图表运行时；
- Dataset 面板增加数值 series 选择、曲线状态和历史/实时提示；默认不替用户猜测 series；
- ViewModel 将 Dataset 批量事件合并为最新状态后再发 UI 信号，并在 replay 生命周期中严格校验 replay UUID/source，
  避免停止后的迟到数据刷新当前视图；
- PyInstaller onedir/onefile 均复用同一应用入口和默认依赖集合。

## 复核发现与处理

1. 曲线初版若默认选第一个字段，可能把非数值字段误当成可视化目标；已改为显式“选择数值 series”，并只列出可用数值字段。
2. Dataset 事件可能高频到达；已在 ViewModel 的事件 drain 和 widget 的 100 ms 单次定时器两层做 latest-wins 合并，
   曲线仍由 immutable snapshot 驱动。
3. 历史回放停止后仍可能有异步事件抵达；已在 replay start/stop 和 source gate 中清除/校验 replay UUID，拒绝陈旧事件。
4. 点数和字段值均有边界：曲线最多 512 点，Dataset sample capacity 仍受既有上限约束，绘制不会按输入无限增长。

## 验证证据

已运行：

- `uv run --locked ruff format --check --no-cache src`：通过，44 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `scripts/check.ps1`：通过；
- inline vectors：finite/error/bool/string/非有限值过滤、历史 source、相对 elapsed、512 点容量和 immutable DTO：通过；
- Qt `QT_QPA_PLATFORM=offscreen`：曲线 widget 绘制、series 选择、历史回放、Dataset stats、窗口关闭和 worker shutdown：通过；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit：通过，PID `47064`；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit：通过，bootstrap `88052`、GUI `59048`；
- 默认 onedir 文件名扫描中的 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs` 匹配数：`0`；
- 验证结束时没有残留由本轮启动的 `SerialForge` EXE 进程。

最终产物：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 2,996,952 B | `94A0B665E50E373266B14D7AFB6266E8264B628DA0F8FD7F0AB2B76EC223A6FA` |
| `dist/SerialForge.exe` | 47,491,089 B | `6E78E5C499CB890B100A0BE5888A39662BD803F101454169CB69182DAECB3612` |

## 未运行/未宣称

- 未运行真实 UART/TCP 长时间吞吐、损坏文件压力、干净 Windows、DPI/字体覆盖、代码签名和第三方许可证发行审查；
- 未运行真实 BLE 扫描/配对/通知、J-Link/目标板和任何驱动/探针操作；J-Link RTT 继续保持最后能力；
- 未纳入多 series 同时绘制、曲线缩放/游标、数据导出、持久化图表配置和跨 source 对比；
- 未创建或修改单元测试、mock、fixture、test harness 或其他 test-only asset；
- 当前任务没有嵌入式 C/C++、固件、MCU、BSP/HAL/RTOS 或硬件源代码变更，因此没有新的厂商固件要求适用性声明。

## 简化评估

通过 presentation-only 的 `CurveSnapshot` 和 QPainter widget 复用既有 Dataset 数据链，
没有引入图表框架、数据库、插件运行时、动态代码执行或新的跨层端口。固定点数、字段类型过滤、source 隔离和 latest-wins 刷新
保留了可解释性与资源边界，同时让后续多 series、缩放和导出可以在 presentation 层增量扩展。

