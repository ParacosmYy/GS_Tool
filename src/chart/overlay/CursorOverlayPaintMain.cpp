/**
 * @file CursorOverlayPaintMain.cpp
 * @brief 波形游标叠加层 -- 绘制主入口与差值面板
 *
 * 本文件从 CursorOverlayPaint.cpp 拆分而来，包含:
 *   - paintEvent:      绘制主入口(高亮区域→游标线→差值面板→框选矩形)
 *   - drawDeltaPanel:  差值信息面板绘制(ΔX/ΔY/频率估算)
 *
 * 辅助绘制方法(游标线/高亮区域/静态插值函数)
 * 见 CursorOverlayPaint.cpp。
 */

#include "chart/overlay/CursorOverlay.h"
#include "chart/model/ChartModel.h"
#include "chart/zoom/ZoomController.h"
#include "core/theme/ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>
#include <algorithm>
#include <cmath>

// interpolateY 声明在 CursorOverlayPaint.cpp 的匿名命名空间中，
// 此处重新定义供 drawDeltaPanel 使用。

/** @brief 二分查找+线性插值(与CursorOverlayPaint.cpp中相同实现) */
static bool interpolateYForMain(const QVector<QPointF>& data, double x, double& outY)
{
    if (data.isEmpty()) return false;
    if (x <= data.first().x()) { outY = data.first().y(); return true; }
    if (x >= data.last().x()) { outY = data.last().y(); return true; }
    auto it = std::lower_bound(data.begin(), data.end(), x,
        [](const QPointF& pt, double val) { return pt.x() < val; });
    if (it == data.end()) { outY = data.last().y(); return true; }
    if (it == data.begin()) { outY = it->y(); return true; }
    const QPointF& p0 = *(it - 1);
    const QPointF& p1 = *it;
    double dx = p1.x() - p0.x();
    if (qFuzzyIsNull(dx)) {
        outY = (p0.y() + p1.y()) * 0.5;
        return true;
    }
    double t = (x - p0.x()) / dx;
    outY = p0.y() + t * (p1.y() - p0.y());
    return true;
}

// ============================================================
// 差值信息面板绘制
// ============================================================

/**
 * @brief 绘制差值信息面板
 * @param painter 画笔
 *
 * 在图表右上角绘制一个半透明面板，显示:
 *   deltaX (采样点差)
 *   1/deltaX (频率估算)
 *   各通道的 deltaY (值差)
 */
void CursorOverlay::drawDeltaPanel(QPainter& painter)
{
    if (!m_hasCursorA || !m_hasCursorB || !m_model) return;

    double deltaX = std::abs(m_cursorBX - m_cursorAX);

    // 仅在游标实际移动时更新测量统计，避免每帧重绘递增
    if (m_measurementDirty) {
        m_measurementDirty = false;
        ++m_totalMeasurements;
        ++m_totalDeltaMeasurements;
        m_sumDeltaX += deltaX;
        if (!m_model->channelNames().isEmpty()) {
            const QString& firstCh = m_model->channelNames().first();
            const QVector<QPointF>& data = m_model->channelData(firstCh);
            double valA = 0.0, valB = 0.0;
            if (interpolateYForMain(data, m_cursorAX, valA) &&
                interpolateYForMain(data, m_cursorBX, valB)) {
                m_sumDeltaY += std::abs(valB - valA);
            }
        }
    }

    // 构建差值文本(使用tr()包裹用户可见文字)
    QStringList lines;
    lines << tr("ΔX: %1 采样").arg(deltaX, 0, 'f', 1);
    if (deltaX > 0) {
        lines << tr("1/ΔX: %1").arg(1.0 / deltaX, 0, 'f', 4);
    }

    // 各通道ΔY — 使用二分查找+插值(避免O(N)遍历和QVector拷贝)
    for (const QString& ch : m_model->channelNames()) {
        const QVector<QPointF>& data = m_model->channelData(ch);
        double valA = 0.0, valB = 0.0;
        if (interpolateYForMain(data, m_cursorAX, valA) &&
            interpolateYForMain(data, m_cursorBX, valB)) {
            lines << tr("%1 ΔY: %2").arg(ch).arg(valB - valA, 0, 'f', 3);
        }
    }

    // 绘制面板背景
    QFont font = painter.font();
    font.setPixelSize(11);
    painter.setFont(font);

    QFontMetrics fm(font);
    int maxW = 0;
    for (const QString& line : lines) {
        maxW = std::max(maxW, fm.horizontalAdvance(line));
    }
    int panelW = maxW + 16;
    int panelH = fm.height() * lines.size() + 12;
    int panelX = width() - panelW - 10;
    int panelY = 10;

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_panelBgColor);
    painter.drawRoundedRect(panelX, panelY, panelW, panelH, 4, 4);

    // 绘制文本
    painter.setPen(m_textColor);
    int textY = panelY + 6;
    for (const QString& line : lines) {
        painter.drawText(panelX + 8, textY + fm.ascent(), line);
        textY += fm.height();
    }
}

// ============================================================
// 绘制主入口
// ============================================================

/**
 * @brief 绘制游标线和差值信息面板
 * @param event 绘制事件(未使用)
 *
 * 绘制顺序: 高亮区域 → 游标A线 → 游标B线 → 差值面板 → 框选矩形
 */
void CursorOverlay::paintEvent(QPaintEvent* /*event*/)
{
    if (!m_hasCursorA && !m_hasCursorB) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 双游标模式: 绘制高亮区域
    if (m_hasCursorA && m_hasCursorB) {
        double pixelAX = dataToPixelX(m_cursorAX);
        double pixelBX = dataToPixelX(m_cursorBX);
        drawHighlightRegion(painter, pixelAX, pixelBX);
    }

    // 绘制游标A
    if (m_hasCursorA) {
        drawCursorLine(painter, dataToPixelX(m_cursorAX),
                       m_cursorAColor, "A");
    }

    // 绘制游标B
    if (m_hasCursorB) {
        drawCursorLine(painter, dataToPixelX(m_cursorBX),
                       m_cursorBColor, "B");
    }

    // 双游标模式: 绘制差值面板
    if (m_hasCursorA && m_hasCursorB) {
        // 累计ΔX/ΔY仅在实际测量时(非每帧重绘)，统计数据由setCursorA/setCursorB驱动
        drawDeltaPanel(painter);
    }

    // 绘制框选缩放的橡皮筋矩形(来自ZoomController)
    if (m_zoomController && m_zoomController->isRubberBandActive()) {
        QRectF rubberRect = m_zoomController->rubberBandRect();
        painter.setPen(QPen(m_cursorAColor, 1, Qt::DashLine));
        // 使用主题Accent色半透明填充，在深色和浅色主题下均可见
        QColor rubberFill = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
        rubberFill.setAlpha(30);
        painter.setBrush(rubberFill);
        painter.drawRect(rubberRect);
    }
}
