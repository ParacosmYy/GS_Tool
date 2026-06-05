#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 快速卷积处理器
 *
 * 基于FFT的重叠保留/重叠相加快速卷积。
 */
class FastConv4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalConvolutions = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FastConv4(QObject* parent = nullptr);

    /** @brief 设置卷积核 */
    void setKernel(const QVector<double>& kernel);

    /** @brief 处理音频块(重叠保留法) */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void convolutionCompleted(int inputSize, int kernelSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_kernel;
    QVector<double> m_overlapBuffer;
};
