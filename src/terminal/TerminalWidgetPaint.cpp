/**
 * @file TerminalWidgetPaint.cpp
 * @brief 自绘制终端控件的渲染实现 — paintEvent主循环与paintLine单行绘制
 *
 * 本文件从TerminalWidget.cpp拆分而来，专注于QPainter渲染逻辑：
 *   - paintEvent: 主渲染循环，处理方向过滤模式与普通模式的缓存构建和行绘制
 *   - paintLine: 单行绘制，包含时间戳、方向前缀、搜索高亮和选区背景
 *
 * 缓存格式化逻辑(formatToCache)和辅助函数(safeFromUtf8)保留在TerminalWidget.cpp中。
 */

#include "terminal/TerminalWidget.h"
#include "core/ThemeManager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>

// ---- 单行绘制 ----
/** @brief 绘制单行终端内容(时间戳+方向前缀+HEX/ASCII数据+搜索高亮) @param painter 画笔 @param cached 缓存行数据 @param y 起始Y坐标 @param displayLine 显示行号 @return 绘制消耗的像素高度 */
int TerminalWidget::paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine)
{
    int xOffset = 0;
    if (m_showTimestamp) {
        painter.setPen(m_timestampColor);
        QString ts = QDateTime::fromMSecsSinceEpoch(cached.timestamp).toString("HH:mm:ss.zzz");
        painter.drawText(4, y + m_lineHeight - 4, ts);
        xOffset = m_fontMetrics.horizontalAdvance(ts) + 12;
    }

    // 选择背景色
    if (m_selectionManager->hasSelection()) {
        int selStart = m_selectionManager->normalizedStartLine();
        int selEnd = m_selectionManager->normalizedEndLine();
        if (displayLine >= selStart && displayLine <= selEnd)
            painter.fillRect(0, y, width(), m_lineHeight, m_selectionManager->selectionBgColor());
    }

    // 方向前缀 "[TX:] " / "[RX:] " 分色渲染
    // 计算文本绘制基线: 先算出方向前缀占用的像素宽度，
    // 搜索高亮和实际文本内容都基于这个基线对齐，避免双倍偏移
    static const QString kTxPrefix = QStringLiteral("[TX:] ");
    static const QString kRxPrefix = QStringLiteral("[RX:] ");
    bool isTx = (cached.direction == DataDirection::Tx);
    QColor dataColor = isTx ? m_txColor : m_rxColor;

    // textXOffset = 文本内容的起始像素位置（时间戳宽度 + 边距 + 方向前缀宽度）
    int textXOffset = xOffset + 4;
    if (m_showDirectionPrefix) {
        const QString& prefix = isTx ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            textXOffset += m_fontMetrics.horizontalAdvance(prefix);
        }
    }

    // 搜索高亮 — 委托给 TerminalSearchRenderer 绘制
    TerminalSearchRenderer::drawHighlights(painter, m_fontMetrics, m_searchManager,
                                           cached, displayLine, textXOffset,
                                           y, m_lineHeight, m_showDirectionPrefix);

    // 绘制文本内容（方向前缀 + 实际数据）
    if (m_showDirectionPrefix) {
        const QString& prefix = isTx ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            painter.setPen(dataColor.darker(130));
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, prefix);
            painter.setPen(dataColor);
            painter.drawText(textXOffset, y + m_lineHeight - 4,
                             cached.text.mid(prefix.length()));
        } else {
            painter.setPen(dataColor);
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
        }
    } else {
        painter.setPen(dataColor);
        painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
    }
    return y + m_lineHeight;
}

// ---- 核心渲染 ----
/** @brief 自绘事件：绘制可见区域的终端行(时间戳+方向+数据+搜索高亮+选中) @param event 绘制事件 */
void TerminalWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), m_bgColor);
    painter.setFont(m_font);

    if (!m_model) {
        painter.setPen(m_timestampColor);
        painter.drawText(rect(), Qt::AlignCenter, tr("未连接 - 等待数据..."));
        return;
    }

    int modelTotalLines = m_model->lineCount();

    // ---- 方向过滤模式 ----
    if (m_directionFilter->isFiltered()) {
        if (m_cachedLineCount > modelTotalLines) {
            m_cachedLineCount = 0; m_cachedLines.clear(); m_directionFilter->reset();
        }
        if (m_cachedLineCount != modelTotalLines) {
            m_cachedLines.resize(modelTotalLines);
            for (int i = m_cachedLineCount; i < modelTotalLines; ++i)
                m_cachedLines[i] = formatToCache(m_model->lineAt(i));
            m_directionFilter->onDataAppended(modelTotalLines,
                [this](int idx) { return m_model->lineAt(idx); });
            m_cachedLineCount = modelTotalLines;
        }
        int totalLines = m_directionFilter->filteredLineCount();
        if (!m_searchManager->searchPattern().isEmpty() && totalLines > 0)
            QTimer::singleShot(0, this, [this]() { refreshSearchAfterCacheUpdate(); });
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
        int endLine = qMin(m_scrollOffset + m_visibleLines + 1, totalLines);
        int y = 0;
        for (int i = m_scrollOffset; i < endLine; ++i) {
            int modelLine = m_directionFilter->modelIndex(i);
            if (modelLine < 0 || modelLine >= m_cachedLines.size()) break;
            y = paintLine(painter, m_cachedLines[modelLine], y, i);
        }
        return;
    }

    // ---- 普通模式(无方向过滤) ----
    int totalLines = modelTotalLines;
    if (m_cachedLineCount != totalLines) {
        if (m_cachedLineCount > totalLines) {
            m_cachedLineCount = 0; m_cachedLines.clear();
        }
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i)
            m_cachedLines[i] = formatToCache(m_model->lineAt(i));
        m_cachedLineCount = totalLines;
        if (!m_searchManager->searchPattern().isEmpty() && m_cachedLineCount > 0)
            QTimer::singleShot(0, this, [this]() { refreshSearchAfterCacheUpdate(); });
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    int endLine = qMin(m_scrollOffset + m_visibleLines + 1, totalLines);
    int y = 0;
    for (int i = m_scrollOffset; i < endLine; ++i)
        y = paintLine(painter, m_cachedLines[i], y, i);
}
