#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SparseLU3 - 稀疏LU分解求解器
 *
 * 针对稀疏矩阵优化的LU分解，使用填充减少排序
 * 和部分主元选取，高效求解大规模稀疏线性系统。
 */
class SparseLU3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFactorizations = 0;
        int totalSolves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU3(QObject* parent = nullptr);

    /** @brief 从COO格式构建稀疏矩阵并分解 */
    bool factorize(const QVector<int>& rows, const QVector<int>& cols,
                   const QVector<double>& values, int n);

    /** @brief 分解后求解Ax=b */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 设置填充减少排序策略 */
    void setOrdering(const QString& ordering);

    /** @brief 获取分解后的非零元数 */
    int nnzAfterFactorization() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int nnz, double fillRatio);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_n = 0;
    int m_nnz = 0;
    QString m_ordering = "amd";
};
