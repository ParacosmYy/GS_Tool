/**
 * @file CursorOverlayPaint.cpp
 * @brief 波形游标叠加层 -- 绘制相关方法实现
 *
 * 本文件从 CursorOverlay.cpp 拆分而来，专注于 QPainter 绘制逻辑:
 *   - paintEvent:     绘制主入口，协调高亮区域、游标线和差值面板
 *   - drawCursorLine: 绘制单条游标竖线和标签
 *   - drawHighlightRegion: 绘制双游标间的高亮区域
 *   - drawDeltaPanel: 绘制差值信息面板(ΔX/ΔY/频率估算)
 *   - interpolateY:   二分查找+线性插值辅助函数
 *
 * 拆分原因:
 *   CursorOverlay.cpp 职责包括构造、游标管理、事件处理和绘制，
 *   将绘制逻辑独立成文件可降低单文件复杂度，便于维护。
 */

#include "chart/overlay/CursorOverlay.h"
#include "chart/model/ChartModel.h"
#include "chart/zoom/ZoomController.h"
#include "core/theme/ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>
#include <QChartView>
#include <QChart>
#include <algorithm>
#include <cmath>

// ============================================================
// 绘制辅助 — 静态插值函数
// ============================================================

/**
 * @brief 在已排序数据中用二分查找定位游标X对应的Y值(含线性插值)
 * @param data 已按X排序的数据点
 * @param x 目标X值
 * @param outY 输出Y值
 * @return true=找到并插值成功
 *
 * 使用 std::lower_bound 实现O(logN)查找，避免O(N)遍历。
 * 当相邻数据点X值相同时取Y平均值，避免除零。
 */
static bool interpolateY(const QVector<QPointF>& data, double x, double& outY)
{
    if (data.isEmpty()) return false;

    // X在数据范围外 — 取边界值
    if (x <= data.first().x()) { outY = data.first().y(); return true; }
    if (x >= data.last().x()) { outY = data.last().y(); return true; }

    // 二分查找第一个 x >= target 的位置
    auto it = std::lower_bound(data.begin(), data.end(), x,
        [](const QPointF& pt, double val) { return pt.x() < val; });

    if (it == data.end()) { outY = data.last().y(); return true; }
    if (it == data.begin()) { outY = it->y(); return true; }

    // 线性插值: it-1 和 it 之间
    const QPointF& p0 = *(it - 1);
    const QPointF& p1 = *it;
    double dx = p1.x() - p0.x();
    if (qFuzzyIsNull(dx)) {
        // 相邻数据点X值相同，取平均值
        outY = (p0.y() + p1.y()) * 0.5;
        return true;
    }
    double t = (x - p0.x()) / dx;
    outY = p0.y() + t * (p1.y() - p0.y());
    return true;
}

// ============================================================
// 游标竖线绘制
// ============================================================

/** @brief 绘制单条游标竖线 @param painter 画笔 @param pixelX 游标像素X位置 @param color 游标颜色 @param label 游标标签(A/B) */
void CursorOverlay::drawCursorLine(QPainter& painter, double pixelX,
                                   const QColor& color, const QString& label)
{
    painter.setPen(QPen(color, 2, Qt::DashLine));
    painter.drawLine(static_cast<int>(pixelX), 0,
                     static_cast<int>(pixelX), height());

    // 绘制标签
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    painter.setPen(color);

    QRect textRect(static_cast<int>(pixelX) + 4, 4, 20, 16);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, label);
}

// ============================================================
// 高亮区域绘制
// ============================================================

/** @brief 绘制游标间的高亮区域 @param painter 画笔 @param pixelAX 游标A像素X @param pixelBX 游标B像素X */
void CursorOverlay::drawHighlightRegion(QPainter& painter, double pixelAX,
                                        double pixelBX)
{
    int left = static_cast<int>(std::min(pixelAX, pixelBX));
    int right = static_cast<int>(std::max(pixelAX, pixelBX));
    painter.fillRect(left, 0, right - left, height(), m_highlightColor);
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
        if (interpolateY(data, m_cursorAX, valA) &&
            interpolateY(data, m_cursorBX, valB)) {
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
        ++m_totalMeasurements;
        // 累计ΔX/ΔY用于计算平均值
        double deltaX = std::abs(m_cursorBX - m_cursorAX);
        m_sumDeltaX += deltaX;
        // 累计首个通道的ΔY
        if (m_model && !m_model->channelNames().isEmpty()) {
            const QString& firstCh = m_model->channelNames().first();
            const QVector<QPointF>& data = m_model->channelData(firstCh);
            double valA = 0.0, valB = 0.0;
            if (interpolateY(data, m_cursorAX, valA) &&
                interpolateY(data, m_cursorBX, valB)) {
                m_sumDeltaY += std::abs(valB - valA);
            }
        }
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
