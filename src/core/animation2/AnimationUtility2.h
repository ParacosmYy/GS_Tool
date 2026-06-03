#pragma once
#include <QObject>
#include <QEasingCurve>
#include <QMap>
#include <QString>
#include <functional>

class QVariantAnimation;
class QWidget;

class AnimationUtility : public QObject {
    Q_OBJECT
public:
    enum Preset { FadeIn, FadeOut, SlideLeft, SlideRight, SlideUp, SlideDown, ScaleIn, ScaleOut, Bounce };
    Q_ENUM(Preset)

    explicit AnimationUtility(QObject *parent = nullptr);
    ~AnimationUtility() override;
    void animate(QWidget *target, Preset preset, int durationMs = 300);
    void animateProperty(QObject *target, const QString &property, const QVariant &from, const QVariant &to, int durationMs = 300, const QEasingCurve &curve = QEasingCurve::OutCubic);
    void stopAll();
    void stopForTarget(QObject *target);
    bool isAnimating(QObject *target) const;
    void setDefaultDuration(int ms);
    void setDefaultCurve(const QEasingCurve &curve);
signals:
    void animationFinished(QObject *target);
    void animationStarted(QObject *target);
private:
    void onFinished();
    QMap<QObject *, QVariantAnimation *> m_animations;
    int m_defaultDuration = 300;
    QEasingCurve m_defaultCurve{QEasingCurve::OutCubic};
};
