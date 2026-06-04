/**
 * @file EdDialogPaint.cpp
 * @brief 自定义对话框组件 — 绘制、动画与键盘事件实现
 *
 * 从 EdDialog.cpp 拆分而来，包含:
 *   - paintEvent(): 自绘圆角背景 + 左侧强调色边框
 *   - showEvent(): 200ms OutCubic 淡入+缩放动画
 *   - keyPressEvent(): Escape键关闭
 *
 * 构造函数见 EdDialog.cpp。
 * 辅助方法(颜色/图标/关闭动画/静态入口)见 EdDialogTabs.cpp。
 */

#include "core/widgets/EdDialog.h"

#include <QKeyEvent>
#include <QParallelAnimationGroup>

/**
 * @brief 自绘事件 — 绘制圆角背景 + 左侧强调色边框
 */
void EdDialog::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto& theme = ThemeManager::instance();
    QColor bg = theme.color(ThemeManager::SemanticColor::BgSecondary);
    QColor accent = accentColor();

    /* 圆角背景 */
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius, kRadius);
    p.fillPath(path, bg);

    /* 左侧强调色边框 */
    p.save();
    p.setClipPath(path);
    p.fillRect(QRect(0, 0, kLeftBorder, height()), accent);
    p.restore();
}

/**
 * @brief 显示事件 — 触发 200ms OutCubic 淡入+缩放动画
 */
void EdDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    ++s_totalDialogOpens;

    /* 缩放动画: 95% → 100% */
    auto* scaleAnim = new QPropertyAnimation(this, "geometry");
    QRect finalGeo = geometry();
    QPoint center = finalGeo.center();
    QRect startGeo = finalGeo;
    startGeo.setWidth(static_cast<int>(finalGeo.width() * 0.95));
    startGeo.setHeight(static_cast<int>(finalGeo.height() * 0.95));
    startGeo.moveCenter(center);
    setGeometry(startGeo);
    scaleAnim->setStartValue(startGeo);
    scaleAnim->setEndValue(finalGeo);
    scaleAnim->setDuration(200);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);

    /* 淡入动画: 0.0 → 1.0 */
    auto* fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setDuration(200);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(scaleAnim);
    group->addAnimation(fadeAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 键盘事件 — Escape 键关闭对话框
 */
void EdDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_resultCode = QDialog::Rejected;
        closeWithAnimation();
        return;
    }
    QDialog::keyPressEvent(event);
}
