/**
 * @file TerminalSearchManager.h
 * @brief 终端搜索管理器 - 管理终端文本搜索、高亮匹配和导航
 *
 * 从 TerminalWidget 中拆分出来，负责:
 *   - 文本/正则/HEX模式搜索
 *   - 搜索匹配结果的存储和管理
 *   - F3/Shift+F3 匹配导航
 *   - 方向过滤模式下的搜索支持
 *
 * 协作关系:
 *   - TerminalWidget: 持有本类实例，在数据更新和用户操作时委托调用
 *   - DirectionFilter: 过滤模式下只在过滤后的行中搜索
 *   - CachedLine: 搜索文本从缓存行数据中匹配
 *   - HexConverter: HEX搜索模式的编解码
 */

#ifndef TERMINALSEARCHMANAGER_H
#define TERMINALSEARCHMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QColor>
#include <QRegularExpression>
#include <functional>
#include "terminal/TerminalTypes.h"

/**
 * @brief 单个搜索匹配的位置信息
 */
struct SearchMatch {
    int line;       ///< 匹配所在行号(显示行号，非模型行号)
    int startCol;   ///< 匹配起始列号(字符偏移)
    int length;     ///< 匹配文本长度(字符数)
};

class DirectionFilter;

/**
 * @brief 终端搜索管理器 - 管理搜索匹配和高亮导航
 *
 * 支持三种搜索模式:
 *   - 纯文本搜索: 直接字符串匹配
 *   - 正则表达式搜索: QRegularExpression匹配
 *   - HEX搜索: 十六进制字符串匹配(忽略大小写)
 *
 * 搜索结果存储在 m_searchMatches 向量中，支持 F3/Shift+F3 循环导航。
 * 在方向过滤模式下，搜索范围限定为过滤后的可见行。
 *
 * 设计模式: 组合模式 — TerminalWidget 通过组合持有本类，
 * 所有搜索相关的状态和行为集中在此管理。
 */
class TerminalSearchManager : public QObject {
    Q_OBJECT

public:
    explicit TerminalSearchManager(QObject* parent = nullptr);

    /**
     * @brief 执行搜索并更新匹配列表
     * @param pattern 搜索模式串(文本/正则/HEX字符串)
     * @param regex 是否使用正则表达式模式
     * @param hex 是否使用HEX模式
     * @param cachedLines 缓存行数据(用于文本匹配)
     * @param directionFilter 方向过滤器(过滤模式下限定搜索范围)
     * @param modelLineCount 模型总行数(HEX模式需要重新从模型读取原始数据)
     * @param lineAtFn 回调函数: 通过行号获取原始TerminalLine数据(HEX模式使用)
     * @return 匹配总数，-1表示搜索参数无效
     */
    int setSearchHighlight(const QString& pattern, bool regex, bool hex,
                           const QVector<CachedLine>& cachedLines,
                           const DirectionFilter* directionFilter,
                           int modelLineCount,
                           const std::function<QByteArray(int)>& lineAtFn);

    /** @brief 清除搜索高亮，重置所有搜索状态 */
    void clearSearchHighlight();

    /** @brief 获取当前匹配总数 */
    int searchMatchCount() const;

    /** @brief 获取当前高亮的匹配索引(从0开始) */
    int currentMatchIndex() const;

    /**
     * @brief 导航到下一个匹配项
     * @return 导航后需要滚动到的行号，-1表示无匹配
     */
    int gotoNextMatch();

    /**
     * @brief 导航到上一个匹配项
     * @return 导航后需要滚动到的行号，-1表示无匹配
     */
    int gotoPrevMatch();

    /** @brief 获取当前搜索模式串 */
    QString searchPattern() const;

    /** @brief 是否正在使用正则表达式模式 */
    bool searchRegex() const;

    /** @brief 是否正在使用HEX模式 */
    bool searchHex() const;

    /** @brief 获取所有搜索匹配列表(用于paintLine绘制高亮) */
    const QVector<SearchMatch>& searchMatches() const;

    /** @brief 获取搜索高亮背景色(非当前匹配) */
    QColor searchHighlightColor() const;

    /** @brief 获取当前匹配高亮背景色 */
    QColor currentMatchColor() const;

    /** @brief 设置搜索高亮配色 */
    void setSearchColors(const QColor& highlight, const QColor& current);

signals:
    /**
     * @brief 搜索匹配结果变化时发射
     * @param total 匹配总数
     * @param current 当前高亮匹配的索引，-1表示无高亮
     */
    void searchMatchesChanged(int total, int current);

private:
    QVector<SearchMatch> m_searchMatches;   ///< 所有搜索匹配的位置列表
    int m_currentMatchIndex = -1;           ///< 当前高亮的匹配索引
    QString m_searchPattern;                ///< 当前搜索模式串
    bool m_searchRegex = false;             ///< 是否使用正则表达式
    bool m_searchHex = false;               ///< 是否使用HEX搜索

    QColor m_searchHighlightColor;          ///< 搜索高亮背景色(半透明黄)
    QColor m_currentMatchColor;             ///< 当前匹配高亮色(高透明度黄)

    // ---- 内部搜索构建方法 ----

    /**
     * @brief 纯文本模式搜索，遍历行并收集匹配
     * @param pattern 搜索文本
     * @param lineProvider 提供 (displayIdx, text) 对的回调，返回false结束迭代
     */
    void buildPlainSearch(const QString& pattern,
                          const std::function<bool(int*, QString*)>& lineProvider);

    /**
     * @brief 正则表达式模式搜索，遍历行并收集匹配
     * @param pattern 正则表达式字符串
     * @param lineProvider 提供 (displayIdx, text) 对的回调，返回false结束迭代
     * @return true 表示正则有效并完成搜索，false 表示正则无效
     */
    bool buildRegexSearch(const QString& pattern,
                          const std::function<bool(int*, QString*)>& lineProvider);

    /**
     * @brief HEX模式搜索，将原始字节转为HEX字符串后匹配
     * @param pattern HEX搜索串
     * @param lineProvider 提供 (displayIdx, text) 对的回调，返回false结束迭代
     * @return true 表示HEX转换有效并完成搜索，false 表示HEX串无效
     */
    bool buildHexSearch(const QString& pattern,
                        const std::function<bool(int*, QString*)>& lineProvider);
};

#endif // TERMINALSEARCHMANAGER_H
