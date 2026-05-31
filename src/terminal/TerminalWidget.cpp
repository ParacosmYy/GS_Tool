#include "TerminalWidget.h"
#include "utils/HexConverter.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QRegularExpression>

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
{
    // 设置等宽字体
    m_font = QFont("Consolas", 10);
    m_font.setStyleHint(QFont::Monospace);
    m_fontMetrics = QFontMetrics(m_font);

    // 暗色主题配色
    m_bgColor = QColor(30, 30, 46);         // #1e1e2e
    m_rxColor = QColor(205, 214, 244);       // #cdd6f4 接收数据(白色)
    m_txColor = QColor(166, 227, 161);       // #a6e3a1 发送数据(绿色)
    m_timestampColor = QColor(147, 153, 178); // #9399b2 时间戳(灰色)
    m_selectionBg = QColor(69, 71, 90);      // #45475a 选中背景

    // 搜索高亮配色
    m_searchHighlightColor = QColor(249, 226, 175, 80);  // #f9e2af 半透明黄
    m_currentMatchColor = QColor(249, 226, 175, 180);    // #f9e2af 高亮当前匹配

    // 基础设置
    setFont(m_font);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // 计算行高
    m_lineHeight = m_fontMetrics.height() + 2;

    setMinimumSize(400, 200);
}

void TerminalWidget::setModel(TerminalModel* model)
{
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    m_model = model;
    if (m_model) {
        connect(m_model, &TerminalModel::dataAppended,
                this, &TerminalWidget::onDataAppended);
        connect(m_model, &TerminalModel::dataCleared,
                this, &TerminalWidget::onDataCleared);
    }
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_filteredIndices.clear();
    update();
}

void TerminalWidget::setDirectionFilter(DataDirection direction)
{
    m_directionFiltered = true;
    m_filterDirection = direction;
    // 强制重建过滤索引和缓存
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_filteredIndices.clear();
    update();
}

void TerminalWidget::clearDirectionFilter()
{
    m_directionFiltered = false;
    // 强制重建缓存
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_filteredIndices.clear();
    update();
}

void TerminalWidget::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
    m_cachedLineCount = 0;  // 清缓存，强制重新格式化
    update();
}

DisplayMode TerminalWidget::displayMode() const
{
    return m_displayMode;
}

void TerminalWidget::setShowTimestamp(bool show)
{
    m_showTimestamp = show;
    m_cachedLineCount = 0;
    update();
}

bool TerminalWidget::showTimestamp() const
{
    return m_showTimestamp;
}

void TerminalWidget::setShowDirectionPrefix(bool show)
{
    m_showDirectionPrefix = show;
    m_cachedLineCount = 0;
    update();
}

bool TerminalWidget::showDirectionPrefix() const
{
    return m_showDirectionPrefix;
}

void TerminalWidget::setAutoScroll(bool autoScroll)
{
    m_autoScroll = autoScroll;
    if (m_autoScroll) {
        // 跳到最新
        m_scrollOffset = m_maxScrollOffset;
        update();
    }
}

bool TerminalWidget::autoScroll() const
{
    return m_autoScroll;
}

void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_filteredIndices.clear();
    update();
}

QString TerminalWidget::selectedText() const
{
    if (m_selectionStartLine < 0 || m_selectionEndLine < 0) return {};

    int start = qMin(m_selectionStartLine, m_selectionEndLine);
    int end = qMax(m_selectionStartLine, m_selectionEndLine);

    // 方向过滤模式: 从过滤索引表获取选中文本
    if (m_directionFiltered) {
        if (m_filteredIndices.isEmpty()) return {};
        end = qMin(end, m_filteredIndices.size() - 1);
        if (start >= m_filteredIndices.size()) return {};

        QStringList lines;
        for (int i = start; i <= end; ++i) {
            int modelLine = m_filteredIndices[i];
            if (modelLine >= 0 && modelLine < m_cachedLines.size()) {
                lines << m_cachedLines[modelLine].text;
            }
        }
        return lines.join('\n');
    }

    // 普通模式: 直接从缓存获取
    if (m_cachedLines.isEmpty()) return {};
    end = qMin(end, m_cachedLines.size() - 1);
    if (start >= m_cachedLines.size()) return {};

    QStringList lines;
    for (int i = start; i <= end; ++i) {
        lines << m_cachedLines[i].text;
    }
    return lines.join('\n');
}

QSize TerminalWidget::sizeHint() const
{
    return QSize(800, 600);
}

// ---- 单行绘制辅助方法 ----
// 从 paintEvent 中提取的公共渲染逻辑，普通模式和过滤模式共用
int TerminalWidget::paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine)
{
    // 计算时间戳偏移（后续绘制搜索高亮和数据内容都需要此偏移）
    int xOffset = 0;
    if (m_showTimestamp) {
        painter.setPen(m_timestampColor);
        QString ts = QDateTime::fromMSecsSinceEpoch(cached.timestamp)
                         .toString("HH:mm:ss.zzz");
        painter.drawText(4, y + m_lineHeight - 4, ts);
        xOffset = m_fontMetrics.horizontalAdvance(ts) + 12;
    }

    // 选择背景色（使用正规化范围，支持反向拖选）
    int selStart = qMin(m_selectionStartLine, m_selectionEndLine);
    int selEnd = qMax(m_selectionStartLine, m_selectionEndLine);
    if (m_selectionStartLine >= 0 && displayLine >= selStart && displayLine <= selEnd) {
        painter.fillRect(0, y, width(), m_lineHeight, m_selectionBg);
    }

    // 搜索高亮: 绘制匹配区域背景
    if (!m_searchMatches.isEmpty()) {
        for (int mi = 0; mi < m_searchMatches.size(); ++mi) {
            const auto& match = m_searchMatches[mi];
            if (match.line != displayLine) continue;
            int xStart = xOffset + 4 + m_fontMetrics.horizontalAdvance(cached.text.left(match.startCol));
            int matchWidth = m_fontMetrics.horizontalAdvance(cached.text.mid(match.startCol, match.length));
            QColor highlightColor = (mi == m_currentMatchIndex)
                ? m_currentMatchColor : m_searchHighlightColor;
            painter.fillRect(xStart, y + 2, matchWidth, m_lineHeight - 4, highlightColor);
        }
    }

    // 方向前缀 "[TX:] " / "[RX:] " 用不同颜色渲染
    static const QString kTxPrefix = QStringLiteral("[TX:] ");
    static const QString kRxPrefix = QStringLiteral("[RX:] ");

    if (m_showDirectionPrefix) {
        const QString& prefix = (cached.direction == DataDirection::Tx)
            ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            // 前缀颜色: TX用稍暗的绿色区分数据，RX用稍暗的蓝色区分数据
            QColor prefixColor = (cached.direction == DataDirection::Tx)
                ? QColor(129, 199, 123)   // #81c77b -- 比m_txColor(#a6e3a1)稍暗
                : QColor(166, 173, 200);  // #a6adc8 -- 比m_rxColor(#cdd6f4)稍暗

            // 绘制前缀（着色）
            painter.setPen(prefixColor);
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, prefix);

            // 绘制前缀后的数据内容（用方向颜色）
            int prefixWidth = m_fontMetrics.horizontalAdvance(prefix);
            if (cached.direction == DataDirection::Tx) {
                painter.setPen(m_txColor);
            } else {
                painter.setPen(m_rxColor);
            }
            painter.drawText(xOffset + 4 + prefixWidth, y + m_lineHeight - 4,
                             cached.text.mid(prefix.length()));
        } else {
            // 缓存文本不以方向前缀开头（不应出现），正常绘制
            if (cached.direction == DataDirection::Tx) {
                painter.setPen(m_txColor);
            } else {
                painter.setPen(m_rxColor);
            }
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
        }
    } else {
        // 无方向前缀时，按方向着色，整体绘制
        if (cached.direction == DataDirection::Tx) {
            painter.setPen(m_txColor);
        } else {
            painter.setPen(m_rxColor);
        }
        painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
    }
    return y + m_lineHeight;
}

void TerminalWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 填充背景
    painter.fillRect(rect(), m_bgColor);
    painter.setFont(m_font);

    if (!m_model) {
        // 没有模型时显示提示
        painter.setPen(m_timestampColor);
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("未连接 - 等待数据..."));
        return;
    }

    // 获取模型数据行数（不拷贝数据）
    int modelTotalLines = m_model->lineCount();

    // ---- 方向过滤模式: 增量构建过滤索引表，只保留匹配方向的行 ----
    if (m_directionFiltered) {
        // 环形缓冲区回绕检测: 模型行数减少说明旧数据被驱逐，需全量重建
        if (m_cachedLineCount > modelTotalLines) {
            m_cachedLineCount = 0;
            m_cachedLines.clear();
            m_filteredIndices.clear();
        }

        // 增量构建: 只处理新增的模型行
        if (m_cachedLineCount != modelTotalLines) {
            m_cachedLines.resize(modelTotalLines);
            for (int i = m_cachedLineCount; i < modelTotalLines; ++i) {
                m_cachedLines[i] = formatToCache(m_model->lineAt(i));
                // 只将匹配过滤方向的行号加入索引表
                if (m_cachedLines[i].direction == m_filterDirection) {
                    m_filteredIndices.append(i);
                }
            }
            m_cachedLineCount = modelTotalLines;
        }

        // 过滤后的总行数
        int totalLines = m_filteredIndices.size();

        // 缓存更新后重新搜索
        if (!m_searchPattern.isEmpty() && totalLines > 0) {
            QString pat = m_searchPattern;
            bool rx = m_searchRegex;
            bool hx = m_searchHex;
            m_searchPattern.clear();
            setSearchHighlight(pat, rx, hx);
        }

        // 更新最大滚动偏移（基于过滤后的行数）
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }

        // 计算可见范围（基于过滤后的行号）
        int startLine = m_scrollOffset;
        int endLine = qMin(startLine + m_visibleLines + 1, totalLines);

        // 逐行绘制 -- 通过过滤索引表映射到模型行
        int y = 0;
        for (int i = startLine; i < endLine; ++i) {
            int modelLine = m_filteredIndices[i];
            const CachedLine& cached = m_cachedLines[modelLine];
            y = paintLine(painter, cached, y, i);
        }
        return;
    }

    // ---- 普通模式（无方向过滤）: 显示所有数据 ----
    int totalLines = modelTotalLines;

    // 如果缓存失效，只格式化新增的行（增量更新）
    if (m_cachedLineCount != totalLines) {
        // 环形缓冲区回绕检测：当行数减少时说明旧数据被驱逐，逻辑索引已偏移，必须清空整个缓存
        if (m_cachedLineCount > totalLines) {
            m_cachedLineCount = 0;
            m_cachedLines.clear();
        }
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i) {
            m_cachedLines[i] = formatToCache(m_model->lineAt(i));
        }
        m_cachedLineCount = totalLines;

        // 缓存更新后重新搜索（确保搜索结果与缓存内容同步）
        if (!m_searchPattern.isEmpty() && m_cachedLineCount > 0) {
            // 直接在此处执行搜索，不调用refreshSearch()避免递归
            QString pat = m_searchPattern;
            bool rx = m_searchRegex;
            bool hx = m_searchHex;
            m_searchPattern.clear();
            setSearchHighlight(pat, rx, hx);
        }

        // 更新最大滚动偏移
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }

    // 计算可见范围
    int startLine = m_scrollOffset;
    int endLine = qMin(startLine + m_visibleLines + 1, totalLines);

    // 逐行绘制 -- 完全从缓存读取，不调用lineAt()
    int y = 0;
    for (int i = startLine; i < endLine; ++i) {
        const CachedLine& cached = m_cachedLines[i];
        y = paintLine(painter, cached, y, i);
    }
}

void TerminalWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateVisibleRange();
}

void TerminalWidget::wheelEvent(QWheelEvent* event)
{
    // 鼠标滚轮滚动
    int delta = event->angleDelta().y();
    int scrollLines = delta / 120 * 3;  // 每次滚轮滚动3行

    m_scrollOffset -= scrollLines;
    m_scrollOffset = qMax(0, qMin(m_scrollOffset, m_maxScrollOffset));

    // 滚动时关闭自动滚动
    if (delta < 0 && m_scrollOffset < m_maxScrollOffset) {
        m_autoScroll = false;
    }

    update();
    event->accept();
}

void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelecting = true;
        int line = m_scrollOffset + event->position().y() / m_lineHeight;
        m_selectionStartLine = line;
        m_selectionEndLine = line;
        update();
    }
    QWidget::mousePressEvent(event);
}

void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isSelecting) {
        int line = m_scrollOffset + event->position().y() / m_lineHeight;
        m_selectionEndLine = line;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelecting = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    // Ctrl+C 复制选中内容
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        QString text = selectedText();
        if (!text.isEmpty()) {
            QApplication::clipboard()->setText(text);
        }
        return;
    }
    // F3 / Shift+F3 搜索导航
    if (event->key() == Qt::Key_F3) {
        if (event->modifiers() & Qt::ShiftModifier) {
            gotoPrevMatch();
        } else {
            gotoNextMatch();
        }
        return;
    }
    QWidget::keyPressEvent(event);
}

void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine);
    Q_UNUSED(count);

    // 方向过滤模式: 不在此处更新滚动偏移，由paintEvent统一处理
    // 因为过滤后的行数可能与模型行数不同
    if (m_directionFiltered) {
        update();
        return;
    }

    if (m_model) {
        int totalLines = m_model->lineCount();
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }

    update();
}

void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_filteredIndices.clear();
    m_scrollOffset = 0;
    m_maxScrollOffset = 0;
    m_selectionStartLine = -1;
    m_selectionEndLine = -1;
    update();
}

void TerminalWidget::updateVisibleRange()
{
    m_visibleLines = height() / m_lineHeight;
    if (m_model) {
        if (m_directionFiltered) {
            // 方向过滤模式: 基于过滤后的行数计算滚动范围
            m_maxScrollOffset = qMax(0, m_filteredIndices.size() - m_visibleLines);
        } else {
            m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        }
        m_scrollOffset = qMin(m_scrollOffset, m_maxScrollOffset);
    }
}

void TerminalWidget::setSearchHighlight(const QString& pattern, bool regex, bool hex)
{
    m_searchPattern = pattern;
    m_searchRegex = regex;
    m_searchHex = hex;
    m_currentMatchIndex = -1;
    m_searchMatches.clear();

    if (pattern.isEmpty() || m_cachedLines.isEmpty()) {
        emit searchMatchesChanged(0, -1);
        update();
        return;
    }

    QString searchStr = pattern;

    // 方向过滤模式: 只在过滤后的行中搜索
    if (m_directionFiltered) {
        if (hex) {
            QByteArray bytes = HexConverter::fromHexString(pattern);
            if (bytes.isEmpty()) {
                emit searchMatchesChanged(0, -1);
                update();
                return;
            }
            for (int displayIdx = 0; displayIdx < m_filteredIndices.size(); ++displayIdx) {
                int modelLine = m_filteredIndices[displayIdx];
                QString hexText = HexConverter::toHexString(
                    m_model ? m_model->lineAt(modelLine).data : QByteArray());
                int pos = 0;
                while ((pos = hexText.indexOf(searchStr, pos, Qt::CaseInsensitive)) >= 0) {
                    m_searchMatches.append({displayIdx, pos, (int)searchStr.length()});
                    pos += (int)searchStr.length();
                }
            }
        } else if (regex) {
            QRegularExpression re(pattern);
            if (!re.isValid()) {
                emit searchMatchesChanged(0, -1);
                update();
                return;
            }
            for (int displayIdx = 0; displayIdx < m_filteredIndices.size(); ++displayIdx) {
                int modelLine = m_filteredIndices[displayIdx];
                const QString& text = m_cachedLines[modelLine].text;
                QRegularExpressionMatchIterator it = re.globalMatch(text);
                while (it.hasNext()) {
                    auto match = it.next();
                    m_searchMatches.append({displayIdx, (int)match.capturedStart(), (int)match.capturedLength()});
                }
            }
        } else {
            for (int displayIdx = 0; displayIdx < m_filteredIndices.size(); ++displayIdx) {
                int modelLine = m_filteredIndices[displayIdx];
                const QString& text = m_cachedLines[modelLine].text;
                int pos = 0;
                while ((pos = text.indexOf(pattern, pos)) >= 0) {
                    m_searchMatches.append({displayIdx, pos, (int)pattern.length()});
                    pos += pattern.length();
                }
            }
        }
    } else {
        // 普通模式: 在全部缓存行中搜索
        if (hex) {
            QByteArray bytes = HexConverter::fromHexString(pattern);
            if (bytes.isEmpty()) {
                emit searchMatchesChanged(0, -1);
                update();
                return;
            }
            for (int i = 0; i < m_cachedLines.size(); ++i) {
                QString hexText = HexConverter::toHexString(
                    m_model ? m_model->lineAt(i).data : QByteArray());
                int pos = 0;
                while ((pos = hexText.indexOf(searchStr, pos, Qt::CaseInsensitive)) >= 0) {
                    m_searchMatches.append({i, pos, (int)searchStr.length()});
                    pos += (int)searchStr.length();
                }
            }
        } else if (regex) {
            QRegularExpression re(pattern);
            if (!re.isValid()) {
                emit searchMatchesChanged(0, -1);
                update();
                return;
            }
            for (int i = 0; i < m_cachedLines.size(); ++i) {
                QRegularExpressionMatchIterator it = re.globalMatch(m_cachedLines[i].text);
                while (it.hasNext()) {
                    auto match = it.next();
                    m_searchMatches.append({i, (int)match.capturedStart(), (int)match.capturedLength()});
                }
            }
        } else {
            for (int i = 0; i < m_cachedLines.size(); ++i) {
                const QString& text = m_cachedLines[i].text;
                int pos = 0;
                while ((pos = text.indexOf(pattern, pos)) >= 0) {
                    m_searchMatches.append({i, pos, (int)pattern.length()});
                    pos += pattern.length();
                }
            }
        }
    }

    if (!m_searchMatches.isEmpty()) {
        m_currentMatchIndex = 0;
    }

    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    update();
}

void TerminalWidget::clearSearchHighlight()
{
    m_searchPattern.clear();
    m_searchMatches.clear();
    m_currentMatchIndex = -1;
    emit searchMatchesChanged(0, -1);
    update();
}

int TerminalWidget::searchMatchCount() const
{
    return m_searchMatches.size();
}

int TerminalWidget::currentMatchIndex() const
{
    return m_currentMatchIndex;
}

void TerminalWidget::gotoNextMatch()
{
    if (m_searchMatches.isEmpty()) return;
    m_currentMatchIndex = (m_currentMatchIndex + 1) % m_searchMatches.size();
    // 滚动到匹配行
    const auto& match = m_searchMatches[m_currentMatchIndex];
    scrollToMatch(match.line);
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    update();
}

void TerminalWidget::gotoPrevMatch()
{
    if (m_searchMatches.isEmpty()) return;
    m_currentMatchIndex = (m_currentMatchIndex - 1 + m_searchMatches.size()) % m_searchMatches.size();
    const auto& match = m_searchMatches[m_currentMatchIndex];
    scrollToMatch(match.line);
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    update();
}

void TerminalWidget::scrollToMatch(int line)
{
    if (line < m_scrollOffset || line >= m_scrollOffset + m_visibleLines) {
        m_scrollOffset = qMax(0, line - m_visibleLines / 3);
        m_autoScroll = false;
    }
}

void TerminalWidget::refreshSearch()
{
    if (m_searchPattern.isEmpty()) return;
    // 保存当前参数，调用setSearchHighlight重新扫描
    QString pat = m_searchPattern;
    bool rx = m_searchRegex;
    bool hx = m_searchHex;
    m_searchPattern.clear();  // 防止setSearchHighlight内部重复
    setSearchHighlight(pat, rx, hx);
}

CachedLine TerminalWidget::formatToCache(const TerminalLine& line) const
{
    CachedLine cached;
    cached.direction = line.direction;
    cached.timestamp = line.timestamp.toMSecsSinceEpoch();

    // 方向前缀
    QString prefix;
    if (m_showDirectionPrefix) {
        prefix = (line.direction == DataDirection::Tx) ? "[TX:] " : "[RX:] ";
    }

    switch (m_displayMode) {
    case DisplayMode::Hex:
        cached.text = prefix + HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Mixed:
        cached.text = prefix + QString::fromUtf8(line.data) + "  |  " + HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Decimal: {
        QStringList decBytes;
        for (unsigned char b : line.data) {
            decBytes << QString::number(b);
        }
        cached.text = prefix + decBytes.join(' ');
        break;
    }
    case DisplayMode::Text:
    default:
        cached.text = prefix + QString::fromUtf8(line.data);
        break;
    }

    return cached;
}
