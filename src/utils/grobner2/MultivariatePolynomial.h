/**
 * @file MultivariatePolynomial.h
 * @brief 多变量多项式算术运算
 *
 * 功能: 多变量多项式的加法、乘法、求值，
 *       统计操作次数/耗时。
 */
#ifndef MULTIVARIATEPOLYNOMIAL_H
#define MULTIVARIATEPOLYNOMIAL_H

#include <QObject>
#include <QVector>

class MultivariatePolynomial : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalOperations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MultivariatePolynomial(QObject* parent = nullptr);

    /**
     * @brief 多项式加法(系数逐项相加)
     * @param coeffs1 第一个多项式系数(高次→低次)
     * @param coeffs2 第二个多项式系数(高次→低次)
     * @return 和多项式系数
     */
    QVector<double> add(const QVector<double>& coeffs1,
                         const QVector<double>& coeffs2);

    /**
     * @brief 多项式乘法(卷积)
     * @param coeffs1 第一个多项式系数(高次→低次)
     * @param coeffs2 第二个多项式系数(高次→低次)
     * @return 积多项式系数
     */
    QVector<double> multiply(const QVector<double>& coeffs1,
                              const QVector<double>& coeffs2);

    /**
     * @brief 多变量多项式求值(Horner法/逐项展开)
     * @param coeffs 多项式系数(高次→低次)
     * @param variables 变量值列表(按阶对应)
     * @return 多项式在给定变量值处的值
     */
    double evaluate(const QVector<double>& coeffs,
                     const QVector<double>& variables);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成 @param operationType 操作类型(0=add,1=multiply,2=evaluate) */
    void operationCompleted(int operationType);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // MULTIVARIATEPOLYNOMIAL_H
