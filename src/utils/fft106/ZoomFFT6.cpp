#include "ZoomFFT6.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化ZoomFFT细化分析器
 * @param parent 父对象指针
 */
ZoomFFT6::ZoomFFT6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ZoomFFT6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置采样率
 * @param sampleRateHz 采样率(Hz)
 */
void ZoomFFT6::setSampleRate(double sampleRateHz)
{
    m_sampleRate = qMax(1.0, sampleRateHz);
}

/**
 * @brief 执行ZoomFFT细化频谱分析
 *
 * 通过频移(将目标频段搬移到基带)、低通滤波和降采样三步实现
 * 对指定频段的高分辨率分析，等效于超长FFT但计算量大幅减少。
 *
 * @param samples 输入时域采样数据
 * @param centerFreqHz 中心频率(Hz)
 * @param spanHz 分析带宽(Hz)
 * @return 细化后的频谱幅度
 */
QVector<double> ZoomFFT6::analyze(const QVector<double>& samples, double centerFreqHz, double spanHz)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    if (n == 0) {
        emit zoomAnalysisCompleted(0);
        return {};
    }

    /* 步骤1：频移 - 将中心频率搬移到基带 */
    QVector<double> shifted(n);
    double phaseStep = -2.0 * M_PI * centerFreqHz / m_sampleRate;
    double phase = 0.0;
    for (int i = 0; i < n; ++i) {
        double cosVal = qCos(phase);
        double sinVal = qSin(phase);
        shifted[i] = samples[i] * cosVal; /* 实数简化处理 */
        phase += phaseStep;
    }

    /* 步骤2：低通滤波 - 保留目标带宽 */
    double normalizedCutoff = spanHz / (2.0 * m_sampleRate);
    int filterLen = qMin(63, n / 4); /* FIR滤波器长度 */
    QVector<double> filter(filterLen);
    double filterSum = 0.0;
    for (int i = 0; i < filterLen; ++i) {
        double t = i - (filterLen - 1) / 2.0;
        if (qAbs(t) < 1e-10) {
            filter[i] = 2.0 * normalizedCutoff;
        } else {
            filter[i] = qSin(2.0 * M_PI * normalizedCutoff * t) / (M_PI * t);
        }
        /* 汉宁窗 */
        filter[i] *= 0.5 * (1.0 - qCos(2.0 * M_PI * i / (filterLen - 1)));
        filterSum += filter[i];
    }

    /* 归一化滤波器 */
    for (int i = 0; i < filterLen; ++i) {
        filter[i] /= filterSum;
    }

    /* 卷积滤波 */
    QVector<double> filtered(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < filterLen && (i - j) >= 0; ++j) {
            filtered[i] += shifted[i - j] * filter[j];
        }
    }

    /* 步骤3：降采样 */
    int decimation = qMax(1, n / (n / m_zoomFactor));
    int outputLen = n / decimation;
    QVector<double> decimated(outputLen);
    for (int i = 0; i < outputLen; ++i) {
        decimated[i] = filtered[i * decimation];
    }

    /* 步骤4：对降采样数据执行DFT获取幅度谱 */
    int binCount = decimated.size();
    QVector<double> magnitude(binCount / 2 + 1);
    m_freqAxis.resize(binCount / 2 + 1);
    double freqRes = (spanHz / decimation) / binCount;

    for (int k = 0; k <= binCount / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < binCount; ++i) {
            double angle = 2.0 * M_PI * k * i / binCount;
            re += decimated[i] * qCos(angle);
            im -= decimated[i] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im) / binCount;
        m_freqAxis[k] = centerFreqHz - spanHz / 2.0 + k * freqRes;
    }

    m_stats.maxFrequencyResolution = freqRes;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalZoomAnalyses++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalZoomAnalyses;

    emit zoomAnalysisCompleted(magnitude.size());
    return magnitude;
}

/**
 * @brief 获取当前频率分辨率
 * @return 频率分辨率(Hz)
 */
double ZoomFFT6::frequencyResolution() const
{
    return m_stats.maxFrequencyResolution;
}
