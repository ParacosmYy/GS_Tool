/**
 * @file SparseLU.h
 * @brief 稀疏LU分解(AMD排序+符号/数值两阶段) — Sparse LU Factorization with Fill-Reducing AMD Ordering and Symbolic/Numeric Phases
 *
 * 功能: 实现稀疏矩阵LU分解，支持AMD(Approximate Minimum Degree)填充缩减排序、
 *       符号分解(预测填充模式)和数值分解两阶段求解。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / GaussElimination5(高斯消元) / LeastSquares5(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏LU分解器
 */
class SparseLU : public QObject {
    Q_OBJECT

public:
    /** @brief 稀疏矩阵三元组 */
    struct Triplet {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFactorizations = 0;   ///< 累计分解次数
        int matrixSize = 0;                ///< 最近矩阵维度
        int nnzOriginal = 0;               ///< 原始非零元数
        int nnzFactor = 0;                 ///< 分解后非零元数
        double fillRatio = 0.0;            ///< 填充率
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit SparseLU(QObject *parent = nullptr);
    ~SparseLU() override;

    /** @brief 从三元组构建稀疏矩阵 */
    void setMatrix(int n, const QVector<Triplet>& triplets);

    /** @brief 执行符号+数值分解 */
    bool factorize();

    /** @brief 求解 Ax=b */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 仅符号分解(预测填充模式) */
    bool symbolicFactorize();

    /** @brief 仅数值分解(基于已有符号结构) */
    bool numericFactorize();

    /** @brief AMD排序: 返回列置换 */
    QVector<int> amdOrdering() const;

    /** @brief 计算填充率 */
    double fillRatio() const;

    int matrixSize() const { return m_n; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int nnz, double fillRatio);
    void solveCompleted(int n);

private:
    /** @brief 三元组转CSC格式 */
    void buildCSC();

    /** @brief 符号分解: 确定L/U非零模式 */
    void symbolicPhase();

    /** @brief 数值分解: 左看算法 */
    void numericPhase();

    /** @brief 三角求解 L*y=b */
    QVector<double> forwardSolve(const QVector<double>& b) const;

    /** @brief 三角求解 U*x=y */
    QVector<double> backwardSolve(const QVector<double>& y) const;

    /** @brief 应用列置换 */
    QVector<double> permute(const QVector<double>& v) const;

    int m_n = 0;                            ///< 矩阵维度
    QVector<Triplet> m_triplets;            ///< 原始三元组

    /** CSC格式 for A */
    QVector<int> m_colPtr;                  ///< 列指针
    QVector<int> m_rowIdx;                  ///< 行索引
    QVector<double> m_values;               ///< 值

    /** L/U factor CSC */
    QVector<int> m_lColPtr, m_lRowIdx;
    QVector<double> m_lValues;
    QVector<int> m_uColPtr, m_uRowIdx;
    QVector<double> m_uValues;

    QVector<int> m_perm;                    ///< 列置换向量
    QVector<int> m_invPerm;                 ///< 逆置换

    Stats m_stats;
    double m_timeSum = 0.0;
};
