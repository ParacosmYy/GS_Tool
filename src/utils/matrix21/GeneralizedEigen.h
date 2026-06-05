/**
 * @file GeneralizedEigen.h
 * @brief 广义特征值问题 — Ax=λBx / QZ分解 / Hessenberg三角化
 *
 * 功能: 求解广义特征值问题Ax=λBx，实现QZ分解算法，
 *       包含化简为上Hessenberg-三角形式，隐式双位移QZ迭代，
 *       用于振动分析、稳定性分析和结构动力学。
 *
 * 协作: SpectrumAnalyzer(模态分析) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值求解器 — QZ分解/Hessenberg三角化
 */
class GeneralizedEigen : public QObject {
    Q_OBJECT

public:
    /** @brief QZ分解结果: A=Q*S*Z^T, B=Q*T*Z^T */
    struct QZResult {
        QVector<QVector<double>> Q;  ///< 正交矩阵Q
        QVector<QVector<double>> S;  ///< 上准三角矩阵S
        QVector<QVector<double>> T;  ///< 上三角矩阵T
        QVector<QVector<double>> Z;  ///< 正交矩阵Z
    };

    /** @brief Hessenberg三角化结果 */
    struct HessenbergResult {
        QVector<QVector<double>> H;  ///< 上Hessenberg矩阵
        QVector<QVector<double>> R;  ///< 上三角矩阵
        QVector<QVector<double>> Q;  ///< 正交变换Q
        QVector<QVector<double>> Z;  ///< 正交变换Z
    };

    /** @brief 特征值(复数形式) */
    struct EigenValuePair {
        double realPart = 0.0;         ///< 实部
        double imagPart = 0.0;         ///< 虚部
        double magnitude = 0.0;        ///< 模
        double phase = 0.0;            ///< 相角(rad)
        QVector<double> leftVector;    ///< 左特征向量
        QVector<double> rightVector;   ///< 右特征向量
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;           ///< 累计求解次数
        quint64 totalEigenvaluesFound = 0; ///< 累计特征值数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        double  lastResidual = 0.0;        ///< 最近残差
        int     lastMatrixSize = 0;        ///< 最近矩阵尺寸
    };

    explicit GeneralizedEigen(QObject* parent = nullptr);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛容差 @param tol 容差 */
    void setTolerance(double tol);

    /** @brief 求解广义特征值问题 Ax=λBx @param A 矩阵A(n×n) @param B 矩阵B(n×n) @return 特征值列表 */
    QVector<EigenValuePair> solve(const QVector<QVector<double>>& A,
                                  const QVector<QVector<double>>& B);

    /** @brief QZ分解: A=Q*S*Z^T, B=Q*T*Z^T @param A 矩阵A @param B 矩阵B @return QZResult */
    QZResult qzDecomposition(const QVector<QVector<double>>& A,
                              const QVector<QVector<double>>& B);

    /** @brief 化简为上Hessenberg-三角形式 @param A 矩阵A @param B 矩阵B @return HessenbergResult */
    HessenbergResult reduceToHessenbergTriangular(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /** @brief 从QZ分解提取特征值 @param S 上准三角 @param T 上三角 @return 特征值列表 */
    QVector<EigenValuePair> extractEigenvalues(
        const QVector<QVector<double>>& S,
        const QVector<QVector<double>>& T);

    /** @brief 计算残差 ||Ax-λBx|| @param A 矩阵A @param B 矩阵B @param eigenvalue 特征值 @param eigenvector 特征向量 @return 残差范数 */
    double residual(const QVector<QVector<double>>& A,
                    const QVector<QVector<double>>& B,
                    const EigenValuePair& eigenvalue,
                    const QVector<double>& eigenvector) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 矩阵尺寸 @param eigenvalueCount 特征值数 */
    void solveCompleted(int n, int eigenvalueCount);

private:
    void householderTriangularize(QVector<QVector<double>>& B,
                                  QVector<QVector<double>>& Q);
    void hessenbergReduce(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& B,
                          QVector<QVector<double>>& Q,
                          QVector<QVector<double>>& Z);
    void qzStep(QVector<QVector<double>>& A,
                QVector<QVector<double>>& B,
                QVector<QVector<double>>& Q,
                QVector<QVector<double>>& Z,
                int lo, int hi);
    void givensRotation(double a, double b, double& c, double& s) const;
    void applyGivensLeft(QVector<QVector<double>>& M, int i, int j,
                         double c, double s, int colStart, int colEnd);
    void applyGivensRight(QVector<QVector<double>>& M, int i, int j,
                          double c, double s, int rowStart, int rowEnd);
    QVector<QVector<double>> identityMatrix(int n) const;

    int m_maxIter;                 ///< 最大迭代次数
    double m_tol;                  ///< 收敛容差

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
