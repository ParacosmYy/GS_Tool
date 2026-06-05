/**
 * @file SchurDecomp2.h
 * @brief Schur分解(增强版) — QR迭代 + 拟上三角形式 + 特征值提取
 *
 * 功能: 将实方阵A分解为A = Q T Q^T，其中Q正交、T拟上三角。
 *       增强版支持: 自动Hessenberg预处理、Francis双移QR步、
 *       2x2块特征值提取、收敛检测。统计分解次数/迭代步数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SchurDecomp2
 * @brief 增强版实Schur分解器，支持双移QR迭代
 */
class SchurDecomp2 : public QObject {
    Q_OBJECT
public:
    /** 复数特征值 */
    struct ComplexEigen {
        double real; ///< 实部
        double imag; ///< 虚部
    };

    /** 分解统计 */
    struct Stats {
        quint64 totalDecompositions = 0;   ///< 总分解次数
        quint64 totalQRIterations = 0;     ///< 累计QR迭代次数
        quint64 totalEigenvaluesExtracted = 0; ///< 累计提取特征值数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 分解结果 */
    struct SchurResult {
        QVector<QVector<double>> Q; ///< 正交矩阵
        QVector<QVector<double>> T; ///< 拟上三角矩阵
        QVector<ComplexEigen> eigenvalues; ///< 提取的特征值
        int iterations;              ///< 实际迭代次数
        bool converged;              ///< 是否收敛
    };

    /** 构造函数 */
    explicit SchurDecomp2(QObject* parent = nullptr);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛阈值 @param eps 收敛判据 */
    void setEpsilon(double eps);

    /**
     * @brief 执行Schur分解
     * @param A 输入方阵(将被拷贝)
     * @return 分解结果(Q, T, 特征值)
     */
    SchurResult decompose(const QVector<QVector<double>>& A);

    /**
     * @brief 仅提取特征值(不保留Q)
     * @param A 输入方阵
     * @return 特征值列表
     */
    QVector<ComplexEigen> eigenvaluesOnly(const QVector<QVector<double>>& A);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param size 矩阵阶数 @param iterations 迭代次数 @param ok 是否收敛 */
    void decompositionCompleted(int size, int iterations, bool ok);
    /** @brief QR迭代步骤 @param step 当前步数 @param residual 当前残差 */
    void iterationStep(int step, double residual);

private:
    /** Hessenberg归约(上Hessenberg形) */
    void hessenbergReduce(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& Q) const;
    /** Francis双移QR步 */
    void francisDoubleShift(QVector<QVector<double>>& H,
                            QVector<QVector<double>>& Q,
                            int p, int q) const;
    /** 从拟上三角T提取特征值 */
    QVector<ComplexEigen> extractEigenvalues(
        const QVector<QVector<double>>& T) const;
    /** 检查子矩阵是否可分裂 */
    bool isDeflatable(const QVector<QVector<double>>& H,
                      int p, int q) const;
    /** Givens旋转 */
    void applyGivens(QVector<QVector<double>>& M,
                     int i, int j, double c, double s,
                     bool left) const;

    int    m_maxIter;       ///< 最大迭代次数
    double m_epsilon;       ///< 收敛阈值

    Stats  m_stats;         ///< 统计信息
    double m_timeSum = 0.0; ///< 累计耗时
};
