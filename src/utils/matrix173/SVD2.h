/**
 * @file SVD2.h
 * @brief 全SVD(Golub-Kahan双对角化+隐式QR零追赶) — Full SVD via Bidiagonalization with Golub-Kahan and Implicit QR Zero-Chasing
 *
 * 功能: 实现全SVD分解，先双对角化(Golub-Kahan)，再隐式QR迭代追赶超对角零元素，
 *       支持奇异值排序、条件数计算和低秩近似。
 *
 * 协作: EigenDecomposition(特征值分解) / PCA2(主成分分析) / LeastSquares3(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解器
 */
class SVD2 : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct Result {
        QVector<QVector<double>> U;     ///< 左奇异向量矩阵
        QVector<double> singularValues;  ///< 奇异值(降序)
        QVector<QVector<double>> V;      ///< 右奇异向量矩阵
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        int lastRows = 0;                 ///< 最近行数
        int lastCols = 0;                 ///< 最近列数
        int lastIterations = 0;           ///< 最近QR迭代数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit SVD2(QObject *parent = nullptr);
    ~SVD2() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /**
     * @brief 执行全SVD分解: A = U * Σ * V^T
     * @param A 输入矩阵(m×n)
     * @return 分解结果
     */
    Result decompose(const QVector<QVector<double>>& A);

    /** @brief 计算条件数(最大奇异值/最小奇异值) */
    double conditionNumber(const QVector<QVector<double>>& A);

    /** @brief 计算矩阵秩(大于阈值的奇异值个数) */
    int rank(const QVector<QVector<double>>& A, double threshold = 1e-10);

    /** @brief 低秩近似: 返回秩r近似矩阵 */
    QVector<QVector<double>> lowRankApprox(const Result& svd, int r) const;

    /** @brief 伪逆: A^+ = V * Σ^+ * U^T */
    QVector<QVector<double>> pseudoInverse(const Result& svd) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param rows 行数 @param cols 列数 */
    void decompositionCompleted(int rows, int cols);

private:
    /** @brief Householder反射变换 */
    void householderBidiagonalize(QVector<QVector<double>>& A,
                                  QVector<QVector<double>>& U,
                                  QVector<QVector<double>>& V);

    /** @brief 隐式QR零追赶(Golub-Kahan) */
    void implicitQRStep(QVector<double>& diag, QVector<double>& superDiag,
                        QVector<QVector<double>>& U, QVector<QVector<double>>& V,
                        int lo, int hi);

    /** @brief Golub-Kahan SVD步 */
    void golubKahanStep(QVector<double>& diag, QVector<double>& superDiag,
                        QVector<QVector<double>>& U, QVector<QVector<double>>& V,
                        int lo, int hi);

    /** @brief Givens旋转应用于行 */
    static void applyGivensLeft(QVector<QVector<double>>& M, int i, int j,
                                double c, double s);
    /** @brief Givens旋转应用于列 */
    static void applyGivensRight(QVector<QVector<double>>& M, int i, int j,
                                 double c, double s);
    /** @brief 单位矩阵 */
    static QVector<QVector<double>> identity(int n);

    int m_maxIter = 1000;
    double m_tol = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;
};
