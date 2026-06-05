/**
 * @file SignalDecomposer.h
 * @brief 信号分解引擎 — 将复合信号分解为趋势+季节+残差
 *
 * 功能: 支持移动平均分解(STL-like)和差分分解，提取趋势/季节/残差分量。
 *
 * 协作: SpectrumAnalyzer(频域分析) / TrendPredictor(趋势预测)
 */
#ifndef SIGNALDECOMPOSER_H
#define SIGNALDECOMPOSER_H

#include <QObject>
#include <QVector>

class SignalDecomposer : public QObject {
    Q_OBJECT
public:
    struct DecompositionResult {
        QVector<double> trend;      ///< 趋势分量
        QVector<double> seasonal;   ///< 季节分量
        QVector<double> residual;   ///< 残差分量
    };

    struct Stats {
        quint64 totalDecompositions = 0;
        double  averageTrendStrength = 0.0;
        double  averageSeasonalStrength = 0.0;
        int     peakPeriod = 0;
    };

    explicit SignalDecomposer(QObject* parent = nullptr);

    void setPeriod(int period);
    DecompositionResult decompose(const QVector<double>& data);
    int detectPeriod(const QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionComplete(int dataSize);

private:
    QVector<double> movingAverage(const QVector<double>& data, int window) const;

    int m_period;
    Stats m_stats;
    double m_trendSum, m_seasonalSum;
};

#endif // SIGNALDECOMPOSER_H
