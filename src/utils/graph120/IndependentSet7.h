#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 独立集算法实现 (版本7)
 *
 * 求解图的最大独立集问题，支持贪心近似和精确回溯搜索。
 */
class IndependentSet7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalSearches = 0;          ///< 总搜索次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int maxSizeFound = 0;           ///< 找到的最大独立集大小
    };

    explicit IndependentSet7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 求解最大独立集（贪心近似）
     * @param adjacencyMatrix 邻接矩阵
     * @return 独立集中顶点索引列表
     */
    QVector<int> greedySolve(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 求解最大独立集（精确回溯）
     * @param adjacencyMatrix 邻接矩阵
     * @param timeLimitMs 时间限制(ms)，0表示无限制
     * @return 最大独立集顶点索引列表
     */
    QVector<int> exactSolve(const QVector<QVector<int>>& adjacencyMatrix, int timeLimitMs = 0);

    /**
     * @brief 验证给定点集是否为独立集
     * @param adjacencyMatrix 邻接矩阵
     * @param vertices 候选顶点索引
     * @return 是否为合法独立集
     */
    bool validate(const QVector<QVector<int>>& adjacencyMatrix, const QVector<int>& vertices) const;

    /**
     * @brief 获取补图中的团（等价于原图独立集）
     * @return 补图团顶点列表
     */
    QVector<int> complementClique() const { return m_lastResult; }

signals:
    /// 搜索完成信号
    void searchCompleted(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_lastResult;
};
