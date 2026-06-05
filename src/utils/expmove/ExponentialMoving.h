/**
 * @file ExponentialMoving.h
 * @brief 指数移动平均/方差/协方差 — 流式EMA统计
 *
 * 功能: 流式计算指数移动平均(EMA)/方差(EMVar)/协方差(EMCov)，
 *       支持多alpha通道，统计更新次数/耗时。
 */
#ifndef EXPONENTIALMOVING_H
#define EXPONENTIALMOVING_H

#include <QObject>
#include <QVector>

class ExponentialMoving : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalResets = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ExponentialMoving(double alpha = 0.1, QObject* parent = nullptr);

    /** @brief 设置平滑因子 @param alpha [0,1] */
    void setAlpha(double alpha);

    /** @brief 更新EMA @param value 新值 @return 当前EMA */
    double update(double value);

    /** @brief 更新EM方差 @param value 新值 @return 当前EM方差 */
    double updateVariance(double value);

    /** @brief 批量更新 @param values 数值序列 @return EMA序列 */
    QVector<double> updateBatch(const QVector<double>& values);

    /** @brief 计算两个序列的EM协方差 @param x X值 @param y Y值 @return EMCov */
    double updateCovariance(double x, double y);

    /** @brief 当前EMA值 */
    double value() const { return m_ema; }

    /** @brief 当前EM方差 */
    double variance() const { return m_emvar; }

    /** @brief 当前EM标准差 */
    double stdDev() const;

    /** @brief 是否已初始化 */
    bool isInitialized() const { return m_initialized; }

    void reset();

    double alpha() const { return m_alpha; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueUpdated(double ema, double variance);

private:
    double m_alpha;
    double m_ema;
    double m_emvar;
    double m_emcov_x;
    double m_emcov_y;
    double m_emcov;
    bool m_initialized;
    Stats m_stats;
    double m_timeSum;
};

#endif // EXPONENTIALMOVING_H
