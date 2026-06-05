#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 对称矩阵特征值求解器
 *
 * 对实对称矩阵计算全部特征值和特征向量，
 * 基于Jacobi迭代或三对角化+QR算法实现。
 */
class SymmetricEigen12 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalSolves = 0;         ///< 已完成求解次数
        int matrixDimension = 0;     ///< 矩阵维度
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SymmetricEigen12(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int n);
    /** @brief 添加矩阵非零元素(对称位置自动填充) */
    void addEntry(int row, int col, double value);
    /** @brief 执行特征值分解 */
    void solve();
    /** @brief 获取所有特征值(升序排列) */
    QVector<double> eigenvalues() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回特征值个数 */
    void solveCompleted(int eigenCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<int, int>> m_entries;
};
