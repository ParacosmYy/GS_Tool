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
#include <QShowEvent>
#include <QHideEvent>
#include <QTimer>
#include <QConicalGradient>

#include "core/theme/ThemeManager.h"

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造加载旋转指示器 @param size 控件固定尺寸(宽高相等) @param parent 父控件指针 */
LoadingSpinner::LoadingSpinner(int size, QWidget* parent)
    : QWidget(parent)
{
    setObjectName("loadingSpinner");
    setFixedSize(size, size);

    // 旋转定时器: 每100ms递增36° = 1秒360° = 1圈/秒
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + 36) % 360;
        ++m_totalTicks;
        update();
    });
    m_timer->start(100);
    ++m_totalStarts;
}

// ============================================================================
// 公开接口
// ============================================================================

/** @brief 获取当前线条宽度 @return 线条宽度(像素) */
int LoadingSpinner::lineWidth() const
{
    return m_lineWidth;
}

/** @brief 设置线条宽度，最小值为1 @param width 线条宽度(像素) */
void LoadingSpinner::setLineWidth(int width)
{
    m_lineWidth = qMax(1, width);
    update();
}

// ============================================================================
// 动画控制
// ============================================================================

/** @brief 启动旋转动画，若已在运行则不重复启动 */
void LoadingSpinner::start()
{
    if (!m_timer->isActive()) {
        m_timer->start(100);
        ++m_totalStarts;
    }
}

/** @brief 停止旋转动画，若未运行则不执行操作 */
void LoadingSpinner::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        ++m_totalStops;
    }
}

/** @brief 查询旋转动画是否正在运行 @return true表示正在旋转，false表示已停止 */
bool LoadingSpinner::isSpinning() const
{
    return m_timer && m_timer->isActive();
}

// ============================================================================
// 统计重置
// ============================================================================

/** @brief 重置旋转指示器的统计计数器(启动次数和停止次数) */
void LoadingSpinner::resetSpinnerStatistics()
{
    m_totalStarts = 0;
    m_totalStops = 0;
    m_totalTicks = 0;
}

// ============================================================================
// 自绘: 旋转圆弧
// ============================================================================

/** @brief 自绘事件，绘制背景圆和270°锥形渐变旋转圆弧 @param event 绘制事件(未使用) */
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

/** @brief 显示事件 — 若动画未运行则恢复旋转 @param event 显示事件 */
void LoadingSpinner::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive()) m_timer->start(100);
}

/** @brief 隐藏事件 — 暂停旋转动画以节省CPU @param event 隐藏事件 */
void LoadingSpinner::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    m_timer->stop();
}
