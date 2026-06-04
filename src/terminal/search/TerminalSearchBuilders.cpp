/**
 * @file TerminalSearchBuilders.cpp
 * @brief 终端搜索导航/状态getter/统计计数实现
 *
 * 从 TerminalSearchManager.cpp 拆分，包含:
 *   - 搜索导航: gotoNextMatch/gotoPrevMatch，支持循环导航
 *   - 状态Getter: 搜索模式/关键字/颜色等查询方法
 *   - 搜索历史: searchHistory/clearSearchHistory
 *   - 统计计数: totalSearches/totalMatches/totalReplacements/resetStats
 *
 * 搜索模式构建方法见TerminalSearchBuild.cpp。
 */

#include "terminal/search/TerminalSearchManager.h"

// ---- 搜索导航 ----

/** @brief 获取当前搜索匹配结果总数 @return 匹配结果数量 */
int TerminalSearchManager::searchMatchCount() const
{
    return m_searchMatches.size();
}

/** @brief 获取当前选中的匹配项索引 @return 当前匹配索引，无匹配时返回-1 */
int TerminalSearchManager::currentMatchIndex() const
{
    return m_currentMatchIndex;
}

/** @brief 跳转到下一个搜索匹配项，支持循环导航 @return 下一个匹配项所在行号，无匹配时返回-1 */
int TerminalSearchManager::gotoNextMatch()
{
    if (m_searchMatches.isEmpty()) return -1;
    m_currentMatchIndex = (m_currentMatchIndex + 1) % m_searchMatches.size();
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches[m_currentMatchIndex].line;
}

/** @brief 跳转到上一个搜索匹配项，支持循环导航 @return 上一个匹配项所在行号，无匹配时返回-1 */
int TerminalSearchManager::gotoPrevMatch()
{
    if (m_searchMatches.isEmpty()) return -1;
    m_currentMatchIndex = (m_currentMatchIndex - 1 + m_searchMatches.size()) % m_searchMatches.size();
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches[m_currentMatchIndex].line;
}

// ---- 状态 Getter ----

/** @brief 获取当前搜索关键字 @return 搜索关键字字符串 */
QString TerminalSearchManager::searchPattern() const
{
    return m_searchPattern;
}

/** @brief 查询当前是否为正则搜索模式 @return 正则模式返回true，否则返回false */
bool TerminalSearchManager::searchRegex() const
{
    return m_searchRegex;
}

/** @brief 查询当前是否为HEX搜索模式 @return HEX模式返回true，否则返回false */
bool TerminalSearchManager::searchHex() const
{
    return m_searchHex;
}

/** @brief 查询当前是否为大小写敏感模式 @return 大小写敏感返回true */
bool TerminalSearchManager::searchCaseSensitive() const
{
    return m_searchCaseSensitive;
}

/** @brief 查询当前是否为全词匹配模式 @return 全词匹配返回true */
bool TerminalSearchManager::searchWholeWord() const
{
    return m_searchWholeWord;
}

/** @brief 获取所有搜索匹配结果的只读引用 @return SearchMatch向量的const引用 */
const QVector<SearchMatch>& TerminalSearchManager::searchMatches() const
{
    return m_searchMatches;
}

/** @brief 获取搜索匹配项的高亮背景色 @return 高亮颜色值 */
QColor TerminalSearchManager::searchHighlightColor() const
{
    return m_searchHighlightColor;
}

/** @brief 获取当前匹配项的高亮背景色（与普通匹配项区分） @return 当前匹配颜色值 */
QColor TerminalSearchManager::currentMatchColor() const
{
    return m_currentMatchColor;
}

/**
 * @brief 设置搜索高亮颜色和当前匹配颜色
 * @param highlight 搜索匹配项的高亮背景色
 * @param current 当前选中匹配项的高亮背景色
 */
void TerminalSearchManager::setSearchColors(const QColor& highlight, const QColor& current)
{
    m_searchHighlightColor = highlight;
    m_currentMatchColor = current;
}

// ---- 搜索历史 ----

/** @brief 获取搜索历史列表(最近的在前) @return 搜索历史字符串列表 */
QStringList TerminalSearchManager::searchHistory() const
{
    return m_searchHistory;
}

/** @brief 清除搜索历史并发射历史变化信号 */
void TerminalSearchManager::clearSearchHistory()
{
    m_searchHistory.clear();
    emit searchHistoryChanged(m_searchHistory);
}

// ---- 统计计数实现 ----

/** @brief 获取累计搜索执行次数 @return 搜索总数 */
quint64 TerminalSearchManager::totalSearches() const { return m_totalSearches; }

/** @brief 获取累计匹配结果总数(跨多次搜索) @return 匹配总数 */
quint64 TerminalSearchManager::totalMatches() const { return m_totalMatches; }

/** @brief 获取累计替换操作次数 @return 替换总数 */
quint64 TerminalSearchManager::totalReplacements() const { return m_totalReplacements; }

/** @brief 获取搜索错误次数(参数无效/正则错误/HEX解析失败) @return 错误总数 */
quint64 TerminalSearchManager::searchErrorCount() const { return m_searchErrorCount; }

/** @brief 重置所有统计计数器为零 */
void TerminalSearchManager::resetStats()
{
    m_totalSearches = 0;
    m_totalMatches = 0;
    m_totalReplacements = 0;
    m_searchErrorCount = 0;
}
