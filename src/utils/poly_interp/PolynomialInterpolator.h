/**
 * @file PolynomialInterpolator.h
 * @brief 多项式插值引擎
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多项式插值引擎
 *
 * 支持Lagrange插值、Newton差商插值和Barycentric插值,
 * 自动选择最优方法。
 */
class PolynomialInterpolator : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInterpolations = 0;     ///< 总插值次数
        int totalEvaluations = 0;        ///< 总求值次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit PolynomialInterpolator(QObject* parent = nullptr);

    /**
     * @brief 设置插值节点
     * @param x X坐标数组
     * @param y Y坐标数组
     */
    void setPoints(const QVector<double>& x, const QVector<double>& y);

    /**
     * @brief Lagrange插值求值
     * @param x 求值点
     * @return 插值结果
     */
    double lagrange(double x) const;

    /**
     * @brief Newton差商插值求值
     * @param x 求值点
     * @return 插值结果
     */
    double newton(double x) const;

    /**
     * @brief Barycentric插值求值(第二型)
     * @param x 求值点
     * @return 插值结果
     */
    double barycentric(double x) const;

    /**
     * @brief 批量求值
     * @param xPoints 求值点数组
     * @return 插值结果数组
     */
    QVector<double> evaluateBatch(const QVector<double>& xPoints);

    /**
     * @brief 计算Newton差商表
     * @return 差商系数
     */
    QVector<double> dividedDifferences() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 插值完成信号 */
    void interpolationCompleted(int pointCount, double evalCount);

private:
    QVector<double> m_x;           ///< X坐标
    QVector<double> m_y;           ///< Y坐标
    QVector<double> m_diffTable;   ///< 差商表
    QVector<double> m_baryWeights; ///< 重心权重

    Stats m_stats;
    double m_timeSum = 0.0;

    void computeDividedDifferences();
    void computeBarycentricWeights();
};
