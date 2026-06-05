/**
 * @file PolynomialResultant.h
 * @brief 多项式结式 — Sylvester矩阵法
 *
 * 功能: 计算两个多项式的结式(行列式)，多项式GCD，
 *       统计计算次数/耗时。
 */
#ifndef POLYNOMIALRESULTANT_H
#define POLYNOMIALRESULTANT_H

#include <QObject>
#include <QVector>

class PolynomialResultant : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit PolynomialResultant(QObject* parent = nullptr);

    /**
     * @brief 计算两个多项式的结式(Sylvester矩阵行列式)
     * @param p 第一个多项式系数(高次→低次)
     * @param q 第二个多项式系数(高次→低次)
     * @return 结式值
     */
    double compute(const QVector<double>& p, const QVector<double>& q);

    /**
     * @brief 计算两个多项式的GCD(辗转相除法)
     * @param p 第一个多项式系数(高次→低次)
     * @param q 第二个多项式系数(高次→低次)
     * @return GCD多项式系数(高次→低次，首一化)
     */
    QVector<double> gcd(const QVector<double>& p, const QVector<double>& q);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param resultant 结式值 */
    void computationCompleted(double resultant);

private:
    /** @brief 多项式除法: a = q*b + r, 返回(r, q) */
    std::pair<QVector<double>, QVector<double>> polyDivide(
        const QVector<double>& a, const QVector<double>& b);

    /** @brief 计算n*n矩阵行列式(LU分解) */
    double determinant(QVector<double>& mat, int n);

    /** @brief 去除前导零系数 */
    static QVector<double> stripLeadingZeros(const QVector<double>& p);

    Stats m_stats;
    double m_timeSum;
};

#endif // POLYNOMIALRESULTANT_H
