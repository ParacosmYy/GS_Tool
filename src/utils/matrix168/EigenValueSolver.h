/**
 * @file EigenValueSolver.h
 * @brief 特征值求解(QR算法+隐式双移) — QR Algorithm Eigenvalue Solver with Implicit Double Shifts
 *
 * 功能: 实现QR迭代法求解实矩阵全部特征值，使用隐式双移(Francis QR步)加速收敛。
 *       先用Householder约化至上Hessenberg型，再执行带位移的QR迭代。
 *       支持2x2实特征块检测(复特征值对)。
 *
 * 协作: LeastSquares(最小二乘) / SvdEngine(SVD) / MatrixDecomposer(LU/QR)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief QR算法特征值求解器
 */
class EigenValueSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 特征值(实部+虚部) */
    struct EigenValue {
        double real;        ///< 实部
        double imag;        ///< 虚部(0表示实特征值)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalQrIterations = 0;      ///< 累计QR迭代次数
        int lastMatrixSize = 0;             ///< 最近矩阵阶数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit EigenValueSolver(QObject* parent = nullptr);
    ~EigenValueSolver() override;

    /** @brief 设置最大QR迭代次数 */
    void setMaxIterations(int maxIter);
    /** @brief 设置收敛阈值 */
    void setConvergenceThreshold(double threshold);

    /**
     * @brief 求解全部特征值
     * @param matrix 方阵(行优先,n x n)
     * @return 特征值列表
     */
    QVector<EigenValue> solve(const QVector<QVector<double>>& matrix);

    /**
     * @brief 求解全部特征值和特征向量
     * @param matrix 方阵
     * @param eigenVectors 输出特征向量(列向量)
     * @return 特征值列表
     */
    QVector<EigenValue> solveWithVectors(const QVector<QVector<double>>& matrix,
                                         QVector<QVector<double>>& eigenVectors);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 矩阵阶数 @param count 特征值数 */
    void solveCompleted(int n, int count);

private:
    /** @brief Householder约化至上Hessenberg */
    void reduceToHessenberg(QVector<QVector<double>>& H,
                            QVector<QVector<double>>& Q) const;

    /** @brief Francis隐式双移QR步 */
    void francisQRStep(QVector<QVector<double>>& H, int lo, int hi,
                       QVector<QVector<double>>& Q) const;

    /** @brief 提取2x2块的特征值 */
    void eigenvalues2x2(double a, double b, double c, double d,
                        EigenValue& e1, EigenValue& e2) const;

    /** @brief Householder向量 */
    static QVector<double> householderVector(const QVector<double>& x, double& beta);

    int m_maxIterations = 1000;
    double m_threshold = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;
};
