#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ShortTimeFFT5 - 短时傅里叶变换(STFT)
 *
 * 支持多种窗函数和跳步长配置的STFT实现，
 * 提供时频矩阵输出和逆变换重建。
 */
class ShortTimeFFT5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ShortTimeFFT5(QObject* parent = nullptr);

    /** @brief 配置FFT大小、跳步和窗函数类型 */
    bool configure(int fftSize, int hopSize, const QString& windowType = "hann");

    /** @brief 执行STFT，返回时频矩阵[frame][bin] */
    QVector<QVector<std::complex<double>>> forward(const QVector<double>& signal);

    /** @brief 执行逆STFT重建时域信号 */
    QVector<double> inverse(const QVector<QVector<std::complex<double>>>& stftMatrix);

    /** @brief 获取幅度谱矩阵 */
    QVector<QVector<double>> magnitudeMatrix(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_fftSize = 1024;
    int m_hopSize = 512;
    QString m_windowType;
};
