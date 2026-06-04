/**
 * @file TerminalSearchBuild.cpp
 * @brief 终端搜索模式构建方法实现 - 纯文本/正则/HEX搜索匹配构建
 *
 * 从TerminalSearchBuilders.cpp拆分而来，包含三种搜索模式的匹配构建方法:
 *   - buildPlainSearch: 纯文本搜索，支持大小写敏感和全词匹配
 *   - buildRegexSearch: 正则表达式搜索，使用QRegularExpression全局匹配
 *   - buildHexSearch: HEX搜索，将输入转换为规范化HEX字符串后匹配
 *
 * 导航/getter/统计方法保留在TerminalSearchBuilders.cpp中。
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
