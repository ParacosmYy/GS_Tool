#include "core/animation2/AnimationUtility2.h"
#include <QVariantAnimation>
#include <QWidget>

AnimationUtility::AnimationUtility(QObject *parent) : QObject(parent) {}
AnimationUtility::~AnimationUtility() { stopAll(); }

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
    emit animationStarted(target);
}

void AnimationUtility::stopAll() { for (auto *a : m_animations) a->stop(); m_animations.clear(); }
void AnimationUtility::stopForTarget(QObject *t) { auto it = m_animations.find(t); if (it != m_animations.end()) { it.value()->stop(); m_animations.erase(it); } }
bool AnimationUtility::isAnimating(QObject *t) const { return m_animations.contains(t); }
void AnimationUtility::setDefaultDuration(int ms) { m_defaultDuration = ms; }
void AnimationUtility::setDefaultCurve(const QEasingCurve &c) { m_defaultCurve = c; }
void AnimationUtility::onFinished() { auto *anim = qobject_cast<QVariantAnimation *>(sender()); if (!anim) return; for (auto it = m_animations.begin(); it != m_animations.end(); ++it) if (it.value() == anim) { emit animationFinished(it.key()); m_animations.erase(it); return; } }
