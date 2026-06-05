#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏矩阵LU分解实现 (版本5)
 *
 * 提供稀疏矩阵的LU分解功能，保持稀疏性结构，适用于大规模稀疏方程组求解。
 */
class SparseLU5 : public QObject {
    Q_OBJECT
public:
    /// 稀疏矩阵三元组表示
    struct Triplet { int row; int col; double value; };

    /// 统计信息结构
    struct Stats {
        int totalFactorizations = 0;   ///< 总分解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int nnzL = 0;                  ///< L矩阵非零元数
        int nnzU = 0;                  ///< U矩阵非零元数
    };

    explicit SparseLU5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行稀疏LU分解 (带部分主元选取)
     * @param triplets 稀疏矩阵的三元组表示
     * @param n 矩阵维度
     * @return 是否分解成功
     */
    bool factorize(const QVector<Triplet>& triplets, int n);

    /**
     * @brief 求解线性方程组 Ax=b
     * @param rhs 右端向量
     * @return 解向量
     */
    QVector<double> solve(const QVector<double>& rhs) const;

    /**
     * @brief 计算行列式
     * @return 行列式值
     */
    double determinant() const { return m_determinant; }

    /**
     * @brief 获取分解的填充比
     * @return 填充比(nnz(L+U)/nnz(A))
     */
    double fillRatio() const;

signals:
    /// 分解完成信号
    void factorizationCompleted(int matrixSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_n = 0;
    double m_determinant = 0.0;
    QVector<Triplet> m_lEntries;
    QVector<Triplet> m_uEntries;
    QVector<int> m_permutation;
};
