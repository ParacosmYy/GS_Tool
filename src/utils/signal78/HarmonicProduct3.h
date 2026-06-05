#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief HarmonicProduct3 - 谐波乘积谱基音检测
 *
 * 通过谐波乘积谱(HPS)方法检测基音频率，
 * 将频谱与其下采样版本相乘以增强基频分量。
 */
class HarmonicProduct3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalPitchEstimates = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HarmonicProduct3(QObject* parent = nullptr);

    /** @brief 设置采样率和HPS阶数 */
    void initialize(double sampleRate, int harmonics = 5);

    /** @brief 从频谱幅度检测基音频率 */
    double detectFromSpectrum(const QVector<double>& magnitude);

    /** @brief 处理时域帧(内部先做FFT) */
    double detectFromFrame(const QVector<double>& frame);

    /** @brief 获取HPS谱 */
    QVector<double> harmonicProductSpectrum() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchEstimated(double frequencyHz, double confidence);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    int m_harmonics = 5;
    QVector<double> m_hps;
};
