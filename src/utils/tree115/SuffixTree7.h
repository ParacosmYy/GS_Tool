#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SuffixTree7 - 后缀树第7代实现
 *
 * 提供Ukkonen线性时间后缀树构建，支持子串搜索、
 * 最长重复子串查找、回文检测及字符串匹配。
 */
class SuffixTree7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearchOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit SuffixTree7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用Ukkonen算法构建后缀树
     * @param text 输入文本
     * @return 是否构建成功
     */
    bool build(const QString& text);

    /**
     * @brief 查找模式串在文本中的所有出现位置
     * @param pattern 待搜索的模式串
     * @return 所有匹配起始位置
     */
    QVector<int> search(const QString& pattern);

    /**
     * @brief 查找最长重复子串
     * @return 最长重复子串及其出现次数
     */
    QPair<QString, int> longestRepeatedSubstring();

    /**
     * @brief 统计不同子串的总数
     * @return 不同子串数量
     */
    int countDistinctSubstrings();

signals:
    void searchCompleted(int matchCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
