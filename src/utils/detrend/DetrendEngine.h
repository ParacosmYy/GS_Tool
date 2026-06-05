/**
 * @file DetrendEngine.h
 * @brief 去趋势引擎 — 从数据中移除趋势分量
 *
 * 功能: 支持常数/线性/多项式/移动平均去趋势，
 *       统计处理次数/平均残余方差。
 */
#ifndef DETRENDENGINE_H
#define DETRENDENGINE_H

#include <QObject>
#include <QVector>

/**
 * @class DetrendEngine
 * @brief 从数据序列中移除趋势，保留波动分量
 */
class DetrendEngine : public QObject {
    Q_OBJECT
public:
    /** 去趋势方法 */
    enum class Method {
        Constant,       ///< 减去均值
        Linear,         ///< 线性回归去趋势
        Polynomial,     ///< 多项式拟合去趋势
        MovingAverage   ///< 移动平均去趋势
    };

    /** 去趋势结果 */
    struct DetrendResult {
        QVector<double> detrended;      ///< 去趋势后数据
        QVector<double> trend;          ///< 提取的趋势
        double originalVariance = 0.0;
        double residualVariance = 0.0;
        double varianceReduction = 0.0; ///< 方差降低百分比
    };

    /** 统计 */
    struct Stats {
        quint64 totalDetrends = 0;
        double  avgVarianceReduction = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit DetrendEngine(QObject* parent = nullptr);

    void setMethod(Method m);
    void setPolynomialDegree(int degree);
    void setWindowSize(int size);

    /** 执行去趋势 */
    DetrendResult detrend(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detrendComplete(double varianceReduction);

private:
    Method m_method;
    int m_polyDegree;
    int m_windowSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // DETRENDENGINE_H
