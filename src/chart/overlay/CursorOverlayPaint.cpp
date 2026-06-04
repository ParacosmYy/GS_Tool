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

// 绘制主入口(paintEvent)与差值面板(drawDeltaPanel)
// 见 CursorOverlayPaintMain.cpp
