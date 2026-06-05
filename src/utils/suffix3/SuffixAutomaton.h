/**
 * @file SuffixAutomaton.h
 * @brief 后缀自动机(SAM) — 在线字符串处理
 *
 * 实现后缀自动机(Suffix Automaton)，支持:
 *   - 在线字符添加(O(1)均摊)
 *   - 子串存在性查询
 *   - 最长子串查询
 *   - 不同子串计数
 *   - 最长公共子串(LCS)
 *   - 子串出现次数统计
 *
 * 协作: BytePatternAnalyzer(模式匹配) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QPair>
#include <QByteArray>

/**
 * @class SuffixAutomaton
 * @brief 后缀自动机(SAM) — 在线构建与查询
 *
 * 后缀自动机是一个字符串的所有后缀的最小DFA。
 * 支持在线添加字符，可高效完成多种字符串查询。
 */
class SuffixAutomaton : public QObject
{
    Q_OBJECT

public:
    /** @brief SAM状态节点 */
    struct State {
        int length = 0;              ///< 最长子串长度
        int link = -1;               ///< 后缀链接(fail指针)
        QMap<quint8, int> next;      ///< 转移函数
        int occurrences = 0;         ///< 出现次数(需调用countOccurrences后)
        bool isClone = false;        ///< 是否为克隆节点
        int firstPos = -1;           ///< 首次出现位置
    };

    /** @brief 查询结果 */
    struct QueryResult {
        bool found = false;          ///< 是否找到
        int length = 0;              ///< 匹配长度
        int occurrences = 0;         ///< 出现次数
        int firstPosition = -1;      ///< 首次出现位置
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalCharsAdded = 0;        ///< 累计添加字符数
        quint64 totalQueries = 0;           ///< 累计查询次数
        quint64 totalStateCount = 0;        ///< 累计状态节点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均查询耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SuffixAutomaton(QObject* parent = nullptr);

    /**
     * @brief 在线添加一个字符
     * @param c 字节值
     */
    void extend(quint8 c);

    /**
     * @brief 批量构建(追加字符串)
     * @param data 字节数据
     */
    void build(const QByteArray& data);

    /**
     * @brief 查询子串是否存在
     * @param pattern 查询模式串
     * @return 查询结果
     */
    QueryResult contains(const QByteArray& pattern) const;

    /**
     * @brief 统计子串出现次数(需先调用countOccurrences)
     * @param pattern 模式串
     * @return 出现次数
     */
    int occurrenceCount(const QByteArray& pattern) const;

    /**
     * @brief 计算所有状态的出现次数(拓扑排序)
     */
    void computeOccurrenceCounts();

    /**
     * @brief 计算不同子串总数
     * @return 不同子串数量
     */
    qint64 distinctSubstrings() const;

    /**
     * @brief 查找两个串的最长公共子串(LCS)
     * @param a 第一个串
     * @param b 第二个串
     * @return (LCS长度, LCS内容)
     */
    QPair<int, QByteArray> longestCommonSubstring(
        const QByteArray& a, const QByteArray& b);

    /**
     * @brief 查找最长重复子串
     * @return (长度, 子串内容)
     */
    QPair<int, QByteArray> longestRepeatedSubstring() const;

    /**
     * @brief 获取所有状态数量
     */
    int stateCount() const;

    /**
     * @brief 获取当前构建的字符串长度
     */
    int textLength() const;

    /**
     * @brief 重置自动机
     */
    void clear();

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 字符添加完成 @param length 当前字符串长度 */
    void characterAdded(int length);

    /** @brief 查询完成 @param found 是否找到 @param occurrences 出现次数 */
    void queryCompleted(bool found, int occurrences);

private:
    /** @brief 克隆状态节点 */
    int cloneState(int source);

    QVector<State> m_states;      ///< 状态集合
    int m_last = 0;               ///< 最后一个状态
    int m_textLength = 0;         ///< 当前文本长度
    bool m_occComputed = false;   ///< 出现次数是否已计算

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
