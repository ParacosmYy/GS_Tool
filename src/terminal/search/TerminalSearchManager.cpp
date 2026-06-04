/**
 * @file TerminalSearchManager.cpp
 * @brief 终端搜索管理器实现 - 文本搜索、匹配存储、导航和搜索历史
 *
 * 支持纯文本/正则/HEX三种搜索模式，在普通模式和方向过滤模式下
 * 均可工作。匹配结果以 SearchMatch 向量存储，支持循环导航。
 * 纯文本模式额外支持大小写敏感和全词匹配选项。
 */

#include "terminal/search/TerminalSearchManager.h"
#include "terminal/types/DirectionFilter.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造终端搜索管理器，初始化搜索高亮和当前匹配颜色，并监听主题切换信号 */
TerminalSearchManager::TerminalSearchManager(QObject* parent)
    : QObject(parent)
    , m_searchHighlightColor(ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight))
    , m_currentMatchColor(ThemeManager::instance().color(ThemeManager::SemanticColor::TermCurrentMatch))
{
    // 主题切换时动态更新搜索高亮颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        m_searchHighlightColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight);
        m_currentMatchColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TermCurrentMatch);
    });
}

/**
 * @brief 设置搜索高亮，根据搜索模式（纯文本/正则/HEX）在缓存行中查找匹配项
 * @param pattern 搜索关键字或正则表达式
 * @param regex 是否启用正则模式
 * @param hex 是否启用HEX模式
 * @param caseSensitive 是否区分大小写(纯文本和正则模式生效)
 * @param wholeWord 是否全词匹配(仅纯文本模式生效)
 * @param cachedLines 终端缓存行数据
 * @param directionFilter 方向过滤器指针，为nullptr时使用普通模式
 * @param modelLineCount 模型总行数
 * @param lineAtFn 根据模型行号获取原始字节数据的回调函数
 * @return 匹配结果总数，搜索模式无效时返回0
 */
int TerminalSearchManager::setSearchHighlight(
    const QString& pattern, bool regex, bool hex,
    bool caseSensitive, bool wholeWord,
    const QVector<CachedLine>& cachedLines,
    const DirectionFilter* directionFilter,
    int modelLineCount,
    const std::function<QByteArray(int)>& lineAtFn)
{
    m_searchPattern = pattern;
    m_searchRegex = regex;
    m_searchHex = hex;
    m_searchCaseSensitive = caseSensitive;
    m_searchWholeWord = wholeWord;
    m_currentMatchIndex = -1;
    m_searchMatches.clear();

    if (pattern.isEmpty() || cachedLines.isEmpty()) {
        ++m_searchErrorCount;  // 搜索参数无效，累计错误计数
        emit searchMatchesChanged(0, -1);
        return 0;
    }

    ++m_totalSearches;  // 每次有效搜索执行，累计搜索次数
    addToHistory(pattern);  // 添加到搜索历史

    // 构建行遍历回调: 每次调用填入 (displayIdx, text)，返回false表示遍历结束
    int cursor = 0;
    const int totalCount = (directionFilter && directionFilter->isFiltered())
                               ? directionFilter->filteredLineCount()
                               : cachedLines.size();

    // HEX模式需要将原始字节转为HEX字符串后再搜索，不能直接用cachedLines.text
    const bool useHexConversion = hex;

    std::function<bool(int*, QString*)> lineProvider;
    if (directionFilter && directionFilter->isFiltered()) {
        // 方向过滤模式: displayIdx → modelLine → text
        lineProvider = [&](int* outIdx, QString* outText) -> bool {
            while (cursor < totalCount) {
                int displayIdx = cursor++;
                int modelLine = directionFilter->modelIndex(displayIdx);
                if (modelLine < 0 || modelLine >= cachedLines.size()) continue;
                *outIdx = displayIdx;
                *outText = useHexConversion ? HexConverter::toHexString(lineAtFn(modelLine))
                                            : cachedLines[modelLine].text;
                return true;
            }
            return false;
        };
    } else {
        // 普通模式: 直接遍历缓存行
        lineProvider = [&](int* outIdx, QString* outText) -> bool {
            while (cursor < totalCount) {
                int i = cursor++;
                *outIdx = i;
                *outText = useHexConversion ? HexConverter::toHexString(lineAtFn(i))
                                            : cachedLines[i].text;
                return true;
            }
            return false;
        };
    }

    // 按搜索模式分发到对应的构建方法
    bool valid = true;
    if (hex) {
        valid = buildHexSearch(pattern, lineProvider);
    } else if (regex) {
        valid = buildRegexSearch(pattern, caseSensitive, lineProvider);
    } else {
        buildPlainSearch(pattern, caseSensitive, wholeWord, lineProvider);
    }

    if (!valid) {
        ++m_searchErrorCount;  // 搜索模式无效(正则/HEX解析失败)，累计错误计数
        emit searchMatchesChanged(0, -1);
        return 0;
    }

    if (!m_searchMatches.isEmpty()) {
        m_currentMatchIndex = 0;
    }

    m_totalMatches += static_cast<quint64>(m_searchMatches.size());
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches.size();
}

// ---- 搜索构建器/导航/getter/统计已拆分至 TerminalSearchBuilders.cpp ----

/**
 * @brief 将搜索关键字添加到历史记录
 *
 * 去重逻辑: 如果关键字已存在则移到最前，保持最近搜索在前。
 * 历史列表上限 kMaxSearchHistory 条，超出时移除最旧的条目。
 * @param pattern 搜索关键字
 */
void TerminalSearchManager::addToHistory(const QString& pattern)
{
    if (pattern.isEmpty()) return;

    // 如果已存在则移除旧位置(后续添加到最前)
    m_searchHistory.removeAll(pattern);

    // 添加到最前面
    m_searchHistory.prepend(pattern);

    // 超出上限时移除最旧的条目
    while (m_searchHistory.size() > kMaxSearchHistory) {
        m_searchHistory.removeLast();
    }

    emit searchHistoryChanged(m_searchHistory);
}

/** @brief 清除搜索高亮，重置搜索模式、匹配结果和当前匹配索引，并发射搜索变化信号 */
void TerminalSearchManager::clearSearchHighlight()
{
    m_searchPattern.clear();
    m_searchMatches.clear();
    m_currentMatchIndex = -1;
    emit searchMatchesChanged(0, -1);
}

