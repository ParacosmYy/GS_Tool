# FEATURE: 高级波形引擎

> 优先级: P0 | 状态: 已通过评审 (PRD-059) | 参考: VOFA+, Serial Studio
> 目标: 超越VOFA+的波形体验

---

## 一、动机

当前 ChartWidget 仅支持基础实时波形显示（QLineSeries + 滑动窗口）。对比 VOFA+ 和 Serial Studio，缺失以下关键能力:

1. **游标测量** — 无法精确读取波形上某个点的数值
2. **缩放平移** — 无法查看数据细节或全局趋势
3. **多Y轴** — 不同量级/单位的通道挤在同一Y轴上
4. **FFT频谱分析** — 无法观察信号的频域特征

## 二、分阶段计划（4个迭代）

### Phase 1: 游标测量 + 缩放平移 (commit #81-#82, ~600行)
- 新增 `CursorOverlay` 绘制层: 双竖线游标 + 差值显示
- 鼠标滚轮缩放 + 中键拖拽平移
- 缩放时自动切换降采样策略（概览→细节）

### Phase 2: 多Y轴支持 (commit #83, ~400行)
- 每个通道可绑定独立Y轴
- Y轴颜色与通道颜色一致
- Y轴可左右两侧分布

### Phase 3: FFT频谱分析 (commit #84, ~500行)
- 新增 `FftEngine` 基于KissFFT或自实现Cooley-Tukey
- FFT窗口类型选择: 矩形/Hanning/Hamming/Blackman
- 频谱图以独立Tab或叠加层展示

### Phase 4: 直方统计 + X/Y Scatter (commit #85, ~300行)
- 通道数据直方图（分布统计）
- X/Y散点模式（两个通道互为X/Y轴）

---

## 三、架构设计

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `CursorOverlay` | `chart/CursorOverlay.h/cpp` | 表现层 | 游标绘制+差值计算 |
| `ZoomController` | `chart/ZoomController.h/cpp` | 数据层 | 缩放/平移状态管理 |
| `FftEngine` | `chart/FftEngine.h/cpp` | 数据层 | FFT计算引擎 |
| `FftWidget` | `chart/FftWidget.h/cpp` | 表现层 | FFT频谱显示控件 |

### 设计模式
- **策略模式**: ZoomController 管理不同的缩放策略
- **观察者模式**: CursorOverlay 监听 ChartModel 数据变化

### 依赖
- ChartModel（已有）— 数据源
- ChartWidget（已有）— 渲染层
- ChartColors（已有）— 配色

---

## 四、Phase 1 详细需求: 游标测量 + 缩放平移

### R1: CursorOverlay 游标绘制
- 在 ChartWidget 上叠加一层透明绘制层
- 双击放置游标A（红色竖线），右键放置游标B（蓝色竖线）
- 游标间区域半透明高亮
- 顶部显示: ΔX (采样点差), ΔY (每个通道的值差), 1/ΔX (频率估算)

### R2: 鼠标缩放
- 滚轮向上: 以鼠标位置为中心放大X轴
- 滚轮向下: 缩小恢复
- Ctrl+滚轮: 缩放Y轴
- 双击: 重置到默认视图

### R3: 平移
- 中键拖拽: 水平平移波形
- Shift+中键: 垂直平移

### R4: 缩放时降采样切换
- 概览模式（>1000点可见）: 降采样到500点
- 细节模式（<1000点可见）: 显示全量数据
- 过渡时平滑切换，不闪烁

### 验收标准
1. 游标A/B可见，差值信息清晰显示
2. 滚轮缩放平滑无卡顿
3. 平移流畅，无延迟感
4. 缩放到最细节时波形不丢失精度
5. 所有新增代码 ≤ 500行/文件, ≤ 80行/方法
6. 编译零错误
