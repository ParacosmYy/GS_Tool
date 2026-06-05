#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 后缀树实现 (版本6)
 *
 * 提供基于Ukkonen算法的后缀树构造，支持快速子串搜索和最长重复子串查找。
 */
class SuffixTree6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalBuilds = 0;            ///< 总构建次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int nodeCount = 0;              ///< 树节点数
    };

    explicit SuffixTree6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 构建后缀树
     * @param text 输入字符串
     * @return 构建是否成功
     */
    bool build(const QString& text);

    /**
     * @brief 搜索子串出现位置
     * @param pattern 搜索模式串
     * @return 所有出现位置的起始索引
     */
    QVector<int> search(const QString& pattern) const;

    /**
     * @brief 查找最长重复子串
     * @return 最长重复子串及其出现次数
     */
    QPair<QString, int> longestRepeatedSubstring() const;

    /**
     * @brief 计算不同子串的总数
     * @return 不同子串数量
     */
    int distinctSubstringCount() const;

signals:
    /// 构建完成信号
    void buildCompleted(int textLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_text;
    int m_nodeCount = 0;
};
