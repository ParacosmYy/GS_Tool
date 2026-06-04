/**
 * @file TerminalSearchBuilders.cpp
 * @brief 终端搜索构建器/导航/getter/统计实现
 *
 * 包含搜索模式构建方法(纯文本/正则/HEX)、匹配导航、状态getter和统计计数。
 * 从 TerminalSearchManager.cpp 拆分，保持主入口点与构建逻辑分离。
 */

#include "terminal/search/TerminalSearchManager.h"
#include "utils/crypto/HexConverter.h"

/**
 * @brief 构建纯文本搜索匹配结果，遍历所有可见行查找关键字出现位置
 *
 * 支持大小写敏感和全词匹配选项:
 *   - 大小写敏感: 使用 Qt::CaseSensitive 进行字符串匹配
 *   - 全词匹配: 使用正则 \b 边界包裹关键字匹配独立单词
 *
 * @param pattern 搜索关键字
 * @param caseSensitive 是否区分大小写
 * @param wholeWord 是否全词匹配
 * @param lineProvider 行数据提供回调，返回显示索引和文本内容
 */
void TerminalSearchManager::buildPlainSearch(
    const QString& pattern,
    bool caseSensitive, bool wholeWord,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    int displayIdx;
    QString text;

    if (wholeWord) {
        // 全词匹配: 使用 \b 边界构建正则表达式
        // 对关键字中的特殊正则字符进行转义，确保安全匹配
        QString escaped = QRegularExpression::escape(pattern);
        QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
        if (!caseSensitive) {
            opts |= QRegularExpression::CaseInsensitiveOption;
        }
        QRegularExpression re(QStringLiteral("\\b%1\\b").arg(escaped), opts);
        if (!re.isValid()) return;

        while (lineProvider(&displayIdx, &text)) {
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                auto match = it.next();
                m_searchMatches.append({displayIdx, static_cast<int>(match.capturedStart()),
                                        static_cast<int>(match.capturedLength())});
            }
        }
    } else {
        // 普通文本搜索: 使用 QString::indexOf 进行直接匹配
        Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        while (lineProvider(&displayIdx, &text)) {
            int pos = 0;
            while ((pos = text.indexOf(pattern, pos, cs)) >= 0) {
                m_searchMatches.append({displayIdx, pos, static_cast<int>(pattern.length())});
                pos += static_cast<int>(pattern.length());
            }
        }
    }
}

/**
 * @brief 构建正则表达式搜索匹配结果，使用QRegularExpression全局匹配
 * @param pattern 正则表达式字符串
 * @param caseSensitive 是否区分大小写
 * @param lineProvider 行数据提供回调，返回显示索引和文本内容
 * @return 正则表达式有效返回true，无效返回false
 */
bool TerminalSearchManager::buildRegexSearch(
    const QString& pattern, bool caseSensitive,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        opts |= QRegularExpression::CaseInsensitiveOption;
    }
    QRegularExpression re(pattern, opts);
    if (!re.isValid()) return false;

    int displayIdx;
    QString text;
    while (lineProvider(&displayIdx, &text)) {
        QRegularExpressionMatchIterator it = re.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            m_searchMatches.append({displayIdx, static_cast<int>(match.capturedStart()),
                                    static_cast<int>(match.capturedLength())});
        }
    }
    return true;
}

/**
 * @brief 构建HEX搜索匹配结果，将用户输入转换为规范化HEX字符串后进行匹配
 * @param pattern HEX字符串（如"AA55"或"AA 55"）
 * @param lineProvider 行数据提供回调，返回显示索引和文本内容
 * @return HEX字符串有效返回true，无效返回false
 */
bool TerminalSearchManager::buildHexSearch(
    const QString& pattern,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    QByteArray bytes = HexConverter::fromHexString(pattern);
    if (bytes.isEmpty()) return false;

    // 将用户输入规范化为空格分隔的大写HEX字符串，与HexConverter::toHexString()输出格式一致
    QString normalized = HexConverter::toHexString(bytes);

    int displayIdx;
    QString text;
    while (lineProvider(&displayIdx, &text)) {
        int pos = 0;
        while ((pos = text.indexOf(normalized, pos)) >= 0) {
            m_searchMatches.append({displayIdx, pos, static_cast<int>(normalized.length())});
            pos += static_cast<int>(normalized.length());
        }
    }
    return true;
}

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
