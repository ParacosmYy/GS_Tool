/**
 * @file SymmetricEigen.h
 * @brief 对称矩阵特征值分解 — Jacobi旋转法
 *
 * 对实对称矩阵进行特征值分解 A = Q * D * Q^T,
 * 其中D为特征值对角阵, Q为正交特征向量矩阵。
 * 使用经典Jacobi旋转迭代法, 适用于中小规模矩阵。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SymmetricEigen
 * @brief 对称矩阵特征值分解 — Jacobi旋转法
 *
 * 迭代消去最大非对角元素直到收敛, 保证正交特征向量。
 * 适用于PCA、振动分析、量子力学等需要特征分解的场景。
 */
class SymmetricEigen : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0; ///< 总分解次数
        quint64 totalRotations = 0;      ///< 总旋转次数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 分解结果 */
    struct EigenResult {
        QVector<double> eigenvalues;       ///< 特征值(降序排列)
        QVector<QVector<double>> eigenvectors; ///< 特征向量矩阵 [n x n]
        int iterations = 0;                ///< 迭代次数
        double finalOffDiag = 0.0;         ///< 最终非对角元素范数
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SymmetricEigen(QObject* parent = nullptr);

    /**
     * @brief 设置最大迭代次数
     * @param maxIterations 最大迭代次数(默认100*n)
     */
    void setMaxIterations(int maxIterations);

    /**
     * @brief 设置收敛阈值
     * @param tolerance 收敛容差(默认1e-10)
     */
    void setTolerance(double tolerance);

    /**
     * @brief 对称矩阵特征值分解
     * @param matrix 对称矩阵(行优先, n x n)
     * @param n 矩阵维度
     * @return 分解结果(特征值+特征向量)
     */
    EigenResult decompose(const QVector<double>& matrix, int n);

    /**
     * @brief 仅计算特征值(不计算特征向量)
     * @param matrix 对称矩阵
     * @param n 矩阵维度
     * @return 特征值(降序)
     */
    QVector<double> eigenvaluesOnly(const QVector<double>& matrix,
                                     int n);

    /**
     * @brief 重构矩阵: Q * D * Q^T
     * @param result 分解结果
     * @param n 维度
     * @return 重构后的矩阵
     */
    QVector<double> reconstruct(const EigenResult& result, int n) const;

    /**
     * @brief 计算条件数(最大特征值/最小特征值)
     * @param eigenvalues 特征值列表
     * @return 条件数
     */
    double conditionNumber(const QVector<double>& eigenvalues) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵维度 @param iterations 迭代次数 */
    void decompositionCompleted(int n, int iterations);

private:
    /** @brief 找到最大非对角元素的位置 */
    QPair<int, int> findMaxOffDiag(const QVector<double>& A, int n) const;

    /** @brief 计算非对角元素范数 */
    double offDiagNorm(const QVector<double>& A, int n) const;

    int m_maxIterations = 0;     ///< 最大迭代次数(0=自动)
    double m_tolerance = 1e-10;  ///< 收敛容差

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
