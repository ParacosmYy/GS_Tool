/**
 * @file SymmetricEigen3.h
 * @brief 对称特征值分解引擎 — 三对角化/隐式QR/Wilkinson位移/特征值排序
 *
 * 功能: 对实对称矩阵执行完整的特征值分解，包含Householder三对角化、
 *       带Wilkinson位移的隐式QR迭代、特征值排序和特征向量累积。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DataNormalizer(数据归一化)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值分解引擎 — 三对角化与隐式QR迭代
 */
class SymmetricEigen3 : public QObject {
    Q_OBJECT

public:
    /** @brief 特征分解结果 */
    struct EigenResult {
        QVector<double> eigenvalues;         ///< 特征值(降序排列)
        QVector<QVector<double>> eigenvectors; ///< 特征向量(列向量)
        int iterations = 0;                  ///< QR迭代次数
        bool converged = false;             ///< 是否收敛
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDecompositions = 0;        ///< 累计分解次数
        quint64 totalMatricesProcessed = 0;     ///< 累计处理矩阵数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
        quint64 totalIterations = 0;            ///< 累计QR迭代次数
        quint64 totalDivergences = 0;           ///< 累计发散次数
    };

    explicit SymmetricEigen3(QObject* parent = nullptr);

    /** @brief 设置最大QR迭代次数 @param maxIter 最大迭代次数 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛阈值 @param eps 收敛阈值 */
    void setEpsilon(double eps);

    /** @brief 对称矩阵特征分解 @param matrix 行优先对称矩阵 @param n 矩阵阶数 @return 特征分解结果 */
    EigenResult decompose(const QVector<double>& matrix, int n);

    /** @brief 仅计算前k个特征值 @param matrix 对称矩阵 @param n 阶数 @param k 目标数量 @return 特征值 */
    QVector<double> topKEigenvalues(const QVector<double>& matrix,
                                    int n, int k);

    /** @brief 计算矩阵条件数 @param matrix 矩阵 @param n 阶数 @return 条件数 */
    double conditionNumber(const QVector<double>& matrix, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵阶数 @param iterations 迭代次数 @param converged 是否收敛 */
    void decompositionComplete(int n, int iterations, bool converged);

private:
    void tridiagonalize(QVector<double>& diag, QVector<double>& offDiag,
                        QVector<QVector<double>>& q, int n) const;
    int implicitQR(QVector<double>& diag, QVector<double>& offDiag,
                   QVector<QVector<double>>& q, int n);
    void wilkinsonShift(double d, double e, double& sigma) const;
    void givensRotation(double a, double b, double& c, double& s) const;

    int m_maxIterations;            ///< 最大QR迭代次数
    double m_epsilon;               ///< 收敛阈值

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
