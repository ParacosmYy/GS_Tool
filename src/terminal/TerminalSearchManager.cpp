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

bool TerminalSearchManager::buildHexSearch(
    const QString& pattern,
    const std::function<bool(int*, QString*)>& lineProvider)
{
    QByteArray bytes = HexConverter::fromHexString(pattern);
    if (bytes.isEmpty()) return false;

    int displayIdx;
    QString text;
    while (lineProvider(&displayIdx, &text)) {
        int pos = 0;
        while ((pos = text.indexOf(pattern, pos, Qt::CaseInsensitive)) >= 0) {
            m_searchMatches.append({displayIdx, pos, static_cast<int>(pattern.length())});
            pos += static_cast<int>(pattern.length());
        }
    }
    return true;
}

void TerminalSearchManager::clearSearchHighlight()
{
    m_searchPattern.clear();
    m_searchMatches.clear();
    m_currentMatchIndex = -1;
    emit searchMatchesChanged(0, -1);
}

int TerminalSearchManager::searchMatchCount() const
{
    return m_searchMatches.size();
}

int TerminalSearchManager::currentMatchIndex() const
{
    return m_currentMatchIndex;
}

int TerminalSearchManager::gotoNextMatch()
{
    if (m_searchMatches.isEmpty()) return -1;
    m_currentMatchIndex = (m_currentMatchIndex + 1) % m_searchMatches.size();
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches[m_currentMatchIndex].line;
}

int TerminalSearchManager::gotoPrevMatch()
{
    if (m_searchMatches.isEmpty()) return -1;
    m_currentMatchIndex = (m_currentMatchIndex - 1 + m_searchMatches.size()) % m_searchMatches.size();
    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches[m_currentMatchIndex].line;
}

QString TerminalSearchManager::searchPattern() const
{
    return m_searchPattern;
}

bool TerminalSearchManager::searchRegex() const
{
    return m_searchRegex;
}

bool TerminalSearchManager::searchHex() const
{
    return m_searchHex;
}

const QVector<SearchMatch>& TerminalSearchManager::searchMatches() const
{
    return m_searchMatches;
}

QColor TerminalSearchManager::searchHighlightColor() const
{
    return m_searchHighlightColor;
}

QColor TerminalSearchManager::currentMatchColor() const
{
    return m_currentMatchColor;
}

void TerminalSearchManager::setSearchColors(const QColor& highlight, const QColor& current)
{
    m_searchHighlightColor = highlight;
    m_currentMatchColor = current;
}
