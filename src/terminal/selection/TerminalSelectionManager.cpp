/**
 * @file TerminalSelectionManager.cpp
 * @brief 终端选区管理器实现 - 鼠标选择交互和选区文本提取
 *
 * 处理鼠标按下/移动/释放事件来维护选区状态，
 * 并根据缓存行数据和方向过滤索引提取选中文本。
 * 选区操作统计在选区创建和复制时自动更新。
 */

#include "terminal/selection/TerminalSelectionManager.h"
#include "terminal/types/DirectionFilter.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造终端选区管理器，初始化选中背景色并监听主题切换信号 */
TerminalSelectionManager::TerminalSelectionManager(QObject* parent)
    : QObject(parent)
    , m_selectionBg(ThemeManager::instance().color(ThemeManager::SemanticColor::TermSelection))
{
    // 主题切换时动态更新选中背景色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        m_selectionBg = ThemeManager::instance().color(ThemeManager::SemanticColor::TermSelection);
    });
}

/**
 * @brief 处理鼠标按下事件，开始选区并记录起始行号
 * @param y 鼠标Y坐标（像素）
 * @param scrollOffset 当前滚动偏移行数
 * @param lineHeight 每行像素高度
 */
void TerminalSelectionManager::onMousePress(double y, int scrollOffset, int lineHeight)
{
    if (lineHeight <= 0) return;  // 防止除零
    m_isSelecting = true;
    int line = scrollOffset + static_cast<int>(y) / lineHeight;
    m_selectionStartLine = line;
    m_selectionEndLine = line;
}

/**
 * @brief 处理鼠标移动事件，正在选择时更新选区结束行号
 * @param y 鼠标Y坐标（像素）
 * @param scrollOffset 当前滚动偏移行数
 * @param lineHeight 每行像素高度
 */
void TerminalSelectionManager::onMouseMove(double y, int scrollOffset, int lineHeight)
{
    if (m_isSelecting && lineHeight > 0) {
        int line = scrollOffset + static_cast<int>(y) / lineHeight;
        if (m_selectionEndLine != line) {
            m_selectionEndLine = line;
            ++m_totalSelectionsChanged;
        }
    }
}

/** @brief 处理鼠标释放事件，结束选区操作并更新选区统计计数器 */
void TerminalSelectionManager::onMouseRelease()
{
    if (m_isSelecting && m_selectionStartLine >= 0 && m_selectionEndLine >= 0
        && m_selectionStartLine != m_selectionEndLine) {
        // 只在有效拖拽选区（起始行不等于结束行）时递增选择计数
        ++m_totalSelections;
    }
    m_isSelecting = false;
}

/**
 * @brief 更新选区字符数统计(累计字符数 + 最大选区长度)
 *
 * 在选区文本确定后调用，将本次选区的字符数累加到 m_totalSelectionChars，
 * 并更新历史最大选区长度 m_maxSelectionLength。
 * @param charCount 本次选区的字符数量
 */
void TerminalSelectionManager::updateSelectionStats(quint64 charCount)
{
    m_totalSelectionChars += charCount;
    if (charCount > m_maxSelectionLength) {
        m_maxSelectionLength = charCount;
    }
}

/**
 * @brief 提取选区范围内的文本内容，支持方向过滤模式和普通模式
 * @param cachedLines 终端缓存行数据
 * @param directionFilter 方向过滤器指针，为nullptr时使用普通模式
 * @return 选中的文本内容，多行以换行符连接，无选区时返回空字符串
 */
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

/** @brief 重置选区状态，清除起始/结束行号并取消选择中标志 */
void TerminalSelectionManager::reset()
{
    m_selectionStartLine = -1;
    m_selectionEndLine = -1;
    m_isSelecting = false;
}

/** @brief 获取规范化后的选区起始行号（始终小于等于结束行号） @return 起始行号 */
int TerminalSelectionManager::normalizedStartLine() const
{
    return qMin(m_selectionStartLine, m_selectionEndLine);
}

/** @brief 获取规范化后的选区结束行号（始终大于等于起始行号） @return 结束行号 */
int TerminalSelectionManager::normalizedEndLine() const
{
    return qMax(m_selectionStartLine, m_selectionEndLine);
}

/** @brief 判断当前是否存在有效选区 @return 起始和结束行号均有效时返回true */
bool TerminalSelectionManager::hasSelection() const
{
    return m_selectionStartLine >= 0 && m_selectionEndLine >= 0;
}

/** @brief 获取选区背景色 @return 选区高亮背景颜色 */
QColor TerminalSelectionManager::selectionBgColor() const
{
    return m_selectionBg;
}

/** @brief 设置选区背景色 @param color 新的选区高亮背景颜色 */
void TerminalSelectionManager::setSelectionBgColor(const QColor& color)
{
    m_selectionBg = color;
}

/**
 * @brief 程序化设置选区范围，直接指定起始和结束行号
 *
 * 同时更新选区统计计数器(选择次数)。
 *
 * @param startLine 选区起始行号
 * @param endLine 选区结束行号
 */
void TerminalSelectionManager::setSelection(int startLine, int endLine)
{
    m_selectionStartLine = startLine;
    m_selectionEndLine = endLine;
    m_isSelecting = false;

    // 程序化设置选区时更新选择统计
    ++m_totalSelections;
    ++m_totalSelectionsChanged;
}

/** @brief 通知复制操作已完成，递增复制计数并更新字符数统计 @param charCount 本次复制的字符数 */
void TerminalSelectionManager::notifyCopyPerformed(quint64 charCount)
{
    ++m_totalCopies;
    if (charCount > 0) {
        updateSelectionStats(charCount);
    }
}

// ── 统计计数器 Getter 实现 ──

/** @brief 获取总选择次数 @return 用户完成选区的总次数 */
quint64 TerminalSelectionManager::totalSelections() const
{
    return m_totalSelections;
}

/** @brief 获取总复制次数 @return 用户执行复制操作的总次数 */
quint64 TerminalSelectionManager::totalCopies() const
{
    return m_totalCopies;
}

/** @brief 获取总选择字符数 @return 历史所有选区字符数之和 */
quint64 TerminalSelectionManager::totalSelectionChars() const
{
    return m_totalSelectionChars;
}

/** @brief 获取最大单次选区长度(字符数) @return 历史最大选区的字符数 */
quint64 TerminalSelectionManager::maxSelectionLength() const
{
    return m_maxSelectionLength;
}

/** @brief 获取选区变更总次数 @return 选区范围发生变化的累计次数 */
quint64 TerminalSelectionManager::totalSelectionsChanged() const
{
    return m_totalSelectionsChanged;
}

/** @brief 重置所有统计计数器为零(选区状态不受影响) */
void TerminalSelectionManager::resetStats()
{
    m_totalSelections = 0;
    m_totalCopies = 0;
    m_totalSelectionChars = 0;
    m_maxSelectionLength = 0;
    m_totalSelectionsChanged = 0;
}
