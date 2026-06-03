/**
 * @file TerminalSearchManager.h
 * @brief 终端搜索管理器 - 管理终端文本搜索、高亮匹配、导航和搜索历史
 *
 * 支持文本/正则/HEX搜索，大小写敏感/全词匹配切换，搜索历史管理。
 * 协作: TerminalWidget(持有), DirectionFilter(过滤), CachedLine(数据)
 */

#ifndef TERMINALSEARCHMANAGER_H
#define TERMINALSEARCHMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QColor>
#include <QRegularExpression>
#include <functional>
#include "terminal/types/TerminalTypes.h"

/** @brief 搜索历史最大条目数 */
constexpr int kMaxSearchHistory = 50;

/** @brief 单个搜索匹配的位置信息 */
struct SearchMatch {
    int line;       ///< 匹配所在行号(显示行号，非模型行号)
    int startCol;   ///< 匹配起始列号(字符偏移)
    int length;     ///< 匹配文本长度(字符数)
};

class DirectionFilter;

/**
 * @brief 终端搜索管理器 - 管理搜索匹配、高亮导航和搜索历史
 *
 * 搜索模式: 纯文本(支持大小写/全词)、正则表达式、HEX
 * 搜索历史: 最近 kMaxSearchHistory 条不重复关键字
 * 设计模式: 组合模式 — TerminalWidget 通过组合持有本类
 */
class TerminalSearchManager : public QObject {
    Q_OBJECT

public:
    explicit TerminalSearchManager(QObject* parent = nullptr);

    /** @brief 执行搜索并更新匹配列表 @return 匹配总数，0表示无效或无匹配 */
    int setSearchHighlight(const QString& pattern, bool regex, bool hex,
                           bool caseSensitive, bool wholeWord,
                           const QVector<CachedLine>& cachedLines,
                           const DirectionFilter* directionFilter,
                           int modelLineCount,
                           const std::function<QByteArray(int)>& lineAtFn);

    void clearSearchHighlight();        ///< 清除搜索高亮，重置所有搜索状态
    int searchMatchCount() const;       ///< 获取当前匹配总数
    int currentMatchIndex() const;      ///< 获取当前高亮的匹配索引(从0开始)
    int gotoNextMatch();                ///< 导航到下一个匹配项 @return 目标行号
    int gotoPrevMatch();                ///< 导航到上一个匹配项 @return 目标行号
    QString searchPattern() const;      ///< 获取当前搜索模式串
    bool searchRegex() const;           ///< 是否正则模式
    bool searchHex() const;             ///< 是否HEX模式
    bool searchCaseSensitive() const;   ///< 是否大小写敏感
    bool searchWholeWord() const;       ///< 是否全词匹配

    /** @brief 获取所有搜索匹配列表(用于paintLine绘制高亮) */
    const QVector<SearchMatch>& searchMatches() const;
    QColor searchHighlightColor() const;    ///< 搜索高亮背景色(非当前匹配)
    QColor currentMatchColor() const;       ///< 当前匹配高亮背景色
    void setSearchColors(const QColor& highlight, const QColor& current); ///< 设置配色

    // ---- 搜索历史接口 ----
    QStringList searchHistory() const;  ///< 获取搜索历史列表(最近的在前)
    void clearSearchHistory();          ///< 清除搜索历史

    // ---- 统计计数接口 ----
    quint64 totalSearches() const;      ///< 累计搜索次数
    quint64 totalMatches() const;       ///< 累计匹配结果总数
    quint64 totalReplacements() const;  ///< 累计替换操作次数
    quint64 searchErrorCount() const;   ///< 搜索错误次数
    void resetStats();                  ///< 重置所有统计计数器

signals:
    /** @brief 搜索匹配结果变化 @param total 匹配总数 @param current 当前高亮索引 */
    void searchMatchesChanged(int total, int current);
    /** @brief 搜索历史变化(新增或清除) @param history 最新搜索历史列表 */
    void searchHistoryChanged(const QStringList& history);

private:
    QVector<SearchMatch> m_searchMatches;   ///< 所有搜索匹配的位置列表
    int m_currentMatchIndex = -1;           ///< 当前高亮的匹配索引
    QString m_searchPattern;                ///< 当前搜索模式串
    bool m_searchRegex = false;             ///< 是否使用正则表达式
    bool m_searchHex = false;               ///< 是否使用HEX搜索
    bool m_searchCaseSensitive = false;     ///< 是否区分大小写
    bool m_searchWholeWord = false;         ///< 是否全词匹配
    QColor m_searchHighlightColor;          ///< 搜索高亮背景色
    QColor m_currentMatchColor;             ///< 当前匹配高亮色
    QStringList m_searchHistory;            ///< 搜索历史列表(最近在前，最多50条)

    /** @brief 纯文本模式搜索(支持大小写/全词选项) */
    void buildPlainSearch(const QString& pattern, bool caseSensitive, bool wholeWord,
                          const std::function<bool(int*, QString*)>& lineProvider);
    /** @brief 正则表达式搜索 @return 正则有效返回true */
    bool buildRegexSearch(const QString& pattern, bool caseSensitive,
                          const std::function<bool(int*, QString*)>& lineProvider);
    /** @brief HEX模式搜索(规范化后匹配) @return HEX有效返回true */
    bool buildHexSearch(const QString& pattern,
                        const std::function<bool(int*, QString*)>& lineProvider);
    /** @brief 添加搜索关键字到历史(去重并移到最前) */
    void addToHistory(const QString& pattern);

    quint64 m_totalSearches = 0;        ///< 累计搜索执行次数
    quint64 m_totalMatches = 0;         ///< 累计匹配结果总数
    quint64 m_totalReplacements = 0;    ///< 累计替换操作次数
    quint64 m_searchErrorCount = 0;     ///< 累计搜索错误次数
};

#endif // TERMINALSEARCHMANAGER_H
