#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ChirpZ4 - Chirp-Z变换
 *
 * 在Z平面上沿任意螺旋轮廓计算频率响应，
 * 比FFT更灵活的频率分析，支持频段细化(zoom)。
 */
class ChirpZ4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalOutputPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChirpZ4(QObject* parent = nullptr);

    /** @brief 设置输出点数和螺旋参数A,W */
    bool setParameters(int outputPoints, const std::complex<double>& A,
                       const std::complex<double>& W);

    /** @brief 执行Chirp-Z变换 */
    QVector<std::complex<double>> transform(const QVector<double>& input);

    /** @brief 指定频率范围进行细化分析 */
    QVector<std::complex<double>> zoom(double freqStart, double freqEnd,
                                        int numPoints, const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int outputPoints);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_outputPoints = 0;
    std::complex<double> m_A;
    std::complex<double> m_W;
};
