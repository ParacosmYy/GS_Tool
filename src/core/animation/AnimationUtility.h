/**
 * @file AnimationUtility.h
 * @brief 动画工具集 — 提供常用UI动画效果的静态工厂方法
 *
 * 统一管理项目中所有动画参数和效果，避免各控件重复创建动画代码。
 * 提供淡入淡出、滑动、缩放、弹性弹出、抖动等预设效果。
 *
 * 所有方法返回 QPropertyAnimation* (或启动动画组)，动画对象设置
 * DeleteWhenStopped 自动清理。
 *
 * 线程安全: s_durationSumMs/s_durationCount 使用 std::atomic 保护，
 * 防止多个动画并发完成时统计计数器数据竞争。
 *
 * 用法:
 *   AnimationUtility::fadeIn(widget, 200);
 *   AnimationUtility::slideIn(widget, SlideDirection::FromRight, 300);
 *   AnimationUtility::shake(widget, 8, 3);
 */

#ifndef ANIMATION_UTILITY_H
#define ANIMATION_UTILITY_H

#include <QEasingCurve>
#include <QPoint>
#include <functional>
#include <atomic>

class QWidget;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

/**
 * @brief 滑动方向
 */
enum class SlideDirection {
    FromLeft,       ///< 从左侧滑入
    FromRight,      ///< 从右侧滑入
    FromTop,        ///< 从上方滑入
    FromBottom      ///< 从下方滑入
};

/**
 * @brief 动画工具集(静态工厂方法)
 *
 * 所有方法返回创建的 QPropertyAnimation*，调用者可进一步定制。
 * 动画对象设置 DeleteWhenStopped，自动清理。
 */
class AnimationUtility {
public:
    /// 默认动画时长(毫秒)
    static constexpr int kDefaultDuration = 200;
    static constexpr int kSlowDuration = 350;
    static constexpr int kFastDuration = 120;

    /**
     * @brief 动画统计计数器
     *
     * 跟踪动画创建、完成、取消、缓动变更和平均时长等运行指标，
     * 用于性能分析和调试。所有计数器在 resetStatistics() 调用时归零。
     */
    struct Stats {
        quint64 totalAnimationsCreated = 0;    ///< 累计创建的动画总数量
        quint64 totalAnimationsCompleted = 0;  ///< 累计正常完成的动画总数量
        quint64 totalAnimationsCancelled = 0;  ///< 累计被停止/取消的动画总数量
        quint64 totalEasingChanges = 0;        ///< 累计缓动曲线变更次数
        double avgDurationMs = 0.0;            ///< 加权平均动画时长(毫秒)
    };

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    static const Stats& stats();

    /** @brief 重置所有统计计数器为零，avgDurationMs 归零 */
    static void resetStatistics();

    /**
     * @brief 淡入效果
     * @param widget 目标控件
     * @param durationMs 时长
     * @param curve 缓动曲线
     * @return 动画对象(已启动)
     */
    static QPropertyAnimation* fadeIn(QWidget* widget,
                                       int durationMs = kDefaultDuration,
                                       QEasingCurve curve = QEasingCurve::OutCubic);

    /**
     * @brief 淡出效果
     * @param widget 目标控件
     * @param durationMs 时长
     * @param onFinished 结束回调
     * @return 动画对象(已启动)
     */
    static QPropertyAnimation* fadeOut(QWidget* widget,
                                        int durationMs = kDefaultDuration,
                                        QEasingCurve curve = QEasingCurve::InCubic,
                                        std::function<void()> onFinished = nullptr);

    /**
     * @brief 滑入效果
     * @param widget 目标控件
     * @param direction 滑动方向
     * @param durationMs 时长
     * @param curve 缓动曲线
     * @return 动画对象(已启动)
     */
    static QPropertyAnimation* slideIn(QWidget* widget,
                                        SlideDirection direction,
                                        int durationMs = kSlowDuration,
                                        QEasingCurve curve = QEasingCurve::OutCubic);

    /**
     * @brief 滑出效果
     * @param widget 目标控件
     * @param direction 滑动方向
     * @param durationMs 时长
     * @param onFinished 结束回调
     * @return 动画对象(已启动)
     */
    static QPropertyAnimation* slideOut(QWidget* widget,
                                         SlideDirection direction,
                                         int durationMs = kSlowDuration,
                                         QEasingCurve curve = QEasingCurve::InCubic,
                                         std::function<void()> onFinished = nullptr);

    /**
     * @brief 缩放弹入效果(从小变大+淡入)
     * @param widget 目标控件
     * @param durationMs 时长
     * @return 动画组(已启动)
     */
    static void scaleIn(QWidget* widget,
                         int durationMs = kSlowDuration);

    /**
     * @brief 弹性滑入(带回弹效果)
     * @param widget 目标控件
     * @param direction 方向
     * @param durationMs 时长
     * @return 动画对象(已启动)
     */
    static QPropertyAnimation* bounceIn(QWidget* widget,
                                          SlideDirection direction,
                                          int durationMs = 400);

    /**
     * @brief 抖动效果(用于错误提示)
     * @param widget 目标控件
     * @param amplitude 抖动幅度(px)
     * @param count 抖动次数
     */
    static void shake(QWidget* widget, int amplitude = 8, int count = 3);

    /**
     * @brief 为控件设置透明度效果(如果不存在则创建)
     * @param widget 目标控件
     * @return 透明度效果对象
     */
    static QGraphicsOpacityEffect* ensureOpacityEffect(QWidget* widget);

private:
    /// 获取滑动偏移量
    static QPoint slideOffset(SlideDirection direction, const QWidget* widget);

    static Stats s_stats;                           ///< 全局统计实例(静态存储)
    static std::atomic<quint64> s_durationSumMs;    ///< 累计动画时长总和(原子操作)
    static std::atomic<quint64> s_durationCount;    ///< 累计已完成动画计数(原子操作)
};

#endif // ANIMATION_UTILITY_H
