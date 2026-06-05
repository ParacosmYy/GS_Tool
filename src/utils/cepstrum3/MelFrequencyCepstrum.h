/**
 * @file MelFrequencyCepstrum.h
 * @brief MFCC梅尔频率倒谱系数 — 语音/音频特征提取
 *
 * 功能: 计算MFCC特征，支持梅尔滤波器组配置、DCT降维，
 *       统计计算次数/帧数/耗时。
 */
#ifndef MELFREQUENCYCEPSTRUM_H
#define MELFREQUENCYCEPSTRUM_H

#include <QObject>
#include <QVector>

class MelFrequencyCepstrum : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalFrames = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MelFrequencyCepstrum(int numFilters = 26, int numCoeffs = 13,
                                    QObject* parent = nullptr);

    /** @brief 计算MFCC @param signal 输入信号 @param sampleRate 采样率 @return MFCC系数矩阵[frame][coeff] */
    QVector<QVector<double>> compute(const QVector<double>& signal,
                                      double sampleRate);

    /** @brief 梅尔滤波器组能量 @param spectrum 功率谱 @return 滤波器组输出 */
    QVector<double> applyMelFilterBank(const QVector<double>& spectrum,
                                        double sampleRate) const;

    /** @brief DCT变换 @param input 输入 @param numCoeffs 系数数 @return DCT系数 */
    QVector<double> dct(const QVector<double>& input, int numCoeffs) const;

    int numFilters() const { return m_numFilters; }
    int numCoeffs() const { return m_numCoeffs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int frames, int coefficients);

private:
    double hzToMel(double hz) const;
    double melToHz(double mel) const;

    int m_numFilters;
    int m_numCoeffs;
    Stats m_stats;
    double m_timeSum;
};

#endif // MELFREQUENCYCEPSTRUM_H
