#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur分解工具类
 *
 * 提供矩阵的Schur分解功能，将矩阵分解为
 * Q * T * Q^T形式，其中T为上三角矩阵。
 */
class SchurDecomp4 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SchurDecomp4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 执行Schur分解 */
    void solve();

    /** @brief 获取Schur上三角矩阵 */
    QVector<QVector<double>> schurForm() const;

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
};
