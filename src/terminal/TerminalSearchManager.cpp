/**
 * @file TerminalSearchManager.cpp
 * @brief 终端搜索管理器实现 - 文本搜索、匹配存储和导航
 *
 * 支持纯文本/正则/HEX三种搜索模式，在普通模式和方向过滤模式下
 * 均可工作。匹配结果以 SearchMatch 向量存储，支持循环导航。
 */

#include "terminal/TerminalSearchManager.h"
#include "terminal/DirectionFilter.h"
#include "utils/HexConverter.h"
#include "core/ThemeManager.h"

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
 * @param cachedLines 终端缓存行数据
 * @param directionFilter 方向过滤器指针，为nullptr时使用普通模式
 * @param modelLineCount 模型总行数
 * @param lineAtFn 根据模型行号获取原始字节数据的回调函数
 * @return 匹配结果总数，搜索模式无效时返回0
 */
int TerminalSearchManager::setSearchHighlight(
    const QString& pattern, bool regex, bool hex,
    const QVector<CachedLine>& cachedLines,
    const DirectionFilter* directionFilter,
    int modelLineCount,
    const std::function<QByteArray(int)>& lineAtFn)
{
    m_searchPattern = pattern;
    m_searchRegex = regex;
    m_searchHex = hex;
    m_currentMatchIndex = -1;
    m_searchMatches.clear();

    if (pattern.isEmpty() || cachedLines.isEmpty()) {
        emit searchMatchesChanged(0, -1);
        return 0;
    }

    // 构建行遍历回调: 每次调用填入 (displayIdx, text)，返回false表示遍历结束
    // 方向过滤模式从过滤映射获取行，普通模式直接遍历缓存
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
        valid = buildRegexSearch(pattern, lineProvider);
    } else {
        buildPlainSearch(pattern, lineProvider);
    }

    if (!valid) {
        emit searchMatchesChanged(0, -1);
        return 0;
    }

    if (!m_searchMatches.isEmpty()) {
        m_currentMatchIndex = 0;
    }

    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches.size();
}

/**
 * @brief 构建纯文本搜索匹配结果，遍历所有可见行查找关键字出现位置
 * @param pattern 搜索关键字
 * @param lineProvider 行数据提供回调，返回显示索引和文本内容
 */
void TerminalSearchManager::buildPlainSearch(
    const QString& pattern,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    int displayIdx;
    QString text;
    while (lineProvider(&displayIdx, &text)) {
        int pos = 0;
        while ((pos = text.indexOf(pattern, pos)) >= 0) {
            m_searchMatches.append({displayIdx, pos, static_cast<int>(pattern.length())});
            pos += static_cast<int>(pattern.length());
        }
    }
}

/**
 * @brief 构建正则表达式搜索匹配结果，使用QRegularExpression全局匹配
 * @param pattern 正则表达式字符串
 * @param lineProvider 行数据提供回调，返回显示索引和文本内容
 * @return 正则表达式有效返回true，无效返回false
 */
bool TerminalSearchManager::buildRegexSearch(
    const QString& pattern,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    QRegularExpression re(pattern);
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
    // 例如: "AA55" → "AA 55", "aa 55" → "AA 55"
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

/** @brief 清除搜索高亮，重置搜索模式、匹配结果和当前匹配索引，并发射搜索变化信号 */
void TerminalSearchManager::clearSearchHighlight()
{
    m_searchPattern.clear();
    m_searchMatches.clear();
    m_currentMatchIndex = -1;
    emit searchMatchesChanged(0, -1);
}

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
