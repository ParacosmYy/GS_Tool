#pragma once
#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief 后缀树构建与查询
 *
 * 基于Ukkonen算法在线构建后缀树，支持模式匹配、
 * 最长重复子串查找等字符串分析操作。
 */
class SuffixTree5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalNodes = 0;          ///< 树节点总数
        int totalSearches = 0;       ///< 已执行查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SuffixTree5(QObject* parent = nullptr);

    /** @brief 从字符串构建后缀树 */
    void build(const QString& text);
    /** @brief 搜索模式串，返回所有出现位置 */
    QVector<int> search(const QString& pattern);
    /** @brief 查找最长重复子串 */
    QString longestRepeat();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 构建完成，返回节点数 */
    void buildCompleted(int nodeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_text;
};
