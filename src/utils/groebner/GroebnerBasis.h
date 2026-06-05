/**
 * @file GroebnerBasis.h
 * @brief Groebner基计算 — Buchberger算法(单变量简化版)
 *
 * 功能: 对单变量多项式集合计算Groebner基，
 *       统计计算次数/耗时。
 */
#ifndef GROEBNERBASIS_H
#define GROEBNERBASIS_H

#include <QObject>
#include <QVector>

class GroebnerBasis : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit GroebnerBasis(QObject* parent = nullptr);

    /**
     * @brief 计算多项式集合的Groebner基(单变量Buchberger算法)
     * @param polynomials 多项式集合，每个QVector<double>为一多项式(高次→低次)
     * @return Groebner基(化简后)
     */
    QVector<QVector<double>> compute(const QVector<QVector<double>>& polynomials);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param basisSize 基的大小 */
    void computationCompleted(int basisSize);

private:
    /** @brief 计算两个单变量多项式的S-多项式 */
    QVector<double> sPolynomial(const QVector<double>& f,
                                 const QVector<double>& g);

    /** @brief 用多项式集合归约一个多项式 */
    QVector<double> reduce(QVector<double> p,
                            const QVector<QVector<double>>& basis);

    /** @brief 多项式除法求余: a mod b */
    QVector<double> polyMod(const QVector<double>& a,
                             const QVector<double>& b);

    /** @brief 去除前导零系数 */
    static QVector<double> strip(const QVector<double>& p);

    /** @brief 多项式乘以标量 */
    static QVector<double> scale(const QVector<double>& p, double s);

    /** @brief 多项式首一化 */
    static QVector<double> monic(const QVector<double>& p);

    Stats m_stats;
    double m_timeSum;
};

#endif // GROEBNERBASIS_H
