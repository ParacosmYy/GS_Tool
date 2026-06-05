#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Gabor变换时频分析
 *
 * 短时傅里叶变换的特例，使用高斯窗进行时频分析。
 */
class GaborTransform3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalWindowsApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaborTransform3(QObject* parent = nullptr);

    /** @brief 执行Gabor变换 */
    QVector<QVector<double>> transform(const QVector<double>& samples,
                                        double sigma, int hopSize);

    /** @brief 逆Gabor变换重建信号 */
    QVector<double> inverse(const QVector<QVector<double>>& coefficients, int outputLength);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frameCount, int freqBins);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sigma = 1.0;
};
