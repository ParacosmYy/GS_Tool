/**
 * @file SkeletonWidget.cpp
 * @brief 骨架屏组件实现 — 圆角灰色矩形+线性渐变微光扫描动画
 *
 * 实现细节:
 *   - QTimer 每30ms递增偏移4px
 *   - 底色: ThemeManager::Border 语义色
 *   - 微光: 从左到右线性渐变，半透明白色叠加
 *   - 偏移范围 0 ~ 2*width，循环往复
 */

#include "core/widgets/SkeletonWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QLinearGradient>

#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

// ============================================================================
// 构造
// ============================================================================

SkeletonWidget::SkeletonWidget(int width, int height, int borderRadius,
                                 QWidget* parent)
    : QWidget(parent)
    , m_borderRadius(borderRadius)
{
    setObjectName("skeletonWidget");
    setFixedSize(width, height);

    // 微光动画: 每30ms偏移4px
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_shimmerOffset += 4;
        if (m_shimmerOffset > 2 * this->width()) {
            m_shimmerOffset = 0;
            ++m_totalAnimations;
        }
        update();
    });
    m_timer->start(30);
}

// ============================================================================
// 自绘: 骨架块 + 微光
// ============================================================================

void SkeletonWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor baseColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Border);

    // 底色圆角矩形
    p.setPen(Qt::NoPen);
    p.setBrush(baseColor);
    p.drawRoundedRect(rect(), m_borderRadius, m_borderRadius);

    // 微光扫描渐变
    QLinearGradient shimmer(m_shimmerOffset - width(), 0, m_shimmerOffset, 0);
    shimmer.setColorAt(0.0, Qt::transparent);
    shimmer.setColorAt(0.5, QColor(255, 255, 255, 30));
    shimmer.setColorAt(1.0, Qt::transparent);
    p.setBrush(shimmer);
    p.drawRoundedRect(rect(), m_borderRadius, m_borderRadius);
}

// ============================================================================
// 统计重置
// ============================================================================

void SkeletonWidget::resetSkeletonStatistics()
{
    m_totalAnimations = 0;
    m_totalLayoutChanges = 0;
}
