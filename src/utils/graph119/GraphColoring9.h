#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图着色算法实现 (版本9)
 *
 * 提供图顶点着色功能，支持贪心着色和回溯搜索最优着色方案。
 */
class GraphColoring9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalColoringRuns = 0;      ///< 总着色运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int chromaticNumber = 0;        ///< 最小色数
    };

    explicit GraphColoring9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 对图执行贪心着色
     * @param adjacencyMatrix 邻接矩阵表示
     * @return 每个顶点的颜色编号
     */
    QVector<int> greedyColoring(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 求解最优着色（回溯法）
     * @param adjacencyMatrix 邻接矩阵表示
     * @param maxColors 最大允许颜色数
     * @return 最优着色方案，空向量表示无解
     */
    QVector<int> optimalColoring(const QVector<QVector<int>>& adjacencyMatrix, int maxColors);

    /**
     * @brief 验证着色方案是否合法
     * @param adjacencyMatrix 邻接矩阵
     * @param colors 着色方案
     * @return 是否合法
     */
    bool validateColoring(const QVector<QVector<int>>& adjacencyMatrix, const QVector<int>& colors) const;

    /**
     * @brief 获取使用的颜色数量
     * @return 当前着色使用的不同颜色数
     */
    int colorCount() const { return m_colorCount; }

signals:
    /// 着色完成信号
    void coloringCompleted(int colorsUsed);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_colorCount = 0;
};
