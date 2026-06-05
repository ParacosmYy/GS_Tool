#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PolyPhase3 - 多相FFT滤波器组
 *
 * 使用多相分解实现高效的均匀DFT滤波器组，
 * 支持分析和综合(重建)，适用于频分复用和子带编码。
 */
class PolyPhase3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFilterbanks = 0;
        int totalSubbands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolyPhase3(QObject* parent = nullptr);

    /** @brief 设置通道数和原型滤波器长度 */
    bool configure(int channels, int filterLength = 0);

    /** @brief 分析滤波器组: 时域转频域子带 */
    QVector<QVector<std::complex<double>>> analysis(const QVector<double>& input);

    /** @brief 综合滤波器组: 频域子带转时域 */
    QVector<double> synthesis(const QVector<QVector<std::complex<double>>>& subbands);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterbankProcessed(int channels, int samples);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_channels = 64;
    int m_filterLength = 0;
    QVector<double> m_prototype;
};
