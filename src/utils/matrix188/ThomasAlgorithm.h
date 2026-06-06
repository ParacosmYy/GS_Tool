/**
 * @file ThomasAlgorithm.h
 * @brief Thomas算法/追赶法(三对角方程组+周期变体+部分选主元) — Thomas Algorithm (TDMA) for Tridiagonal Systems with Periodic Variant and Partial Pivoting
 *
 * 功能: 实现Thomas算法求解三对角线性方程组，支持标准TDMA、
 *       周期性三对角系统(CTDMA)和部分选主元提高数值稳定性。
 *
 * 协作: GaussianElimination3(高斯消元) / LU5(LU分解) / SparseSolver7(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Thomas算法求解器(TDMA+周期变体+部分选主元)
 */
class ThomasAlgorithm : public QObject {
    Q_OBJECT

public:
    /** @brief Solver variant */
    enum class Variant { Standard, Periodic, PartialPivoting };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int systemSize = 0;
        bool periodic = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm(QObject *parent = nullptr);
    ~ThomasAlgorithm() override;

    void setVariant(Variant v);

    /** @brief 求解标准三对角系统 */
    QVector<double> solve(const QVector<double>& lower,
                           const QVector<double>& main,
                           const QVector<double>& upper,
                           const QVector<double>& rhs);

    /** @brief 求解周期性三对角系统 */
    QVector<double> solvePeriodic(const QVector<double>& lower,
                                   const QVector<double>& main,
                                   const QVector<double>& upper,
                                   const QVector<double>& rhs);

    /** @brief 带部分选主元的三对角求解 */
    QVector<double> solveWithPivoting(const QVector<double>& lower,
                                       const QVector<double>& main,
                                       const QVector<double>& upper,
                                       const QVector<double>& rhs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, double timeMs);

private:
    Variant m_variant = Variant::Standard;

    Stats m_stats;
    double m_timeSum = 0.0;
};
