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

    QString searchStr = pattern;

    // 方向过滤模式: 只在过滤后的行中搜索
    if (directionFilter && directionFilter->isFiltered()) {
        int filteredCount = directionFilter->filteredLineCount();
        if (hex) {
            QByteArray bytes = HexConverter::fromHexString(pattern);
            if (bytes.isEmpty()) {
                emit searchMatchesChanged(0, -1);
                return 0;
            }
            for (int displayIdx = 0; displayIdx < filteredCount; ++displayIdx) {
                int modelLine = directionFilter->modelIndex(displayIdx);
                if (modelLine < 0 || modelLine >= cachedLines.size()) continue;
                QString hexText = HexConverter::toHexString(lineAtFn(modelLine));
                int pos = 0;
                while ((pos = hexText.indexOf(searchStr, pos, Qt::CaseInsensitive)) >= 0) {
                    m_searchMatches.append({displayIdx, pos, static_cast<int>(searchStr.length())});
                    pos += static_cast<int>(searchStr.length());
                }
            }
        } else if (regex) {
            QRegularExpression re(pattern);
            if (!re.isValid()) {
                emit searchMatchesChanged(0, -1);
                return 0;
            }
            for (int displayIdx = 0; displayIdx < filteredCount; ++displayIdx) {
                int modelLine = directionFilter->modelIndex(displayIdx);
                if (modelLine < 0 || modelLine >= cachedLines.size()) continue;
                const QString& text = cachedLines[modelLine].text;
                QRegularExpressionMatchIterator it = re.globalMatch(text);
                while (it.hasNext()) {
                    auto match = it.next();
                    m_searchMatches.append({displayIdx, static_cast<int>(match.capturedStart()),
                                            static_cast<int>(match.capturedLength())});
                }
            }
        } else {
            for (int displayIdx = 0; displayIdx < filteredCount; ++displayIdx) {
                int modelLine = directionFilter->modelIndex(displayIdx);
                if (modelLine < 0 || modelLine >= cachedLines.size()) continue;
                const QString& text = cachedLines[modelLine].text;
                int pos = 0;
                while ((pos = text.indexOf(pattern, pos)) >= 0) {
                    m_searchMatches.append({displayIdx, pos, static_cast<int>(pattern.length())});
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
                return 0;
            }
            for (int i = 0; i < cachedLines.size(); ++i) {
                QString hexText = HexConverter::toHexString(lineAtFn(i));
                int pos = 0;
                while ((pos = hexText.indexOf(searchStr, pos, Qt::CaseInsensitive)) >= 0) {
                    m_searchMatches.append({i, pos, static_cast<int>(searchStr.length())});
                    pos += static_cast<int>(searchStr.length());
                }
            }
        } else if (regex) {
            QRegularExpression re(pattern);
            if (!re.isValid()) {
                emit searchMatchesChanged(0, -1);
                return 0;
            }
            for (int i = 0; i < cachedLines.size(); ++i) {
                QRegularExpressionMatchIterator it = re.globalMatch(cachedLines[i].text);
                while (it.hasNext()) {
                    auto match = it.next();
                    m_searchMatches.append({i, static_cast<int>(match.capturedStart()),
                                            static_cast<int>(match.capturedLength())});
                }
            }
        } else {
            for (int i = 0; i < cachedLines.size(); ++i) {
                const QString& text = cachedLines[i].text;
                int pos = 0;
                while ((pos = text.indexOf(pattern, pos)) >= 0) {
                    m_searchMatches.append({i, pos, static_cast<int>(pattern.length())});
                    pos += pattern.length();
                }
            }
        }
    }

    if (!m_searchMatches.isEmpty()) {
        m_currentMatchIndex = 0;
    }

    emit searchMatchesChanged(m_searchMatches.size(), m_currentMatchIndex);
    return m_searchMatches.size();
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
