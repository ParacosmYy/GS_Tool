/**
 * @file AnimationUtility2.cpp
 * @brief 动画工具类实现 - 提供预设动画和属性动画的便捷接口
 */

#include "core/animation2/AnimationUtility2.h"
#include <QVariantAnimation>
#include <QWidget>

/** @brief 构造动画工具类 @param parent 父对象指针 */
AnimationUtility::AnimationUtility(QObject *parent) : QObject(parent) {}

/** @brief 析构函数，停止所有活跃动画释放资源 */
AnimationUtility::~AnimationUtility() { stopAll(); }

/**
 * @brief 使用预设动画类型对目标控件执行动画
 * @param target 目标控件指针，为空时直接返回
 * @param preset 预设动画类型枚举(淡入/淡出/滑动/缩放/弹跳)
 * @param dur 动画持续时间(毫秒)，小于等于0时使用默认时长
 */
void AnimationUtility::animate(QWidget *target, Preset preset, int dur) {
    if (!target) return;
    QEasingCurve curve;
    switch (preset) {
    case FadeIn: animateProperty(target, "windowOpacity", 0.0, 1.0, dur, QEasingCurve::OutCubic); return;
    case FadeOut: animateProperty(target, "windowOpacity", 1.0, 0.0, dur, QEasingCurve::InCubic); return;
    case SlideLeft: animateProperty(target, "geometry", target->geometry(), target->geometry().translated(-target->width(), 0), dur, QEasingCurve::OutCubic); return;
    case SlideRight: animateProperty(target, "geometry", target->geometry(), target->geometry().translated(target->width(), 0), dur, QEasingCurve::OutCubic); return;
    case SlideUp: animateProperty(target, "geometry", target->geometry(), target->geometry().translated(0, -target->height()), dur, QEasingCurve::OutCubic); return;
    case SlideDown: animateProperty(target, "geometry", target->geometry(), target->geometry().translated(0, target->height()), dur, QEasingCurve::OutCubic); return;
    case ScaleIn: animateProperty(target, "geometry", target->geometry(), target->geometry(), dur, QEasingCurve::OutBounce); return;
    case ScaleOut: animateProperty(target, "geometry", target->geometry(), target->geometry(), dur, QEasingCurve::InBounce); return;
    case Bounce: curve.setType(QEasingCurve::OutBounce); break;
    }
}

/**
 * @brief 对目标对象的指定属性执行自定义范围动画
 * @param target 目标QObject指针
 * @param prop 要动画化的属性名称
 * @param from 属性起始值
 * @param to 属性目标值
 * @param dur 动画持续时间(毫秒)，小于等于0时使用默认时长
 * @param curve 缓动曲线，无效时使用默认曲线
 */
void AnimationUtility::animateProperty(QObject *target, const QString &prop, const QVariant &from, const QVariant &to, int dur, const QEasingCurve &curve) {
    stopForTarget(target);
    auto *anim = new QVariantAnimation(this);
    anim->setStartValue(from); anim->setEndValue(to);
    anim->setDuration(dur > 0 ? dur : m_defaultDuration);
    anim->setEasingCurve(curve.isValid() ? curve : m_defaultCurve);
    connect(anim, &QVariantAnimation::valueChanged, target, [target, prop](const QVariant &v) { target->setProperty(prop.toUtf8().constData(), v); });
    connect(anim, &QVariantAnimation::finished, this, &AnimationUtility::onFinished);
    m_animations[target] = anim;
    anim->start(QVariantAnimation::DeleteWhenStopped);
    ++m_totalAnimationsStarted;
    emit animationStarted(target);
}

/** @brief 停止所有活跃动画并清空动画映射表 */
void AnimationUtility::stopAll() { int n = m_animations.size(); for (auto *a : m_animations) a->stop(); m_totalAnimationsStopped += static_cast<quint64>(n); m_animations.clear(); }

/**
 * @brief 停止指定目标对象上的动画
 * @param t 目标QObject指针
 */
void AnimationUtility::stopForTarget(QObject *t) { auto it = m_animations.find(t); if (it != m_animations.end()) { it.value()->stop(); m_animations.erase(it); ++m_totalAnimationsStopped; } }

/**
 * @brief 查询指定目标对象是否有活跃动画
 * @param t 目标QObject指针
 * @return 存在活跃动画返回true，否则返回false
 */
bool AnimationUtility::isAnimating(QObject *t) const { return m_animations.contains(t); }

/**
 * @brief 设置默认动画持续时间
 * @param ms 默认时长(毫秒)
 */
void AnimationUtility::setDefaultDuration(int ms) { m_defaultDuration = ms; }

/**
 * @brief 设置默认缓动曲线
 * @param c 缓动曲线对象
 */
void AnimationUtility::setDefaultCurve(const QEasingCurve &c) { m_defaultCurve = c; }

/** @brief 动画完成内部处理，从映射表中移除并发射animationFinished信号 */
void AnimationUtility::onFinished() { auto *anim = qobject_cast<QVariantAnimation *>(sender()); if (!anim) return; for (auto it = m_animations.begin(); it != m_animations.end(); ++it) if (it.value() == anim) { ++m_totalAnimationsFinished; emit animationFinished(it.key()); m_animations.erase(it); return; } }
