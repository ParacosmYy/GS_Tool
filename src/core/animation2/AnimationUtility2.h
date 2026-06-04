/**
 * @file AnimationUtility2.h
 * @brief 动画工具类，提供预设动画和属性动画的便捷接口
 */
#pragma once
#include <QObject>
#include <QEasingCurve>
#include <QMap>
#include <QString>
#include <functional>

class QVariantAnimation;
class QWidget;

/**
 * @class AnimationUtility
 * @brief 动画管理工具，封装QVariantAnimation提供淡入/淡出/滑动/缩放等预设动画效果
 */
class AnimationUtility : public QObject {
    Q_OBJECT
public:
    /** @brief 预设动画类型枚举 */
    enum Preset { FadeIn, FadeOut, SlideLeft, SlideRight, SlideUp, SlideDown, ScaleIn, ScaleOut, Bounce };
    Q_ENUM(Preset)

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit AnimationUtility(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~AnimationUtility() override;

    /** @brief 对目标控件执行预设动画 @param target 目标控件 @param preset 预设类型 @param durationMs 持续时间(毫秒) */
    void animate(QWidget *target, Preset preset, int durationMs = 300);
    /** @brief 对目标对象的指定属性执行动画 @param target 目标对象 @param property 属性名 @param from 起始值 @param to 结束值 @param durationMs 持续时间 @param curve 缓动曲线 */
    void animateProperty(QObject *target, const QString &property, const QVariant &from, const QVariant &to, int durationMs = 300, const QEasingCurve &curve = QEasingCurve::OutCubic);
    /** @brief 停止所有正在执行的动画 */
    void stopAll();
    /** @brief 停止指定目标对象上的动画 @param target 目标对象 */
    void stopForTarget(QObject *target);
    /** @brief 查询目标对象是否正在动画中 @param target 目标对象 @return 是否有动画在执行 */
    bool isAnimating(QObject *target) const;
    /** @brief 设置默认动画时长 @param ms 毫秒数 */
    void setDefaultDuration(int ms);
    /** @brief 设置默认缓动曲线 @param curve 缓动曲线 */
    void setDefaultCurve(const QEasingCurve &curve);

    // ---- 统计计数器 ----
    /** @brief 获取累计启动的动画总次数 @return 动画启动次数 */
    quint64 totalAnimationsStarted() const { return m_totalAnimationsStarted; }
    /** @brief 获取累计完成的动画总次数 @return 动画完成次数 */
    quint64 totalAnimationsFinished() const { return m_totalAnimationsFinished; }
    /** @brief 获取累计手动停止的动画总次数 @return 动画停止次数 */
    quint64 totalAnimationsStopped() const { return m_totalAnimationsStopped; }
    /** @brief 重置所有统计计数器 */
    void resetAnimationStatistics() { m_totalAnimationsStarted = 0; m_totalAnimationsFinished = 0; m_totalAnimationsStopped = 0; }

signals:
    /** @brief 动画完成时发射 @param target 目标对象 */
    void animationFinished(QObject *target);
    /** @brief 动画开始时发射 @param target 目标对象 */
    void animationStarted(QObject *target);

private:
    /** @brief 动画完成的内部回调 */
    void onFinished();

    QMap<QObject *, QVariantAnimation *> m_animations;  ///< 目标对象到动画实例的映射
    int m_defaultDuration = 300;                         ///< 默认动画时长(毫秒)
    QEasingCurve m_defaultCurve{QEasingCurve::OutCubic}; ///< 默认缓动曲线

    // ---- 统计 ----
    quint64 m_totalAnimationsStarted = 0;   ///< 统计: 累计动画启动次数
    quint64 m_totalAnimationsFinished = 0;  ///< 统计: 累计动画完成次数
    quint64 m_totalAnimationsStopped = 0;   ///< 统计: 累计手动停止次数
};
