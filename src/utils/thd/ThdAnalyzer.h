/**
 * @file ThdAnalyzer.h
 * @brief 总谐波失真分析器 — THD/THD+N/SINAD
 *
 * 功能: 基于FFT提取基波与谐波分量，计算THD/THD+N/SINAD/ENOB，
 *       统计分析次数/谐波数/耗时。
 */
#ifndef THDANALYZER_H
#define THDANALYZER_H

#include <QObject>
#include <QVector>

class ThdAnalyzer : public QObject {
    Q_OBJECT
public:
    /** 分析结果 */
    struct Result {
        double thdDb = 0.0;                ///< THD (dB)
        double thdPercent = 0.0;           ///< THD (%)
        double thdnDb = 0.0;               ///< THD+N (dB)
        double sinadDb = 0.0;              ///< SINAD (dB)
        double enob = 0.0;                 ///< 有效位数
        double fundamentalFreq = 0.0;      ///< 基波频率(Hz)
        double fundamentalPower = 0.0;     ///< 基波功率(dB)
        QVector<QPair<int, double>> harmonics; ///< (谐波次数, 功率dB)
    };

    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalHarmonicsDetected = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit ThdAnalyzer(QObject* parent = nullptr);

    /** @brief 设置参数 @param maxHarmonics 最大谐波次数 @param sampleRate 采样率 */
    void setParameters(int maxHarmonics, double sampleRate);

    /** @brief 分析THD @param data 信号 @return 分析结果 */
    Result analyze(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(double thdDb, double sinadDb);

private:
    /** FFT(简化DFT) */
    void computeMagnitude(const QVector<double>& data,
                          QVector<double>& magnitude) const;
    /** 寻找峰值频率 */
    int findPeakBin(const QVector<double>& magnitude, int searchStart,
                    int searchEnd) const;

    int m_maxHarmonics;
    double m_sampleRate;
    Stats m_stats;
    double m_timeSum;
};

#endif // THDANALYZER_H
