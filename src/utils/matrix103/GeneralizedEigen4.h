#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值问题求解器
 *
 * 求解广义特征值问题 Ax = λBx，其中A和B为对称矩阵，
 * 用于结构力学和振动分析等工程计算。
 */
class GeneralizedEigen4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalSolves = 0;         ///< 已完成求解次数
        int matrixDimension = 0;     ///< 矩阵维度
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit GeneralizedEigen4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int n);
    /** @brief 添加矩阵A的元素 */
    void addA(int row, int col, double value);
    /** @brief 添加矩阵B的元素 */
    void addB(int row, int col, double value);
    /** @brief 执行广义特征值求解 */
    void solve();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回特征值个数 */
    void solveCompleted(int eigenCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<int, int>> m_entriesA;
    QVector<QPair<int, int>> m_entriesB;
};
