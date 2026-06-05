#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 对称矩阵特征值求解工具类
 *
 * 提供对称矩阵特征值分解功能，适用于实对称矩阵，
 * 返回全部特征值和对应的特征向量。
 */
class SymmetricEigen11 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SymmetricEigen11(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加对称矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 执行特征值分解 */
    void solve();

    /** @brief 获取特征值列表(升序排列) */
    QVector<double> eigenvalues() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回维度 */
    void solveCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<QPair<int, int>, double>> m_entries;
    QVector<double> m_eigenvalues;
};
