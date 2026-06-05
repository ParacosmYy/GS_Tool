/**
 * @file FractalDimension.h
 * @brief 分形维数估计器 — 盒计数/Higuchi方法
 *
 * 功能: 盒计数法计算分形维数，Higuchi分形维数，
 *       信号复杂度评估，统计计算次数/维数/耗时。
 */
#ifndef FRACTALDIMENSION_H
#define FRACTALDIMENSION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 分形维数估计器
 */
class FractalDimension : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalEstimations = 0;  ///< 累计估计次数
        double  lastBoxCountDim = 0.0;  ///< 最近盒计数维数
        double  lastHiguchiDim = 0.0;   ///< 最近Higuchi维数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit FractalDimension(QObject* parent = nullptr);

    /** @brief 盒计数法 @param data 信号数据 @param minSize 最小盒子尺寸 @param maxSize 最大盒子尺寸 @return 分形维数 */
    double boxCountingDimension(const QVector<double>& data,
                                int minSize = 2, int maxSize = 0);

    /** @brief 盒计数详细结果 @param data 信号数据 @param minSize 最小尺寸 @param maxSize 最大尺寸 @return (log(size), log(count))点对 */
    QVector<QPair<double, double>> boxCountingCurve(
        const QVector<double>& data, int minSize = 2, int maxSize = 0);

    /** @brief Higuchi分形维数 @param data 信号数据 @param maxK 最大k值 @return 分形维数 */
    double higuchiDimension(const QVector<double>& data, int maxK = 0);

    /** @brief 信号复杂度评估 @param data 信号数据 @return 复杂度指标[0,1] */
    double complexityMeasure(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 维数估计完成 @param dimension 分形维数 @param method 方法名 */
    void dimensionEstimated(double dimension, const QString& method);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // FRACTALDIMENSION_H
