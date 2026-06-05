#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ZeroCrossing3 - 过零率分析器
 *
 * 计算信号的过零率及其统计特征，用于语音/非语音
 * 判别、基音估计和信号频率粗估计。
 */
class ZeroCrossing3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalZeroCrossings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZeroCrossing3(QObject* parent = nullptr);

    /** @brief 设置采样率 */
    void setSampleRate(double sampleRate);

    /** @brief 计算一帧的过零率(交叉次数/帧长) */
    double zeroCrossingRate(const QVector<double>& frame) const;

    /** @brief 计算过零点对应的估计频率(Hz) */
    double estimatedFrequency(const QVector<double>& frame) const;

    /** @brief 检测帧是否为有声/无声(基于过零率阈值) */
    bool isVoiced(const QVector<double>& frame, double threshold = 0.1) const;

    /** @brief 获取过零点的精确位置(插值) */
    QVector<double> crossingPositions(const QVector<double>& frame) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void crossingRateComputed(double rate, double frequencyHz);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
};
