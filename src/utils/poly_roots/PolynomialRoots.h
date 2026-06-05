/**
 * @file PolynomialRoots.h
 * @brief 多项式求根 — 实数/复数根的高精度求解
 *
 * 功能: 提供多项式求根算法，包括同伴矩阵QR分解法和
 *       Aberth迭代法，支持实数根过滤和多项式求值。
 *
 * 协作: DataInterpolator(插值) / NumericalDerivative(导数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 多项式求根器
 */
class PolynomialSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalComputed = 0;       ///< 累计求根次数
        quint64 totalRoots = 0;          ///< 累计找到的根数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PolynomialSolver(QObject* parent = nullptr);

    /**
     * @brief 求多项式所有根(含复数)
     * @param coeffs 系数向量(高次到低次，如[1,0,-1]表示x^2-1)
     * @return 根的实部列表(复数根的虚部存储在内部)
     */
    QVector<double> findAllRoots(const QVector<double>& coeffs);

    /**
     * @brief 仅求实数根(虚部<阈值)
     * @param coeffs 系数向量
     * @return 实数根列表
     */
    QVector<double> findRealRoots(const QVector<double>& coeffs);

    /**
     * @brief 求多项式在某点的值
     * @param coeffs 系数向量
     * @param x 自变量
     * @return 多项式值
     */
    double evaluate(const QVector<double>& coeffs, double x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求根完成 @param rootCount 根的数量 */
    void computed(int rootCount);

private:
    /** @brief 复数根结构 */
    struct Complex {
        double re = 0.0;  ///< 实部
        double im = 0.0;  ///< 虚部
    };

    /** @brief 同伴矩阵QR分解求根 */
    QVector<Complex> solveCompanionQR(const QVector<double>& coeffs) const;

    /** @brief 对复数向量做QR迭代 */
    void qrIteration(QVector<QVector<Complex>>& mat) const;

    /** @brief Aberth迭代优化根的精度 */
    void aberthRefine(const QVector<double>& coeffs,
                      QVector<Complex>& roots) const;

    /** @brief 复数求多项式值 */
    Complex evalComplex(const QVector<double>& coeffs,
                        const Complex& z) const;

    /** @brief 复数乘法 */
    static Complex cmul(const Complex& a, const Complex& b);

    /** @brief 复数加法 */
    static Complex cadd(const Complex& a, const Complex& b);

    /** @brief 复数除法 */
    static Complex cdiv(const Complex& a, const Complex& b);

    static constexpr double IMAG_THRESHOLD = 1e-8; ///< 虚部阈值
    static constexpr int    MAX_ITER = 50;          ///< 最大迭代次数

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
