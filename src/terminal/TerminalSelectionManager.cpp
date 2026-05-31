/**
 * @file TerminalSelectionManager.cpp
 * @brief 终端选区管理器实现 - 鼠标选择交互和选区文本提取
 *
 * 处理鼠标按下/移动/释放事件来维护选区状态，
 * 并根据缓存行数据和方向过滤索引提取选中文本。
 */

#include "terminal/TerminalSelectionManager.h"
#include "terminal/DirectionFilter.h"
#include "core/ThemeManager.h"

TerminalSelectionManager::TerminalSelectionManager(QObject* parent)
    : QObject(parent)
    , m_selectionBg(ThemeManager::instance().color(ThemeManager::SemanticColor::TermSelection))
{
    // 主题切换时动态更新选中背景色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        m_selectionBg = ThemeManager::instance().color(ThemeManager::SemanticColor::TermSelection);
    });
}

void TerminalSelectionManager::onMousePress(double y, int scrollOffset, int lineHeight)
{
    if (lineHeight <= 0) return;  // 防止除零
    m_isSelecting = true;
    int line = scrollOffset + static_cast<int>(y) / lineHeight;
    m_selectionStartLine = line;
    m_selectionEndLine = line;
}

void TerminalSelectionManager::onMouseMove(double y, int scrollOffset, int lineHeight)
{
    if (m_isSelecting && lineHeight > 0) {
        int line = scrollOffset + static_cast<int>(y) / lineHeight;
        m_selectionEndLine = line;
    }
}

void TerminalSelectionManager::onMouseRelease()
{
    m_isSelecting = false;
}

QString TerminalSelectionManager::selectedText(
    const QVector<CachedLine>& cachedLines,
    const DirectionFilter* directionFilter) const
{
    if (m_selectionStartLine < 0 || m_selectionEndLine < 0) return {};

    int start = qMin(m_selectionStartLine, m_selectionEndLine);
    int end = qMax(m_selectionStartLine, m_selectionEndLine);

    // 方向过滤模式: 从过滤索引表获取选中文本
    if (directionFilter && directionFilter->isFiltered()) {
        int filteredCount = directionFilter->filteredLineCount();
        if (filteredCount == 0) return {};
        end = qMin(end, filteredCount - 1);
        if (start >= filteredCount) return {};

        QStringList lines;
        for (int i = start; i <= end; ++i) {
            int modelLine = directionFilter->modelIndex(i);
            if (modelLine >= 0 && modelLine < cachedLines.size()) {
                lines << cachedLines[modelLine].text;
            }
        }
        return lines.join('\n');
    }

    // 普通模式: 直接从缓存获取
    if (cachedLines.isEmpty()) return {};
    end = qMin(end, cachedLines.size() - 1);
    if (start >= cachedLines.size()) return {};

    QStringList lines;
    for (int i = start; i <= end; ++i) {
        lines << cachedLines[i].text;
    }
    return lines.join('\n');
}

void TerminalSelectionManager::reset()
{
    m_selectionStartLine = -1;
    m_selectionEndLine = -1;
    m_isSelecting = false;
}

int TerminalSelectionManager::normalizedStartLine() const
{
    return qMin(m_selectionStartLine, m_selectionEndLine);
}

int TerminalSelectionManager::normalizedEndLine() const
{
    return qMax(m_selectionStartLine, m_selectionEndLine);
}

bool TerminalSelectionManager::hasSelection() const
{
    return m_selectionStartLine >= 0 && m_selectionEndLine >= 0;
}

QColor TerminalSelectionManager::selectionBgColor() const
{
    return m_selectionBg;
}

void TerminalSelectionManager::setSelectionBgColor(const QColor& color)
{
    m_selectionBg = color;
}
