/**
 * @file BackgroundWidgetPaint.cpp
 * @brief 背景层控件 - 绘制与动画方法
 *
 * 从 BackgroundWidget.cpp 拆分，包含:
 *   - paintEvent: 四层渲染(黑底→模糊图→遮罩→涟漪)
 *   - generateBlurred: 缩放法快速高斯模糊
 *   - regenerateScaledBackground: Cover模式缩放缓存
 *   - advanceRipples: 涟漪帧动画
 *   - mousePressEvent: 涟漪触发
 *   - resizeEvent: 缩放缓存失效重建
 *
 * 性能关键路径: paintEvent 不做耗时缩放，所有缩放在 resize 时预缓存。
 */

#include "core/background/BackgroundWidget.h"
#include "core/theme/ThemeManager.h"
#include "shared/TimerConstants.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QtMath>

/** @brief 自绘事件，按四层渲染背景(黑底→模糊图→遮罩→涟漪) @param event 绘制事件 */
void BackgroundWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    ++m_totalPaints;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 第1层：主题背景色底色
    painter.fillRect(rect(), ThemeManager::instance().color(ThemeManager::SemanticColor::BgPrimary));

    // 第2层：背景图（模糊版），居中裁剪覆盖，使用预缓存避免每帧缩放
    if (!m_scaledBlurredImage.isNull()) {
        painter.setOpacity(m_bgOpacity);
        painter.drawPixmap(m_scaledOffset, m_scaledBlurredImage);
        painter.setOpacity(1.0);
    }

    // 第3层：半透明遮罩层（暗色遮罩让文字更易读）
    if (m_overlayOpacity > 0.0) {
        QColor overlay = m_overlayColor;
        overlay.setAlphaF(m_overlayOpacity);
        painter.fillRect(rect(), overlay);
    }

    // 第4层：涟漪特效（使用主题accent色）
    if (!m_ripples.isEmpty()) {
        for (const auto& ripple : m_ripples) {
            QRadialGradient gradient(ripple.center, ripple.currentRadius);
            QColor inner = m_rippleColor;
            inner.setAlpha(int(ripple.opacity * 80));
            QColor mid = m_rippleColor;
            mid.setAlpha(int(ripple.opacity * 30));
            QColor outer = m_rippleColor;
            outer.setAlpha(0);
            gradient.setColorAt(0, inner);
            gradient.setColorAt(0.5, mid);
            gradient.setColorAt(1, outer);
            painter.setBrush(gradient);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(ripple.center, ripple.currentRadius, ripple.currentRadius);
        }
    }
}

/** @brief 鼠标按下事件，在点击位置生成涟漪动画并启动定时器 @param event 鼠标事件 */
void BackgroundWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_rippleEnabled) {
        QWidget::mousePressEvent(event);
        return;
    }

    Ripple ripple;
    ripple.center = event->pos();
    ripple.currentRadius = 1.0;
    // 随机化最大半径，避免所有涟漪看起来完全相同
    ripple.maxRadius = 120.0 + QRandomGenerator::global()->bounded(60);
    ripple.opacity = 0.6;
    m_ripples.append(ripple);
    ++m_totalRipples;

    // 如果定时器未运行则启动（涟漪结束后自动停止）
    if (!m_rippleTimer->isActive()) {
        m_rippleTimer->start();
    }

    update();
}

/** @brief 窗口尺寸变化事件，重新缓存缩放后的背景图 @param event 尺寸变化事件 */
void BackgroundWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    ++m_totalResizes;
    regenerateScaledBackground();
}

/** @brief 从原始图片生成模糊版本，使用缩放法快速近似高斯模糊 @param src 原始像素图 @param radius 模糊半径 @return 模糊后的像素图 */
QPixmap BackgroundWidget::generateBlurred(const QPixmap& src, qreal radius) const
{
    if (src.isNull() || radius <= 0) return src;

    qreal baseFactor = 1.0 / (1.0 + radius * 0.15);

    QPixmap result = src;

    for (int i = 0; i < m_blurIterations; ++i) {
        // 第一次缩小最多，后续逐步放大回原尺寸
        qreal stepFactor = baseFactor + (1.0 - baseFactor) * (qreal(i) / qreal(m_blurIterations));
        QSize targetSize = src.size() * stepFactor;
        targetSize = targetSize.expandedTo(QSize(1, 1));  // 防止尺寸为零
        result = result.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 最终放大回原尺寸
    result = result.scaled(src.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    return result;
}

/** @brief 缓存当前窗口尺寸下的缩放背景图，Cover模式等比缩放填满整个widget */
void BackgroundWidget::regenerateScaledBackground()
{
    if (m_blurredImage.isNull()) {
        m_scaledBlurredImage = QPixmap();
        m_scaledOffset = QPoint();
        return;
    }

    // Cover模式: 等比缩放填满整个widget，裁剪多余部分
    QSize widgetSize = size();
    if (widgetSize.isEmpty()) return;

    QSize imgSize = m_blurredImage.size();
    qreal scaleX = (qreal)widgetSize.width() / imgSize.width();
    qreal scaleY = (qreal)widgetSize.height() / imgSize.height();
    // 取较大缩放比，确保填满（多余部分被裁剪）
    qreal scale = qMax(scaleX, scaleY);

    QSize scaledSize = imgSize * scale;
    // 计算居中偏移量（负值表示裁剪区域）
    m_scaledOffset = QPoint(
        (widgetSize.width() - scaledSize.width()) / 2,
        (widgetSize.height() - scaledSize.height()) / 2
    );

    // 预先缩放并缓存，paintEvent 直接 drawPixmap 不再做 scaled()
    m_scaledBlurredImage = m_blurredImage.scaled(
        scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

/** @brief 涟漪动画帧更新，每帧扩大半径(+4px)并降低透明度(-0.025)，完成后移除并停止定时器 */
void BackgroundWidget::advanceRipples()
{
    // 逆序遍历，安全删除已完成涟漪
    for (int i = m_ripples.size() - 1; i >= 0; --i) {
        auto& r = m_ripples[i];
        r.currentRadius += 4.0;
        r.opacity -= 0.025;

        if (r.opacity <= 0 || r.currentRadius >= r.maxRadius) {
            m_ripples.removeAt(i);
        }
    }

    // 所有涟漪完成后停止定时器，避免空转浪费 CPU
    if (m_ripples.isEmpty()) {
        m_rippleTimer->stop();
    }

    update();
}
