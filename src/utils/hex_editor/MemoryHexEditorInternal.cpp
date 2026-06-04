/**
 * @file MemoryHexEditorInternal.cpp
 * @brief 十六进制内存编辑器 — 内部辅助方法
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从MemoryHexEditor.cpp拆分: 布局计算/坐标转换/滚动/光标定位等辅助方法
 */

#include "utils/hex_editor/MemoryHexEditor.h"

#include <QtMath>

// ============================================================
// 内部辅助
// ============================================================

QSize MemoryHexEditor::minimumSizeHint() const
{
    return QSize(m_leftMargin + m_addressWidth + m_hexAreaWidth + m_asciiAreaWidth + 4,
                 m_lineHeight * 4 + m_topMargin * 2);
}

int MemoryHexEditor::totalLines() const
{
    if (m_data.isEmpty()) return 0;
    return (m_data.size() + m_bytesPerLine - 1) / m_bytesPerLine;
}

int MemoryHexEditor::visibleLines() const
{
    return qMax(1, (height() - m_topMargin * 2) / m_lineHeight);
}

int MemoryHexEditor::lineAtY(int y) const
{
    return (y - m_topMargin) / m_lineHeight;
}

int MemoryHexEditor::byteAtPos(const QPoint& pos) const
{
    if (m_data.isEmpty()) return -1;

    int line = lineAtY(pos.y());
    int dataLine = m_scrollOffset + line;
    if (dataLine < 0 || dataLine >= totalLines()) return -1;

    int lineStartByte = dataLine * m_bytesPerLine;
    int x = pos.x() - m_leftMargin;

    /* 检查HEX区域 */
    if (m_displayMode != DisplayMode::AsciiOnly) {
        int hexStart = m_addressWidth;
        int hexEnd = hexStart + m_hexAreaWidth;
        if (x >= hexStart && x < hexEnd) {
            int charPos = (x - hexStart) / m_charWidth;
            int col = charPos / 3;
            col = qBound(0, col, m_bytesPerLine - 1);
            int byteIdx = lineStartByte + col;
            return (byteIdx < m_data.size()) ? byteIdx : -1;
        }
    }

    /* 检查ASCII区域 */
    if (m_displayMode != DisplayMode::HexOnly) {
        int asciiStart = m_addressWidth + m_hexAreaWidth;
        if (m_displayMode == DisplayMode::AsciiOnly) {
            asciiStart = m_addressWidth;
        }
        if (x >= asciiStart) {
            int col = (x - asciiStart) / m_charWidth;
            col = qBound(0, col, m_bytesPerLine - 1);
            int byteIdx = lineStartByte + col;
            return (byteIdx < m_data.size()) ? byteIdx : -1;
        }
    }

    return -1;
}

void MemoryHexEditor::ensureCursorVisible()
{
    if (m_data.isEmpty()) return;
    int cursorLine = m_cursorPos / m_bytesPerLine;
    int vis = visibleLines();

    if (cursorLine < m_scrollOffset) {
        m_scrollOffset = cursorLine;
    } else if (cursorLine >= m_scrollOffset + vis) {
        m_scrollOffset = cursorLine - vis + 1;
    }
    m_scrollOffset = qBound(0, m_scrollOffset, maxScrollOffset());
}

int MemoryHexEditor::maxScrollOffset() const
{
    int total = totalLines();
    int vis = visibleLines();
    return qMax(0, total - vis);
}

char MemoryHexEditor::toPrintable(uint8_t byte)
{
    return (byte >= 0x20 && byte <= 0x7E) ? static_cast<char>(byte) : '.';
}
