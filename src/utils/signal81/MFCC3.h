#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MFCC3 - 梅尔频率倒谱系数提取器
 *
 * 从音频信号提取MFCC特征，支持可配置梅尔滤波器组
 * 和delta/delta-delta系数，用于语音识别和音频分类。
 */
class MFCC3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalCoefficients = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MFCC3(QObject* parent = nullptr);

    /** @brief 设置采样率、帧长和系数数 */
    void initialize(double sampleRate, int fftSize, int numCoeffs = 13);

    /** @brief 计算一帧的MFCC系数 */
    QVector<double> compute(const QVector<double>& frame);

    /** @brief 计算delta和delta-delta系数 */
    QVector<QVector<double>> computeWithDeltas(const QVector<double>& frame);

    /** @brief 获取梅尔滤波器组能量 */
    QVector<double> melEnergies(const QVector<double>& frame) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coefficientsComputed(int numCoeffs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 16000.0;
    int m_fftSize = 512;
    int m_numCoeffs = 13;
};
