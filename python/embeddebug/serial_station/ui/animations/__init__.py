"""UI 动画引擎：缩放、滑入滑出、折叠展开、菜单隐藏、页面切换过渡。

提供可复用的动画工厂与控制器，统一管理 EmbedDebug 全应用的动态视觉效果。

公共导出：
    AnimationTokens     — 动画时长/缓动曲线 token 集合
    ScaleAnimation      — 点击缩放回弹（按钮按压反馈）
    SlideAnimation      — 从指定方向滑入/滑出
    CollapseAnimation   — 高度折叠/展开（菜单隐藏）
    FadeTransition      — 页面/面板淡入淡出切换
    ShakeAnimation      — 错误抖动反馈
    PulseAnimation      — 脉冲呼吸效果（状态指示）
    AnimationController — 统一管理多个动画的生命周期
    BouncePathAnimation — 带位移的路径动画（下落、滑入）
    GlowAnimation       — 辉光脉冲（连接成功反馈）
    TypewriterAnimation — 打字机效果（状态栏文本）
    ElasticSnapAnimation — 弹性吸附（拖拽磁吸）
"""

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.animations.scale import ScaleAnimation
from embeddebug.serial_station.ui.animations.slide import SlideAnimation, SlideDirection
from embeddebug.serial_station.ui.animations.collapse import CollapseAnimation
from embeddebug.serial_station.ui.animations.fade import FadeTransition
from embeddebug.serial_station.ui.animations.shake import ShakeAnimation
from embeddebug.serial_station.ui.animations.pulse import PulseAnimation
from embeddebug.serial_station.ui.animations.controller import AnimationController
from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
from embeddebug.serial_station.ui.animations.elevation import elevation_effect
from embeddebug.serial_station.ui.animations.glow import GlowAnimation
from embeddebug.serial_station.ui.animations.typewriter import TypewriterAnimation
from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation

__all__ = [
    "AnimationController",
    "AnimationTokens",
    "BouncePathAnimation",
    "CollapseAnimation",
    "ElasticSnapAnimation",
    "elevation_effect",
    "FadeTransition",
    "GlowAnimation",
    "PulseAnimation",
    "ScaleAnimation",
    "ShakeAnimation",
    "SlideAnimation",
    "SlideDirection",
    "TypewriterAnimation",
]
