/**
 * @file PacketVisualizerPaint.cpp
 * @brief PacketVisualizer 绘制逻辑 -- paintEvent 实现
 *
 * 从 PacketVisualizer.cpp 拆分而来，仅包含 paintEvent 方法。
 * 绘制流程: 刻度标尺 → 字段着色块 → 悬停高亮 → 选中边框 → 未定义区域 → 统计更新。
 */

#include "protocol/visual/PacketVisualizer.h"

#include <QPainter>
#include <QFontMetrics>
#include "core/theme/ThemeManager.h"

/**
 * @brief 绘制数据包可视化图形
 *
 * 绘制流程:
 *   1. 空状态检测(无数据时显示提示)
 *   2. 刻度标尺(偏移量标记线 + 标签)
 *   3. 各字段着色块(圆角矩形 + 名称 + 十六进制值)
 *   4. 悬停高亮(半透明白色叠加)
 *   5. 选中边框(强调色描边)
 *   6. 未定义区域(灰色填充)
 *   7. 无字段定义时的原始数据十六进制显示
 *   8. 渲染统计更新 + packetRendered 信号发射
 *
 * @param event 绘制事件
 */
void PacketVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto& theme = ThemeManager::instance();

    int totalLen = totalPacketLength();
    if (totalLen <= 0 && m_data.isEmpty()) {
        /* 无数据时绘制空状态提示 */
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));
        painter.setFont(font());
        painter.drawText(rect(), Qt::AlignCenter, tr("No packet data"));
        painter.end();
        return;
    }

    if (totalLen <= 0) {
        totalLen = 1;  ///< 防止除零
    }

    /* 自适应模式: 每次绘制重新计算缩放 */
    if (m_autoFit) {
        m_zoomLevel = computeFitZoom();
    }

    /* 绘制参数 */
    const int leftMargin = 10;
    const int topMargin = 20;
    const int rulerHeight = 16;             ///< 刻度标尺高度
    const int blockY = topMargin + rulerHeight + 4;  ///< 字段块起始Y
    const int blockHeight = height() - blockY - 4;   ///< 字段块高度
    const double unitPx = m_zoomLevel;      ///< 每单位像素数

    /* --- 1. 绘制刻度标尺 --- */
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));
    QFont rulerFont = font();
    rulerFont.setPointSize(qMax(7, font().pointSize() - 1));
    painter.setFont(rulerFont);

    int step = 1;  ///< 刻度间隔(字节或位)
    if (unitPx < 6.0) {
        step = (m_viewMode == ViewMode::BitLevel) ? 32 : 8;
    } else if (unitPx < 12.0) {
        step = (m_viewMode == ViewMode::BitLevel) ? 16 : 4;
    } else if (unitPx < 20.0) {
        step = (m_viewMode == ViewMode::BitLevel) ? 8 : 2;
    }

    QFontMetrics fm(rulerFont);
    for (int i = 0; i <= totalLen; i += step) {
        double x = leftMargin + i * unitPx - m_scrollOffset;
        if (x < leftMargin - 20 || x > width()) continue;

        /* 刻度线 */
        painter.setPen(theme.color(ThemeManager::SemanticColor::Border));
        painter.drawLine(static_cast<int>(x), topMargin, static_cast<int>(x), topMargin + rulerHeight);

        /* 刻度标签 */
        QString label = (m_viewMode == ViewMode::BitLevel)
                            ? QString("b%1").arg(i)
                            : QString::number(i);
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));
        QRect labelRect(static_cast<int>(x) - 20, topMargin, 40, rulerHeight);
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    /* --- 2. 绘制字段块 --- */
    int fieldCount = 0;
    for (int i = 0; i < m_fieldDefs.size(); ++i) {
        const auto &def = m_fieldDefs[i];
        double fieldX = leftMargin + def.offset * unitPx - m_scrollOffset;
        double fieldW = def.length * unitPx;

        /* 跳过完全不可见的字段 */
        if (fieldX + fieldW < leftMargin || fieldX > width()) continue;

        /* 裁剪到可见区域 */
        double drawX = fieldX;
        double drawW = fieldW;
        if (drawX < leftMargin) {
            drawW -= (leftMargin - drawX);
            drawX = leftMargin;
        }

        QColor blockColor = def.color.isValid() ? def.color
                            : theme.color(ThemeManager::SemanticColor::Accent);

        /* 绘制字段着色块 */
        QRectF blockRect(drawX, blockY, drawW, blockHeight);
        painter.setPen(Qt::NoPen);
        painter.setBrush(blockColor);
        painter.drawRoundedRect(blockRect, 3.0, 3.0);

        /* 悬停高亮叠加 */
        if (i == m_hoveredIndex) {
            painter.setBrush(QColor(255, 255, 255, 40));
            painter.drawRoundedRect(blockRect, 3.0, 3.0);
        }

        /* 选中边框 */
        if (i == m_selectedIndex) {
            QColor selectColor = theme.color(ThemeManager::SemanticColor::Accent);
            QPen selectPen(selectColor, 2.5);
            painter.setPen(selectPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(blockRect.adjusted(1, 1, -1, -1), 3.0, 3.0);
        }

        /* 字段内文本: 名称 + 十六进制值 */
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextPrimary));
        QFont textFont = font();
        int fontSize = qMax(7, static_cast<int>(blockHeight * 0.18));
        textFont.setPointSize(fontSize);
        painter.setFont(textFont);

        QFontMetrics textFm(textFont);
        /* 名称(第一行) */
        QString nameText = def.name;
        if (textFm.horizontalAdvance(nameText) > drawW - 6) {
            nameText = textFm.elidedText(nameText, Qt::ElideRight, static_cast<int>(drawW - 6));
        }
        QRectF nameRect(drawX + 3, blockY + 2, drawW - 6, blockHeight * 0.45);
        painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignBottom, nameText);

        /* 十六进制值(第二行) */
        if (drawW > 20) {
            QString hexText = fieldHexValue(def);
            QFont smallFont = textFont;
            smallFont.setPointSize(qMax(6, fontSize - 1));
            painter.setFont(smallFont);
            QFontMetrics smallFm(smallFont);
            if (smallFm.horizontalAdvance(hexText) > drawW - 6) {
                hexText = smallFm.elidedText(hexText, Qt::ElideMiddle, static_cast<int>(drawW - 6));
            }
            QRectF hexRect(drawX + 3, blockY + blockHeight * 0.5, drawW - 6, blockHeight * 0.45);
            painter.drawText(hexRect, Qt::AlignLeft | Qt::AlignTop, hexText);
        }

        ++fieldCount;
    }

    /* --- 3. 绘制未定义区域(灰色底色) --- */
    if (!m_fieldDefs.isEmpty()) {
        /* 找出所有未被字段覆盖的区域 */
        int dataLen = (m_viewMode == ViewMode::BitLevel) ? m_data.size() * 8 : m_data.size();
        QVector<bool> covered(dataLen, false);

        for (const auto &def : m_fieldDefs) {
            int start = qMax(0, def.offset);
            int end = qMin(dataLen, def.offset + def.length);
            for (int b = start; b < end; ++b) {
                covered[b] = true;
            }
        }

        /* 绘制连续的未覆盖区域 */
        int gapStart = -1;
        for (int b = 0; b <= dataLen; ++b) {
            bool isCovered = (b < dataLen) ? covered[b] : true;
            if (!isCovered && gapStart < 0) {
                gapStart = b;
            } else if (isCovered && gapStart >= 0) {
                double gapX = leftMargin + gapStart * unitPx - m_scrollOffset;
                double gapW = (b - gapStart) * unitPx;
                QRectF gapRect(gapX, blockY, gapW, blockHeight);
                painter.setPen(Qt::NoPen);
                painter.setBrush(theme.color(ThemeManager::SemanticColor::BgTertiary));
                painter.drawRoundedRect(gapRect, 2.0, 2.0);
                gapStart = -1;
            }
        }
    } else if (!m_data.isEmpty()) {
        /* 无字段定义: 将整个数据作为灰色块 */
        double dataLen = (m_viewMode == ViewMode::BitLevel) ? m_data.size() * 8 : m_data.size();
        double dataX = leftMargin - m_scrollOffset;
        double dataW = dataLen * unitPx;
        QRectF dataRect(dataX, blockY, dataW, blockHeight);
        painter.setPen(Qt::NoPen);
        painter.setBrush(theme.color(ThemeManager::SemanticColor::BgTertiary));
        painter.drawRoundedRect(dataRect, 3.0, 3.0);

        /* 显示原始十六进制 */
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextSecondary));
        QFont hexFont = font();
        hexFont.setPointSize(qMax(7, font().pointSize() - 1));
        painter.setFont(hexFont);

        for (int i = 0; i < m_data.size(); ++i) {
            double charX = leftMargin + i * unitPx - m_scrollOffset;
            if (charX + unitPx < leftMargin || charX > width()) continue;

            if (unitPx >= 14.0) {
                QString hex = QString("%1").arg(static_cast<unsigned char>(m_data[i]), 2, 16, QChar('0')).toUpper();
                QRect charRect(static_cast<int>(charX), blockY + 4, static_cast<int>(unitPx), blockHeight - 8);
                painter.drawText(charRect, Qt::AlignCenter, hex);
            }
        }
        fieldCount = m_data.size();
    }

    /* 更新渲染统计 */
    m_stats.totalFieldsRendered += fieldCount;
    emit packetRendered(fieldCount);
    ++m_stats.totalPacketsVisualized;

    painter.end();
}
