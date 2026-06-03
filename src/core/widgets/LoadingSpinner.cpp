/**
 * @file LoadingSpinner.cpp
 * @brief 加载旋转指示器实现 — 270°圆弧+锥形渐变旋转动画
 *
 * 实现细节:
 *   - QTimer 每100ms递增角度36°(1秒一圈)
 *   - paintEvent: 背景圆(10%透明度) + 旋转圆弧(锥形渐变)
 *   - 所有颜色通过 ThemeManager::Accent 获取
 */

#include "core/widgets/LoadingSpinner.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QConicalGradient>

#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

// ============================================================================
// 构造
// ============================================================================

LoadingSpinner::LoadingSpinner(int size, QWidget* parent)
    : QWidget(parent)
{
    setObjectName("loadingSpinner");
    setFixedSize(size, size);

    // 旋转定时器: 每100ms递增36° = 1秒360° = 1圈/秒
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + 36) % 360;
        update();
    });
    m_timer->start(100);
    ++m_totalStarts;
}

// ============================================================================
// 公开接口
// ============================================================================

int LoadingSpinner::lineWidth() const
{
    return m_lineWidth;
}

void LoadingSpinner::setLineWidth(int width)
{
    m_lineWidth = qMax(1, width);
    update();
}

// ============================================================================
// 动画控制
// ============================================================================

void LoadingSpinner::start()
{
    if (!m_timer->isActive()) {
        m_timer->start(100);
        ++m_totalStarts;
    }
}

void LoadingSpinner::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        ++m_totalStops;
    }
}

bool LoadingSpinner::isSpinning() const
{
    return m_timer && m_timer->isActive();
}

// ============================================================================
// 统计重置
// ============================================================================

void LoadingSpinner::resetSpinnerStatistics()
{
    m_totalStarts = 0;
    m_totalStops = 0;
}

// ============================================================================
// 自绘: 旋转圆弧
// ============================================================================

void LoadingSpinner::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    QColor accent = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);

    // 背景圆 (10%透明度)
    p.setOpacity(0.1);
    p.setBrush(accent);
    p.drawEllipse(rect().adjusted(m_lineWidth, m_lineWidth, -m_lineWidth, -m_lineWidth));

    // 旋转圆弧 (270°锥形渐变)
    p.setOpacity(1.0);
    QConicalGradient gradient(rect().center(), m_angle);
    gradient.setColorAt(0.0, accent);
    gradient.setColorAt(0.75, accent);  // 270° = 75%圆周
    gradient.setColorAt(0.76, QColor(accent.red(), accent.green(), accent.blue(), 0));
    gradient.setColorAt(1.0, QColor(accent.red(), accent.green(), accent.blue(), 0));

    QPen pen(QBrush(gradient), m_lineWidth, Qt::SolidLine, Qt::RoundCap);
    p.setPen(pen);
    p.drawArc(rect().adjusted(m_lineWidth, m_lineWidth, -m_lineWidth, -m_lineWidth),
              0, 270 * 16);
}
