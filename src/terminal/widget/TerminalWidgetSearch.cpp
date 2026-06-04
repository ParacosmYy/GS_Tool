/**
 * @file TerminalWidgetSearch.cpp
 * @brief 终端控件搜索功能实现 — 搜索高亮设置、导航和缓存刷新
 *
 * 从 TerminalWidget.cpp 拆分而来，包含搜索高亮设置、匹配导航
 * (gotoNextMatch/gotoPrevMatch)、scrollToMatch和缓存更新后搜索刷新。
 */

#include "terminal/widget/TerminalWidget.h"

/** @brief 设置搜索高亮(支持正则/HEX/普通文本/大小写敏感/全词匹配) @param pattern 搜索模式 @param regex 是否正则 @param hex 是否HEX模式 @param caseSensitive 是否区分大小写 @param wholeWord 是否全词匹配 */
void TerminalWidget::setSearchHighlight(const QString& pattern, bool regex, bool hex,
                                        bool caseSensitive, bool wholeWord)
{
    auto lineAtFn = [this](int idx) -> QByteArray {
        return m_model ? m_model->lineAt(idx).data : QByteArray();
    };
    m_searchManager->setSearchHighlight(pattern, regex, hex, caseSensitive, wholeWord,
                                         m_cachedLines, m_directionFilter,
                                         m_cachedLineCount, lineAtFn);
    update();
}

/** @brief 清除搜索高亮并重绘 */
void TerminalWidget::clearSearchHighlight() { m_searchManager->clearSearchHighlight(); update(); }

/** @brief 返回搜索匹配总数 @return 匹配数量 */
int TerminalWidget::searchMatchCount() const { return m_searchManager->searchMatchCount(); }

/** @brief 返回当前高亮的匹配索引 @return 当前索引 */
int TerminalWidget::currentMatchIndex() const { return m_searchManager->currentMatchIndex(); }

/** @brief 返回搜索管理器指针(用于外部连接搜索历史信号) @return 搜索管理器 */
TerminalSearchManager* TerminalWidget::searchManager() const { return m_searchManager; }

/** @brief 跳转到下一个搜索匹配项 */
void TerminalWidget::gotoNextMatch()
{
    ++m_totalMatchNavigations;  ///< 统计: 匹配导航递增
    int line = m_searchManager->gotoNextMatch();
    if (line >= 0) scrollToMatch(line); update();
}

/** @brief 跳转到上一个搜索匹配项 */
void TerminalWidget::gotoPrevMatch()
{
    ++m_totalMatchNavigations;  ///< 统计: 匹配导航递增
    int line = m_searchManager->gotoPrevMatch();
    if (line >= 0) scrollToMatch(line); update();
}

/** @brief 滚动到指定行并确保可见 @param line 目标行号 */
void TerminalWidget::scrollToMatch(int line)
{
    if (line < m_scrollOffset || line >= m_scrollOffset + m_visibleLines) {
        m_scrollOffset = qMax(0, line - m_visibleLines / 3);
        m_autoScroll = false;
    }
}

/** @brief 缓存更新后刷新搜索匹配(重新计算所有匹配位置) */
void TerminalWidget::refreshSearchAfterCacheUpdate()
{
    if (m_searchManager->searchPattern().isEmpty()) return;
    QString pat = m_searchManager->searchPattern();
    bool rx = m_searchManager->searchRegex(), hx = m_searchManager->searchHex();
    bool cs = m_searchManager->searchCaseSensitive(), ww = m_searchManager->searchWholeWord();
    setSearchHighlight(pat, rx, hx, cs, ww);
}
