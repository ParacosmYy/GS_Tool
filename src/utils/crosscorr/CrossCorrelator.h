/**
 * @file CrossCorrelator.h
 * @brief 互相关引擎 — 两路信号的时延/相似度/相位分析
 *
 * 功能: 计算两路数据的互相关函数，检测信号间时延关系，
 *       支持标准/归一化/相位相关三种模式。
 *
 * 协作: AutoCorrelator(自相关) / DataSynchronizer(时间对齐)
 */
#ifndef CROSSCORRELATOR_H
#define CROSSCORRELATOR_H

#include <QObject>
#include <QVector>
#include <QPair>

class CrossCorrelator : public QObject {
    Q_OBJECT

public:
    /** @brief 相关方法 */
    enum class Method {
        Standard,       ///< 标准互相关
        Normalized,     ///< 归一化互相关
        Phase           ///< 相位相关(频域)
    };
    Q_ENUM(Method)

    /** @brief 相关结果 */
    struct Result {
        QVector<double> lags;       ///< 滞后值
        QVector<double> values;     ///< 相关值
        int peakLag = 0;            ///< 峰值滞后
        double peakValue = 0.0;     ///< 峰值相关
        double delay = 0.0;         ///< 检测到的时延
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;      ///< 累计计算次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  peakCorrelation = 0.0;      ///< 峰值相关系数
        quint64 totalDelaysDetected = 0;    ///< 累计检测时延数
    };

    explicit CrossCorrelator(QObject* parent = nullptr);

    /** @brief 设置相关方法 @param method 方法 */
    void setMethod(Method method);

    /** @brief 设置最大滞后 @param maxLag 最大滞后点数 */
    void setMaxLag(int maxLag);

    /** @brief 计算互相关 @param x 第一路信号 @param y 第二路信号 @return 结果 */
    Result compute(const QVector<double>& x, const QVector<double>& y);

    /** @brief 检测两路信号间时延 @param x 第一路 @param y 第二路 @return 时延(采样点数) */
    double detectDelay(const QVector<double>& x, const QVector<double>& y);

    /** @brief 计算两路信号的相似度 @param x 第一路 @param y 第二路 @return 相似度[0,1] */
    double similarity(const QVector<double>& x, const QVector<double>& y);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param peakLag 峰值滞后 @param peakValue 峰值 */
    void computationComplete(int peakLag, double peakValue);

private:
    Result computeStandard(const QVector<double>& x, const QVector<double>& y);
    Result computeNormalized(const QVector<double>& x, const QVector<double>& y);

    Method m_method;        ///< 相关方法
    int m_maxLag;           ///< 最大滞后
    Stats m_stats;
};

#endif // CROSSCORRELATOR_H
