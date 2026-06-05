/**
 * @file ConstantQTransform.cpp
 * @brief 常数Q变换实现
 */

#include "ConstantQTransform.h"
#include <QElapsedTimer>
#include <cmath>

ConstantQTransform::ConstantQTransform(double minFreq, double maxFreq,
                                         int binsPerOctave, double sampleRate,
                                         QObject* parent)
    : QObject(parent)
    , m_minFreq(minFreq)
    , m_maxFreq(maxFreq)
    , m_binsPerOctave(binsPerOctave)
    , m_sampleRate(sampleRate)
    , m_timeSum(0.0)
{
    int octaves = static_cast<int>(std::ceil(std::log2(m_maxFreq / m_minFreq)));
    m_numBins = octaves * m_binsPerOctave;
    m_qualityFactor = 1.0 / (std::pow(2.0, 1.0 / m_binsPerOctave) - 1.0);
}

QVector<QVector<double>> ConstantQTransform::transform(
    const QVector<double>& signal) const
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    int fftSize = 1;
    while (fftSize < N) fftSize *= 2;

    /* 预计算FFT(简化DFT) */
    QVector<double> magnitude(fftSize / 2 + 1, 0.0);
    for (int k = 0; k <= fftSize / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double w = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (N - 1)));
            double angle = -2.0 * M_PI * k * n / fftSize;
            re += signal[n] * w * std::cos(angle);
            im += signal[n] * w * std::sin(angle);
        }
        magnitude[k] = std::sqrt(re * re + im * im);
    }

    /* 构建CQT核: 每个bin对应一个频率范围 */
    QVector<QVector<double>> result(m_numBins, QVector<double>(1, 0.0));

    for (int bin = 0; bin < m_numBins; ++bin) {
        double freq = m_minFreq * std::pow(2.0, static_cast<double>(bin) / m_binsPerOctave);
        double q = m_qualityFactor;
        int filterLen = static_cast<int>(q * m_sampleRate / freq);

        /* 从FFT幅度中提取该bin附近的能量 */
        int centerBin = static_cast<int>(freq * fftSize / m_sampleRate);
        int halfBand = qMax(1, static_cast<int>(filterLen / 2));

        double energy = 0.0;
        int count = 0;
        for (int k = qMax(0, centerBin - halfBand);
             k <= qMin(fftSize / 2, centerBin + halfBand); ++k) {
            double weight = std::cos(M_PI * (k - centerBin) / (2.0 * halfBand));
            energy += magnitude[k] * qMax(0.0, weight);
            count++;
        }

        result[bin][0] = (count > 0) ? energy / count : 0.0;
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_numBins, 1);
    return result;
}

QVector<double> ConstantQTransform::frequencies() const
{
    QVector<double> freqs(m_numBins);
    for (int i = 0; i < m_numBins; ++i)
        freqs[i] = m_minFreq * std::pow(2.0, static_cast<double>(i) / m_binsPerOctave);
    return freqs;
}

int ConstantQTransform::binCount() const { return m_numBins; }

ConstantQTransform::Stats ConstantQTransform::stats() const { return m_stats; }

void ConstantQTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
