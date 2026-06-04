/**
 * @file MemoryHexEditor.cpp
 * @brief 十六进制内存编辑器实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 自绘偏移/HEX/ASCII三栏视图，支持键盘导航与编辑、鼠标选区、
 * 字节搜索、多格式导出。仅绘制可见行以保持高效滚动。
 */

#include "utils/hex_editor/MemoryHexEditor.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QTextStream>
#include <QtMath>

#include "core/theme/ThemeManager.h"

// ============================================================
// 构造 / 布局
// ============================================================

MemoryHexEditor::MemoryHexEditor(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("MemoryHexEditor");
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    /* 等宽字体 -- Windows优先Consolas，其余平台回退Courier */
    m_monoFont = QFont("Consolas", 10);
    m_monoFont.setStyleHint(QFont::Monospace);
    if (!QFontInfo(m_monoFont).fixedPitch()) {
        m_monoFont = QFont("Courier New", 10);
    }
    setFont(m_monoFont);
    calculateLayout();
}

void MemoryHexEditor::calculateLayout()
{
    QFontMetrics fm(m_monoFont, this);
    m_charWidth  = fm.horizontalAdvance('0');
    m_lineHeight = fm.height() + 2;

    /* 地址栏: "XXXXXXXX" + 间距 */
    m_addressWidth = m_charWidth * 8 + m_areaGap;

    /* HEX区域: 每字节 "XX " + 行末无尾随空格 */
    m_hexAreaWidth = m_charWidth * (m_bytesPerLine * 3 - 1) + m_areaGap;

    /* ASCII区域: 每字节一个字符 */
    m_asciiAreaWidth = m_charWidth * m_bytesPerLine;

    /* 更新最小尺寸 */
    int w = m_leftMargin + m_addressWidth + m_hexAreaWidth + m_asciiAreaWidth + 4;
    int h = m_lineHeight * 4 + m_topMargin * 2;
    setMinimumSize(w, h);

    update();
}

// ============================================================
// 数据接口
// ============================================================

void MemoryHexEditor::setData(const QByteArray& data)
{
    m_data = data;
    m_cursorPos = 0;
    m_scrollOffset = 0;
    m_selection = {-1, -1};
    m_nibbleIndex = 0;

    m_stats.totalDataLoads++;
    m_stats.bytesViewed += static_cast<quint64>(data.size());

    calculateLayout();
    emit dataChanged();
}

QByteArray MemoryHexEditor::data() const
{
    return m_data;
}

void MemoryHexEditor::setBytesPerLine(int bytes)
{
    if (bytes == 8 || bytes == 16) {
        m_bytesPerLine = bytes;
        calculateLayout();
    }
}

int MemoryHexEditor::bytesPerLine() const
{
    return m_bytesPerLine;
}

void MemoryHexEditor::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
    calculateLayout();
}

void MemoryHexEditor::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

// ============================================================
// 高亮 / 导航
// ============================================================

void MemoryHexEditor::setHighlightRange(int start, int end, const QColor& color)
{
    m_highlights.append({start, end, color});
    update();
}

void MemoryHexEditor::clearHighlights()
{
    m_highlights.clear();
    update();
}

void MemoryHexEditor::scrollToOffset(int offset)
{
    if (m_data.isEmpty()) return;
    offset = qBound(0, offset, m_data.size() - 1);
    int line = offset / m_bytesPerLine;
    m_scrollOffset = qBound(0, line - visibleLines() / 3, maxScrollOffset());
    m_cursorPos = offset;
    m_stats.totalScrollEvents++;
    update();
}

// ============================================================
// 选区
// ============================================================

MemoryHexEditor::Selection MemoryHexEditor::selection() const
{
    return m_selection;
}

QByteArray MemoryHexEditor::selectedBytes() const
{
    if (m_selection.startOffset < 0 || m_selection.endOffset < 0) return {};
    int s = qMin(m_selection.startOffset, m_selection.endOffset);
    int e = qMax(m_selection.startOffset, m_selection.endOffset);
    s = qBound(0, s, m_data.size());
    e = qBound(0, e + 1, m_data.size());
    return m_data.mid(s, e - s);
}

// ============================================================
// 搜索
// ============================================================

int MemoryHexEditor::findNext(const QByteArray& pattern, int fromOffset)
{
    if (pattern.isEmpty() || m_data.isEmpty()) return -1;
    m_stats.totalSearches++;

    int idx = m_data.indexOf(pattern, fromOffset);
    if (idx >= 0) {
        m_selection = {idx, static_cast<int>(idx + pattern.size() - 1)};
        m_cursorPos = idx;
        ensureCursorVisible();
        emit searchFound(idx);
        emit selectionChanged(m_selection.startOffset, m_selection.endOffset);
    }
    update();
    return idx;
}

int MemoryHexEditor::findPrev(const QByteArray& pattern, int fromOffset)
{
    if (pattern.isEmpty() || m_data.isEmpty()) return -1;
    m_stats.totalSearches++;

    if (fromOffset < 0) fromOffset = m_data.size() - 1;
    int idx = m_data.lastIndexOf(pattern, fromOffset);
    if (idx >= 0) {
        m_selection = {idx, static_cast<int>(idx + pattern.size() - 1)};
        m_cursorPos = idx;
        ensureCursorVisible();
        emit searchFound(idx);
        emit selectionChanged(m_selection.startOffset, m_selection.endOffset);
    }
    update();
    return idx;
}

// ============================================================
// 导出
// ============================================================

bool MemoryHexEditor::exportToBin(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(m_data);
    f.close();
    m_stats.totalExports++;
    return true;
}

bool MemoryHexEditor::exportToHex(const QString& filePath)
{
    /* Intel HEX 格式导出: 每行最多16字节的数据记录 + EOF */
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&f);

    int offset = 0;
    while (offset < m_data.size()) {
        int chunk = qMin(16, m_data.size() - offset);
        quint8 len = static_cast<quint8>(chunk);
        quint16 addr = static_cast<quint16>(offset & 0xFFFF);
        quint8 type = 0x00; // 数据记录

        quint8 checksum = len + (addr >> 8) + (addr & 0xFF) + type;
        out << ":";
        out << QString("%1").arg(len, 2, 16, QChar('0')).toUpper();
        out << QString("%1").arg(addr, 4, 16, QChar('0')).toUpper();
        out << QString("%1").arg(type, 2, 16, QChar('0')).toUpper();

        for (int i = 0; i < chunk; ++i) {
            quint8 b = static_cast<quint8>(m_data[offset + i]);
            checksum += b;
            out << QString("%1").arg(b, 2, 16, QChar('0')).toUpper();
        }
        checksum = (~checksum + 1) & 0xFF;
        out << QString("%1").arg(checksum, 2, 16, QChar('0')).toUpper();
        out << "\n";
        offset += chunk;
    }

    /* 扩展线性地址记录(若数据 > 64KB) -- 此处简化，仅写EOF */
    out << ":00000001FF\n";
    f.close();
    m_stats.totalExports++;
    return true;
}

bool MemoryHexEditor::exportToCpp(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&f);

    out << "/* Auto-generated by EmbedDebug MemoryHexEditor */\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "const uint8_t data[" << m_data.size() << "] = {\n";

    for (int i = 0; i < m_data.size(); ++i) {
        if (i % m_bytesPerLine == 0) out << "    ";
        out << "0x" << QString("%1").arg(static_cast<uint8_t>(m_data[i]), 2, 16, QChar('0')).toUpper();
        if (i < m_data.size() - 1) out << ", ";
        if ((i + 1) % m_bytesPerLine == 0 || i == m_data.size() - 1) out << "\n";
    }

    out << "};\n";
    out << "const size_t data_size = " << m_data.size() << ";\n";
    f.close();
    m_stats.totalExports++;
    return true;
}

// ============================================================
// paintEvent -- 核心渲染
// ============================================================

void MemoryHexEditor::paintEvent(QPaintEvent*)
{
    if (m_data.isEmpty()) return;

    QPainter p(this);
    p.setFont(m_monoFont);
    p.setRenderHint(QPainter::Antialiasing, false);

    /* 从 ThemeManager 获取语义颜色 */
    auto& theme = ThemeManager::instance();
    QColor bgCol    = theme.color(ThemeManager::SemanticColor::BgPrimary);
    QColor textCol  = theme.color(ThemeManager::SemanticColor::TextPrimary);
    QColor mutedCol = theme.color(ThemeManager::SemanticColor::TextMuted);
    QColor accentCol= theme.color(ThemeManager::SemanticColor::Accent);
    QColor selCol   = theme.color(ThemeManager::SemanticColor::TermSelection);
    QColor borderCol= theme.color(ThemeManager::SemanticColor::Border);

    p.fillRect(rect(), bgCol);

    QFontMetrics fm(m_monoFont, this);
    int visLines = visibleLines();
    int x = m_leftMargin;
    int y = m_topMargin + fm.ascent();

    /* 选区范围(归一化) */
    int selStart = -1, selEnd = -1;
    if (m_selection.startOffset >= 0 && m_selection.endOffset >= 0) {
        selStart = qMin(m_selection.startOffset, m_selection.endOffset);
        selEnd   = qMax(m_selection.startOffset, m_selection.endOffset);
    }

    for (int line = 0; line < visLines; ++line) {
        int dataOffset = (m_scrollOffset + line) * m_bytesPerLine;
        if (dataOffset >= m_data.size()) break;

        int lineX = x;
        int lineY = m_topMargin + line * m_lineHeight;

        /* ---- 地址列 ---- */
        QString addr = QString("%1").arg(dataOffset, 8, 16, QChar('0')).toUpper();
        p.setPen(mutedCol);
        p.drawText(lineX, y + line * m_lineHeight, addr);
        lineX += m_addressWidth;

        /* ---- HEX列 ---- */
        if (m_displayMode == DisplayMode::HexOnly ||
            m_displayMode == DisplayMode::HexAndAscii) {
            for (int col = 0; col < m_bytesPerLine; ++col) {
                int byteIdx = dataOffset + col;
                if (byteIdx >= m_data.size()) break;

                uint8_t byteVal = static_cast<uint8_t>(m_data[byteIdx]);
                QString hexStr = QString("%1").arg(byteVal, 2, 16, QChar('0')).toUpper();

                /* 背景高亮优先级: 选区 > 自定义 > 光标 */
                bool inSel = (selStart >= 0 && byteIdx >= selStart && byteIdx <= selEnd);
                bool isCur = (byteIdx == m_cursorPos);
                QColor bg;

                if (inSel) {
                    bg = selCol;
                } else {
                    for (const auto& hl : m_highlights) {
                        if (byteIdx >= hl.start && byteIdx <= hl.end) {
                            bg = hl.color;
                            break;
                        }
                    }
                }

                if (bg.isValid()) {
                    QRect bgRect(lineX - 1, lineY, m_charWidth * 2 + 2, m_lineHeight);
                    p.fillRect(bgRect, bg);
                }

                /* 光标所在字节用强调色绘制 */
                if (isCur && !inSel) {
                    QRect curRect(lineX - 1, lineY, m_charWidth * 2 + 2, m_lineHeight);
                    p.fillRect(curRect, accentCol);
                    p.setPen(bgCol);
                } else {
                    p.setPen(textCol);
                }

                p.drawText(lineX, y + line * m_lineHeight, hexStr);
                lineX += m_charWidth * 3;
            }
            lineX += m_areaGap - m_charWidth;
        }

        /* ---- 分隔线 ---- */
        if (m_displayMode == DisplayMode::HexAndAscii) {
            int sepX = m_leftMargin + m_addressWidth + m_hexAreaWidth - m_areaGap / 2;
            p.setPen(borderCol);
            p.drawLine(sepX, lineY, sepX, lineY + m_lineHeight);
        }

        /* ---- ASCII列 ---- */
        if (m_displayMode == DisplayMode::HexAndAscii ||
            m_displayMode == DisplayMode::AsciiOnly) {

            if (m_displayMode == DisplayMode::AsciiOnly) {
                lineX = m_leftMargin + m_addressWidth;
            }

            for (int col = 0; col < m_bytesPerLine; ++col) {
                int byteIdx = dataOffset + col;
                if (byteIdx >= m_data.size()) break;

                char ch = toPrintable(static_cast<uint8_t>(m_data[byteIdx]));

                bool inSel = (selStart >= 0 && byteIdx >= selStart && byteIdx <= selEnd);
                bool isCur = (byteIdx == m_cursorPos);

                if (inSel) {
                    QRect bgRect(lineX, lineY, m_charWidth, m_lineHeight);
                    p.fillRect(bgRect, selCol);
                    p.setPen(textCol);
                } else if (isCur) {
                    QRect bgRect(lineX, lineY, m_charWidth, m_lineHeight);
                    p.fillRect(bgRect, accentCol);
                    p.setPen(bgCol);
                } else {
                    p.setPen(textCol);
                }

                p.drawText(lineX, y + line * m_lineHeight, QString(ch));
                lineX += m_charWidth;
            }
        }
    }

    /* 左侧地址列与HEX列之间的分隔线 */
    if (m_displayMode != DisplayMode::AsciiOnly) {
        int sepX = m_leftMargin + m_addressWidth - m_areaGap / 2;
        p.setPen(borderCol);
        p.drawLine(sepX, 0, sepX, height());
    }
}

// ============================================================
// 鼠标交互
// ============================================================

void MemoryHexEditor::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    int byteIdx = byteAtPos(event->pos());
    if (byteIdx < 0) return;

    m_cursorPos = byteIdx;
    m_selection = {byteIdx, byteIdx};
    m_nibbleIndex = 0;
    emit selectionChanged(byteIdx, byteIdx);
    update();
}

void MemoryHexEditor::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;
    int byteIdx = byteAtPos(event->pos());
    if (byteIdx < 0) return;

    m_selection.endOffset = byteIdx;
    m_cursorPos = byteIdx;
    emit selectionChanged(m_selection.startOffset, m_selection.endOffset);
    update();
}

// ============================================================
// 键盘交互
// ============================================================

void MemoryHexEditor::keyPressEvent(QKeyEvent* event)
{
    if (m_data.isEmpty()) return;

    int key = event->key();
    Qt::KeyboardModifiers mods = event->modifiers();

    /* Ctrl+C: 复制选区 */
    if (mods & Qt::ControlModifier && key == Qt::Key_C) {
        QByteArray sel = selectedBytes();
        if (!sel.isEmpty()) {
            QString hexText;
            for (int i = 0; i < sel.size(); ++i) {
                if (i > 0) hexText += ' ';
                hexText += QString("%1").arg(static_cast<uint8_t>(sel[i]), 2, 16, QChar('0')).toUpper();
            }
            QApplication::clipboard()->setText(hexText);
            m_stats.totalCopies++;
        }
        return;
    }

    /* Ctrl+F: 不在此处理，由外部监听 */
    if (mods & Qt::ControlModifier && key == Qt::Key_F) {
        QWidget::keyPressEvent(event);
        return;
    }

    /* 方向键导航 */
    int newPos = m_cursorPos;
    if (key == Qt::Key_Left)       newPos = qMax(0, m_cursorPos - 1);
    else if (key == Qt::Key_Right) newPos = qMin(m_data.size() - 1, m_cursorPos + 1);
    else if (key == Qt::Key_Up)    newPos = qMax(0, m_cursorPos - m_bytesPerLine);
    else if (key == Qt::Key_Down)  newPos = qMin(m_data.size() - 1, m_cursorPos + m_bytesPerLine);
    else if (key == Qt::Key_Home)  newPos = (m_cursorPos / m_bytesPerLine) * m_bytesPerLine;
    else if (key == Qt::Key_End)   newPos = qMin(m_data.size() - 1,
                                       (m_cursorPos / m_bytesPerLine + 1) * m_bytesPerLine - 1);
    else if (key == Qt::Key_PageUp) {
        newPos = qMax(0, m_cursorPos - m_bytesPerLine * visibleLines());
    } else if (key == Qt::Key_PageDown) {
        newPos = qMin(m_data.size() - 1, m_cursorPos + m_bytesPerLine * visibleLines());
    } else if (!m_readOnly && !mods) {
        /* HEX输入编辑: 0-9, a-f */
        int hexVal = -1;
        if (key >= Qt::Key_0 && key <= Qt::Key_9)
            hexVal = key - Qt::Key_0;
        else if (key >= Qt::Key_A && key <= Qt::Key_F)
            hexVal = key - Qt::Key_A + 10;

        if (hexVal >= 0 && m_cursorPos < m_data.size()) {
            uint8_t oldVal = static_cast<uint8_t>(m_data[m_cursorPos]);
            uint8_t newVal = oldVal;
            if (m_nibbleIndex == 0) {
                newVal = (hexVal << 4) | (oldVal & 0x0F);
            } else {
                newVal = (oldVal & 0xF0) | hexVal;
            }
            m_data[m_cursorPos] = static_cast<char>(newVal);
            m_stats.totalByteEdits++;
            m_stats.bytesEdited++;

            emit byteEdited(m_cursorPos, oldVal, newVal);

            /* 推进光标: 输入两个半字节后移到下一字节 */
            m_nibbleIndex++;
            if (m_nibbleIndex >= 2) {
                m_nibbleIndex = 0;
                m_cursorPos = qMin(m_data.size() - 1, m_cursorPos + 1);
            }
            ensureCursorVisible();
            update();
            emit dataChanged();
            return;
        }
    } else {
        QWidget::keyPressEvent(event);
        return;
    }

    /* Shift+方向键扩展选区 */
    if (mods & Qt::ShiftModifier) {
        if (m_selection.startOffset < 0) m_selection.startOffset = m_cursorPos;
        m_selection.endOffset = newPos;
    } else {
        m_selection = {newPos, newPos};
    }

    m_cursorPos = newPos;
    m_nibbleIndex = 0;
    ensureCursorVisible();
    emit selectionChanged(m_selection.startOffset, m_selection.endOffset);
    update();
}

// ============================================================
// 滚轮
// ============================================================

void MemoryHexEditor::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    int steps = -delta / 120;
    int newOff = qBound(0, m_scrollOffset + steps, maxScrollOffset());
    if (newOff != m_scrollOffset) {
        m_scrollOffset = newOff;
        m_stats.totalScrollEvents++;
        update();
    }
    event->accept();
}

QSize MemoryHexEditor::minimumSizeHint() const
{
    return QSize(m_leftMargin + m_addressWidth + m_hexAreaWidth + m_asciiAreaWidth + 4,
                 m_lineHeight * 4 + m_topMargin * 2);
}

// ============================================================
// 内部辅助
// ============================================================

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
