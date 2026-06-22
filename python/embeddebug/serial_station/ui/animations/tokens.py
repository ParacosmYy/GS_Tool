"""动画 token：统一时长与缓动曲线，全应用一致。

本模块是整个 EmbedDebug UI 动画系统的「节奏真相源」——所有动画工厂
（``animations/*``、``panel_animations.py``、``micro_interactions.py``）
都必须引用本类的常量，禁止在别处再定义并行的时间常量，避免双轨制导致
节奏割裂（参见 05-ui-standard §六 动画应服务于信息传达）。

设计要点：
- 时长分 5 档（instant/fast/normal/slow/slower），覆盖瞬时反馈到面板入场。
- 缓动曲线以 OutCubic 为基调（自然减速），按压回弹用 OutBack，弹跳用 OutBounce。
- 抬升 / 阴影 / 抖动幅度作为常量集中管理，便于全应用一致。
"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve


class AnimationTokens:
    """动画时长（毫秒）与缓动曲线 token。

    所有动画工厂引用本类的常量，保证视觉节奏统一。
    """

    # === 时长档位（毫秒）===
    DURATION_INSTANT = 100   # 瞬时反馈：状态色切换、tooltip
    DURATION_FAST = 160      # 快速反馈：按钮按压、hover 阴影
    DURATION_NORMAL = 240    # 常规过渡：面板淡入、滑入、折叠
    DURATION_CONTAINER = 300  # 容器变换：页面/卡片容器 morph（Material 3 emphasized 300ms）
    DURATION_SLOW = 360      # 较慢过渡：页面切换组合动画
    DURATION_SLOWER = 600    # 慢速：呼吸灯、加载脉冲

    # === 缓动曲线 ===
    EASE_OUT = QEasingCurve.Type.OutCubic       # 自然减速（入场默认）
    EASE_IN = QEasingCurve.Type.InCubic         # 自然加速（离场默认）
    EASE_IN_OUT = QEasingCurve.Type.InOutCubic  # 对称（呼吸、抖动）
    EASE_OUT_BACK = QEasingCurve.Type.OutBack   # 轻微过冲（按压回弹）
    EASE_OUT_BOUNCE = QEasingCurve.Type.OutBounce  # 弹跳（通知）
    EASE_OUT_ELASTIC = QEasingCurve.Type.OutElastic  # 弹性（少用）
    EASE_OUT_QUART = QEasingCurve.Type.OutQuart  # 强调减速（页面入场、容器变换）
    EASE_IN_QUART = QEasingCurve.Type.InQuart    # 强调加速（页面离场）
    EASE_OUT_QUINT = QEasingCurve.Type.OutQuint  # Material 3 emphasized ≈ cubic-bezier(0.2, 0, 0, 1.0)
    EASE_OUT_QUAD = QEasingCurve.Type.OutQuad   # 轻减速（Fluent 风格，比 OutCubic 更柔）
    LINEAR = QEasingCurve.Type.Linear           # 仅进度条/匀速场景

    # === 语义化时长别名（Fluent Design 对标）===
    DURATION_PROGRESS = 150  # 进度条数值变化（Fluent 标准）
    DURATION_FLYOUT = 187    # 浮出/flyout（Fluent 标准，≈ FAST 但语义独立可调）
    DURATION_DRAWER = 300    # 抽屉/侧栏开关（= CONTAINER 别名，保留两者）
    DURATION_SCROLL = 500    # 滚动条平滑滚动（Fluent 标准）

    # === Elevation 层级（对标 Material 3 L0-L5）===
    # 每级是 (blur_radius, offset_y, alpha) 三元组，由 elevation_effect() 解析。
    ELEVATION_L0 = (0, 0, 0)       # resting flat：无阴影
    ELEVATION_L1 = (8, 1, 60)      # 卡片静态 / 曲线 glow
    ELEVATION_L2 = (12, 3, 80)     # 抽屉展开 / 折叠面板
    ELEVATION_L3 = (16, 3, 120)    # hover / focus（默认交互态）
    ELEVATION_L4 = (24, 4, 160)    # popover / toast / flyout
    ELEVATION_L5 = (32, 6, 200)    # modal / dragged

    # === 曲线 glow 专用 blur（waveform_preview 用）===
    SHADOW_BLUR_CURVE_GLOW = 8  # 波形曲线发光（= ELEVATION_L1[0]）

    # === 关键帧位置（choreography 用）===
    KEYFRAME_PREVIEW = 0.3   # 预览位置：复杂动画在 30% 时刻达到「可识别」状态
    KEYFRAME_HERALED = 0.5   # hero 元素到中点
    STAGGER_STEP_MS = 60     # stagger 默认步长（Linear/Vercel 风格）

    # === 缩放比例（按压/弹入用）===
    # 统一按压缩小到 0.96（分离式 press_down/up 的克制值，合并式 press() 也用此值）。
    SCALE_PRESSED = 0.96   # 按压缩小比例（分离式与合并式共用，消除双轨制）
    SCALE_POP_IN = 0.6     # 弹入起始比例
    SCALE_BOUNCE = 1.08    # 弹跳峰值比例
    SCALE_HOVER = 1.03     # hover 轻微放大（hover_in 方法用）
    SCALE_NAV_HOVER = 1.12  # NavRail 图标 hover 放大（主导航元素需更强反馈）
    SCALE_NORMAL = 1.0

    # === drop_in 弹跳落地参数（BouncePathAnimation 用）===
    DROP_VERTICAL_OFFSET = -50  # 起始上偏移：从上方下落
    SQUASH_HEIGHT_RATIO = 0.92  # 落地挤压：高度收缩 8%
    SQUASH_WIDTH_RATIO = 1.04   # 落地挤压：宽度膨胀 4%（面积守恒直觉）

    # === hover 抬升（像素）===
    LIFT_PIXELS = 3        # hover 时垂直位移
    SHADOW_BLUR_NORMAL = 16
    SHADOW_BLUR_HOVER = 32
    SHADOW_BLUR_FOCUS = 18
    SHADOW_OFFSET_Y = 3    # 静态阴影 Y 偏移

    # === 抖动 ===
    SHAKE_AMPLITUDE = 8    # 抖动振幅
    SHAKE_COUNT = 3        # 抖动往返次数

    # === 折叠 ===
    COLLAPSED_HEIGHT = 0

    # === 阴影色（hover accent tint）===
    # 黑色阴影保持深度，accent 色阴影用于 hover/active 强调。
    SHADOW_COLOR_RGB = (0, 0, 0, 120)
    SHADOW_COLOR_HOVER_ALPHA = 180
