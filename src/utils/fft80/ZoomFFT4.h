#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ZoomFFT4 - 细化FFT(Zoom FFT)
 *
 * 在指定频段内进行高分辨率频谱分析，通过频移、
 * 低通滤波和降采样实现频段细化，等效于超长FFT。
 */
class ZoomFFT4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalZoomPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT4(QObject* parent = nullptr);

    /** @brief 设置细化频段和输出点数 */
    bool setZoomRange(double centerFreq, double bandwidth, int outputPoints);

    /** @brief 执行细化FFT分析 */
    QVector<std::complex<double>> transform(const QVector<double>& input);

    /** @brief 获取幅度谱 */
    QVector<double> magnitudeSpectrum(const QVector<double>& input);

    /** @brief 设置等效FFT大小 */
    void setEquivalentSize(int size);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int outputPoints, double resolutionHz);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_centerFreq = 0.0;
    double m_bandwidth = 0.0;
    int m_outputPoints = 0;
    int m_equivalentSize = 0;
};
