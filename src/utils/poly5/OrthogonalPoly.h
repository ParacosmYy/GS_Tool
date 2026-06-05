/**
 * @file OrthogonalPoly.h
 * @brief 正交多项式族 — Legendre/Hermite/Laguerre求值与递推
 *
 * 功能: 实现三大经典正交多项式族(Legendre, Hermite, Laguerre)的
 *       递推求值、高斯求积节点/权重计算、以及多项式拟合。
 *       基于三项递推关系保证数值稳定性。
 *
 * 协作: AdamsIntegrator(数值积分) / AnomalyDetector(特征展开)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>

/**
 * @brief 正交多项式类型
 */
enum class PolyFamily {
    Legendre,   ///< 勒让德多项式 [-1,1] 权函数 w(x)=1
    Hermite,    ///< 埃尔米特多项式 (-∞,+∞) 权函数 w(x)=e^{-x^2}
    Laguerre    ///< 拉盖尔多项式 [0,+∞) 权函数 w(x)=e^{-x}
};

/**
 * @brief 正交多项式求值与求积工具
 */
class OrthogonalPoly : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalEvaluations = 0;     ///< 累计求值次数
        int    totalQuadratures = 0;     ///< 累计求积计算次数
        int    totalFits = 0;            ///< 累计拟合次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit OrthogonalPoly(QObject* parent = nullptr);

    /**
     * @brief 计算正交多项式在x处的值
     * @param family 多项式族
     * @param n 阶数(从0开始)
     * @param x 求值点
     * @return P_n(x) 的值
     */
    double evaluate(PolyFamily family, int n, double x) const;

    /**
     * @brief 批量计算多个阶数在x处的值
     * @param family 多项式族
     * @param maxOrder 最大阶数(计算0到maxOrder)
     * @param x 求值点
     * @return 值数组 [P_0(x), P_1(x), ..., P_maxOrder(x)]
     */
    QVector<double> evaluateRange(PolyFamily family, int maxOrder,
                                  double x) const;

    /**
     * @brief 计算高斯求积节点和权重
     * @param family 多项式族(Legendre/Laguerre)
     * @param nPoints 求积点数
     * @return (节点数组, 权重数组)
     */
    QPair<QVector<double>, QVector<double>>
    gaussQuadrature(PolyFamily family, int nPoints);

    /**
     * @brief 使用正交多项式进行最小二乘拟合
     * @param family 多项式族
     * @param xData 自变量数据
     * @param yData 因变量数据
     * @param degree 拟合阶数
     * @return 展开系数 [c_0, c_1, ..., c_degree]
     */
    QVector<double> polyFit(PolyFamily family,
                            const QVector<double>& xData,
                            const QVector<double>& yData,
                            int degree);

    /**
     * @brief 使用展开系数计算拟合值
     * @param family 多项式族
     * @param coeffs 展开系数
     * @param x 求值点
     * @return f(x) = sum c_k * P_k(x)
     */
    double evaluateFit(PolyFamily family, const QVector<double>& coeffs,
                       double x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求积完成 @param nPoints 点数 */
    void quadratureCompleted(int nPoints);
    /** @brief 拟合完成 @param degree 阶数 */
    void fitCompleted(int degree);

private:
    /** @brief Legendre三项递推 */
    double legendreRecurse(int n, double x) const;
    /** @brief Hermite三项递推 */
    double hermiteRecurse(int n, double x) const;
    /** @brief Laguerre三项递推 */
    double laguerreRecurse(int n, double x) const;

    /** @brief 求根Newton迭代 */
    double findRoot(PolyFamily family, int n, double guess,
                    double a, double b) const;

    /** @brief 计算内积 */
    double innerProduct(PolyFamily family, int k,
                        const QVector<double>& xData,
                        const QVector<double>& yData,
                        const QVector<double>& weights) const;

    mutable Stats              m_stats;
    mutable double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
