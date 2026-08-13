# ADR 0012：使用 presentation-only 自绘控件显示 Dataset 曲线

状态：已接受（M5d，2026-08-09）

## 背景

M5b 已把组件字段转换为 typed DatasetValue，M5c 已能把历史 RX 通过同一条派生链回放。
当前只有文本预览和 CSV，调试传感器时需要快速观察数值趋势；但引入图表库会增加依赖、
PyInstaller 收集和许可证边界，也容易把数据处理职责推入 UI。

## 决策

在 presentation 层增加 immutable curve projection 和 Qt Widgets-only `QPainter` 控件：

- `presentation/curve.py` 的 `build_curve_snapshot()` 只读取已有 `DatasetSample` DTO，不读取 port、锁、事件总线或 worker；
- 一次只绘制用户选择的一个 Dataset series，避免不同 unit 共用一条误导性 Y 轴；
- 仅接受有限 `int/float` typed value；bool、字符串/enum、None、缺失字段和 `DatasetValue.error` 计为 skipped，
  保留在现有文本 Dataset 预览，不变成 0；
- X 轴是当前 source 的 retained window 中相对 `occurred_at`，不暴露绝对 monotonic/wall time；
- curve point 上限为 512，且先受 Dataset 当前 `capacity` 限制；超过上限保留最新点并计 `truncated_points`；
- source 取 retained window 第一个 source，其余 source 不混入；历史 source 用已有 `DataOrigin.HISTORICAL` 标记；
- `DatasetCurveWidget` 只接收 `CurveSnapshot`，以 100 ms single-shot timer 合并重绘请求，绘制在 Qt 线程；
- Dataset signal 在一次 50 ms event drain 内 latest-wins 合并，降低文本/曲线重复刷新；
- `stop_replay()` 立即关闭 historical acceptance，迟到的历史 protocol/component/dataset preview 不再回填曲线，
  但已有曲线窗口保留；连接新的实时传输会清除旧历史 source gate。

## 非范围

M5d 不做多曲线、多 Y 轴、缩放/游标、重采样/聚合、告警、设备时间戳、数据库、回放筛选、
曲线导出、UDP/TCP Server/BLE/RTT Dataset 扩展，也不引入 QtCharts、pyqtgraph 或 WebEngine。

## 六角色只读复核

| 角色 | 子代理 run | 结论 |
|---|---|---|
| 产品 | `019fe476-b6fe-7b83-91dc-6e03af4f5f52` | pass（条件放行）；冻结单 series、relative elapsed、错误/空数据/历史状态和非范围 |
| 架构 | `019fe476-b745-75b1-80aa-23e9a94c0a92` | pass（条件放行）；presentation-only immutable snapshot + QPainter，不新增 domain/application port |
| UI 设计 | `019fe476-b787-7951-8f25-8854c3ef73cd` | pass（条件放行）；series 选择、文本等价状态、100–200ms coalescing 和可访问性 |
| 开发 | `019fe476-b7c2-7bb2-9f6b-b42ffca4b21d` | pass（条件放行）；最小文件为 curve/widget/main_window/qt，复用现有信号 |
| 验证 | `019fe476-b800-79a2-86a3-bac57531272f` | pass（条件放行）；要求有限值、容量、历史、刷新、offscreen、关闭和打包矩阵 |
| 打包/流程 | `019fe476-b83f-7142-8c00-d032aeca7ccf` | pass（条件放行）；无新增依赖/资源，提醒 onefile/许可证/CI 仍需门禁 |

六个子代理均为只读复核，已关闭；父代理负责唯一源码写入、整合和最终验证。

## 验证与风险

M5d 使用临时内存 DTO/曲线投影向量和 Qt offscreen `grab()` 验证，不创建测试专用资产。
必须记录静态检查、有限点过滤/容量/历史来源、曲线绘制、Dataset 回放、窗口关闭和最终
onedir/onefile 启动/WM_CLOSE。已知未决项是高吞吐压力、不同 DPI/字体环境、干净 Windows、
签名与第三方许可证归档；这些不能由 offscreen 证据替代。

## 简化评估

自绘控件把绘图职责收敛在 presentation，不改变现有 transport、protocol、component、Dataset
和 replay worker；曲线数据是有界 immutable projection，避免第二套数据处理 pipeline。现有
PySide6 已包含所需 QtGui 能力，无新增运行时依赖或资源收集规则。
