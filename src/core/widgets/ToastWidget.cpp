/** @file ToastWidget.cpp @brief ToastWidget实现 -- 构造/绘制/显示/消失/堆叠重排/防抖 */
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFontMetrics>

#include "core/widgets/ToastWidget.h"
#include "core/theme/ThemeManager.h"
#include "shared/AnimationConstants.h"

// ---- 构造 ----
ToastWidget::ToastWidget(QWidget* parent, const QString& message, ToastType type)
    : QWidget(parent), m_type(type), m_message(message)
{
    setObjectName("toastWidget");
    setProperty("type", type == ToastType::Success ? "success"
                 : type == ToastType::Error ? "error" : "info");
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(kWidth);
    QFont textFont;
    textFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
    textFont.setPointSize(12);
    QFontMetrics fm(textFont);
    int textW = kWidth - kLeftBorder - kPad * 3 - kIconArea;
    QRect bound = fm.boundingRect(0, 0, textW, 0, Qt::TextWordWrap, message);
    setFixedHeight(qMax(kMinHeight, bound.height() + kPad * 2));
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);
}

// ---- 颜色/图标 ----
QColor ToastWidget::semanticColor() const
{
    using SC = ThemeManager::SemanticColor;
    switch (m_type) {
    case ToastType::Success: return ThemeManager::instance().color(SC::Success);
    case ToastType::Error:   return ThemeManager::instance().color(SC::Error);
    case ToastType::Info:    return ThemeManager::instance().color(SC::Accent);
    }
    return ThemeManager::instance().color(SC::Accent);
}

QString ToastWidget::iconChar() const
{
    switch (m_type) {
    case ToastType::Success: return tr("✓");
    case ToastType::Error:   return tr("✕");
    case ToastType::Info:    return tr("ℹ");
    }
    return tr("ℹ");
}

// ---- 绘制 ----
void ToastWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    auto& theme = ThemeManager::instance();
    QColor bg = theme.color(ThemeManager::SemanticColor::BgSecondary);
    QColor accent = semanticColor();
    QColor txt = theme.color(ThemeManager::SemanticColor::TextPrimary);
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius, kRadius);
    p.fillPath(path, bg);
    p.save();
    p.setClipPath(path);
    p.fillRect(QRect(0, 0, kLeftBorder, height()), accent);
    p.restore();
    int iconX = kLeftBorder + kPad;
    p.setPen(accent);
    QFont iconFont;
    iconFont.setFamilies({"Segoe UI Emoji", "Noto Color Emoji", "Apple Color Emoji"});
    iconFont.setPointSize(kIconSize);
    iconFont.setBold(true);
    p.setFont(iconFont);
    p.drawText(QRect(iconX, 0, kIconArea, height()), Qt::AlignCenter, iconChar());
    int textX = iconX + kIconArea + kPad;
    p.setPen(txt);
    QFont textFont;
    textFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
    textFont.setPointSize(12);
    p.setFont(textFont);
    p.drawText(QRect(textX, kPad, width() - textX - kPad, height() - kPad * 2),
               Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, m_message);
}

// ---- 静态映射 ----
QList<ToastWidget*>& ToastWidget::activeToasts(QWidget* parent) { return activeToastsMap()[parent]; }
QMap<QWidget*, QList<ToastWidget*>>& ToastWidget::activeToastsMap() {
    static QMap<QWidget*, QList<ToastWidget*>> map; return map; }
QHash<QString, QElapsedTimer>& ToastWidget::debounceMap() {
    static QHash<QString, QElapsedTimer> map; return map; }

// ---- 消失动画 ----
void ToastWidget::dismiss()
{
    ++s_totalDismisses;
    auto* group = new QParallelAnimationGroup(this);
    auto* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(Animations::kToastDismissMs);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(fadeOut);
    auto* drift = new QPropertyAnimation(this, "pos");
    drift->setStartValue(pos());
    drift->setEndValue(pos() + QPoint(0, -30));
    drift->setDuration(Animations::kToastDismissMs);
    drift->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(drift);
    connect(group, &QAbstractAnimation::finished, this, [this]() {
        QWidget* pw = parentWidget();
        if (pw) { activeToasts(pw).removeOne(this); repositionToasts(pw); }
        deleteLater();
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---- 重排 ----
void ToastWidget::repositionToasts(QWidget* parent)
{
    if (!parent) return;
    auto& list = activeToasts(parent);
    int bottomY = parent->height() - kMargin;
    for (auto* t : list) {
        int y = bottomY - t->height();
        QPoint tgt(parent->width() - kMargin - t->width(), y);
        if (t->pos() != tgt) {
            auto* slide = new QPropertyAnimation(t, "pos");
            slide->setEndValue(tgt);
            slide->setDuration(Animations::kNavIndicatorMs);
            slide->setEasingCurve(QEasingCurve::OutCubic);
            slide->start(QAbstractAnimation::DeleteWhenStopped);
        }
        bottomY = y - kGap;
    }
}

// ---- show ----
void ToastWidget::show(QWidget* parent, const QString& msg, ToastType type, int ms)
{
    if (!parent) return;
    auto* toast = new ToastWidget(parent, msg, type);
    ++s_totalShows;
    if (type == ToastType::Error) ++s_totalErrors;
    activeToasts(parent).append(toast);
    if (!parent->property("_toastDestroyConnected").toBool()) {
        connect(parent, &QObject::destroyed, parent, [parent]() { activeToastsMap().remove(parent); });
        parent->setProperty("_toastDestroyConnected", true);
    }
    const auto& list = activeToasts(parent);
    int bottomY = parent->height() - kMargin;
    for (auto* t : list) { bottomY -= t->height(); if (t != toast) bottomY -= kGap; }
    QPoint target(parent->width() - kMargin - toast->width(), bottomY);
    toast->move(target.x(), target.y() + 30);
    toast->QWidget::show();
    auto* slide = new QPropertyAnimation(toast, "pos");
    slide->setEndValue(target);
    slide->setDuration(Animations::kToastPopMs);
    slide->setEasingCurve(QEasingCurve::OutBack);
    auto* fade = new QPropertyAnimation(toast->m_opacityEffect, "opacity");
    fade->setEndValue(1.0);
    fade->setDuration(Animations::kToastPopMs);
    fade->setEasingCurve(QEasingCurve::OutBack);
    connect(slide, &QAbstractAnimation::finished, toast, [toast, ms]() {
        QTimer::singleShot(ms, toast, [toast]() { toast->dismiss(); });
    });
    slide->start(QAbstractAnimation::DeleteWhenStopped);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---- showDebounced ----
void ToastWidget::showDebounced(QWidget* parent, const QString& msg, ToastType type, int cooldownMs)
{
    QString key = QString::number(static_cast<int>(type)) + "|" + msg;
    auto& map = debounceMap();
    if (map.contains(key) && map[key].elapsed() < cooldownMs) return;
    map[key].start();
    show(parent, msg, type);
}

// ---- 统计 ----
quint64 ToastWidget::totalShows() { return s_totalShows; }
quint64 ToastWidget::totalDismisses() { return s_totalDismisses; }
quint64 ToastWidget::totalErrors() { return s_totalErrors; }
void ToastWidget::resetToastStatistics() { s_totalShows = 0; s_totalDismisses = 0; s_totalErrors = 0; }
