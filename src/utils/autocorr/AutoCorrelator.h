/**
 * @file AutoCorrelator.h
 * @brief 自相关引擎 — 周期性检测/基频估计/自相似度分析
 *
 * 功能: 计算数据序列的自相关函数，检测周期性，
 *       估计基频，支持标准/归一化/有偏/无偏估计。
 *
 * 协作: CrossCorrelator(互相关) / PeakDetector(峰值检测)
 */
#ifndef AUTOCORRELATOR_H
#define AUTOCORRELATOR_H

#include <QObject>
#include <QVector>
#include <QPair>

class AutoCorrelator : public QObject {
    Q_OBJECT

public:
    /** @brief 计算方法 */
    enum class Method {
        Standard,       ///< 标准自相关
        FFT,            ///< FFT加速(回退到标准)
        Biased,         ///< 有偏估计(除以N)
        Unbiased        ///< 无偏估计(除以N-k)
    };
    Q_ENUM(Method)

    /** @brief 相关结果 */
    struct CorrelationResult {
        QVector<double> lags;           ///< 滞后序列
        QVector<double> values;         ///< 自相关值
        int peakLag = 0;                ///< 峰值滞后
        double peakValue = 0.0;         ///< 峰值相关
        double fundamentalFrequency = 0.0; ///< 基频
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;      ///< 累计计算次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  peakCorrelation = 0.0;      ///< 峰值相关系数
        quint64 totalPeriodicDetected = 0;  ///< 累计检测周期数
    };

    explicit AutoCorrelator(QObject* parent = nullptr);

    void setMethod(Method method);
    void setMaxLag(int maxLag);

    /** @brief 计算自相关 @param data 数据 @return 结果 */
    CorrelationResult compute(const QVector<double>& data);

    /** @brief 检测周期性 @param data 数据 @return 是否周期性 */
    bool detectPeriodicity(const QVector<double>& data);

    /** @brief 估计基频 @param data 数据 @param sampleRate 采样率 @return 基频(Hz) */
    double findFundamentalFrequency(const QVector<double>& data,
                                     double sampleRate);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationComplete(int lagCount, double peakValue);

private:
    Method m_method;
    int m_maxLag;
    Stats m_stats;
};

#endif // AUTOCORRELATOR_H
