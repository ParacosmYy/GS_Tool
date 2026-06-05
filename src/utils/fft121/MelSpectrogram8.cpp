#include "MelSpectrogram8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Mel频谱图v8引擎
 * @param parent 父对象指针
 */
MelSpectrogram8::MelSpectrogram8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MelSpectrogram8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 频率值转Mel刻度
 * @param frequencyHz 频率(Hz)
 * @return Mel值
 */
double MelSpectrogram8::hzToMel(double frequencyHz) const
{
    return 2595.0 * qLn(1.0 + frequencyHz / 700.0) / qLn(10.0);
}

/**
 * @brief Mel刻度转频率值
 * @param mel Mel值
 * @return 频率(Hz)
 */
double MelSpectrogram8::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 设置Mel滤波器组参数
 *
 * 根据参数构建三角滤波器组，在Mel刻度上均匀分布中心频率。
 *
 * @param numMelBands Mel频带数量
 * @param fftSize FFT大小
 * @param sampleRate 采样率(Hz)
 * @param lowFreqHz 最低频率(Hz)
 * @param highFreqHz 最高频率(Hz)
 */
void MelSpectrogram8::setFilterBank(int numMelBands, int fftSize, double sampleRate,
                                     double lowFreqHz, double highFreqHz)
{
    m_numMelBands = qMax(1, numMelBands);
    m_fftSize = qMax(4, fftSize);
    m_sampleRate = qMax(1.0, sampleRate);
    m_lowFreq = qMax(0.0, lowFreqHz);
    m_highFreq = (highFreqHz <= 0.0) ? m_sampleRate / 2.0 : highFreqHz;
    m_filterBankReady = false;

    /* 构建滤波器组 */
    int numFftBins = m_fftSize / 2 + 1;
    double melMin = hzToMel(m_lowFreq);
    double melMax = hzToMel(m_highFreq);

    QVector<double> melPoints(m_numMelBands + 2);
    for (int i = 0; i < m_numMelBands + 2; ++i)
        melPoints[i] = melMin + (melMax - melMin) * i / (m_numMelBands + 1);

    QVector<int> binPoints(m_numMelBands + 2);
    for (int i = 0; i < m_numMelBands + 2; ++i)
        binPoints[i] = static_cast<int>(qRound(melToHz(melPoints[i]) / m_sampleRate * m_fftSize));

    m_filterBank.clear();
    m_filterBank.resize(m_numMelBands, QVector<double>(numFftBins, 0.0));

    for (int m = 0; m < m_numMelBands; ++m) {
        int left = qMax(0, binPoints[m]);
        int center = qBound(0, binPoints[m + 1], numFftBins - 1);
        int right = qMin(binPoints[m + 2], numFftBins - 1);
        if (center <= left || right <= center) continue;

        for (int k = left; k <= right; ++k) {
            if (k <= center && center > left)
                m_filterBank[m][k] = static_cast<double>(k - left) / (center - left);
            else if (k > center && right > center)
                m_filterBank[m][k] = static_cast<double>(right - k) / (right - center);
        }
    }
    m_filterBankReady = true;
}

/**
 * @brief 从功率谱计算Mel频谱
 *
 * 将线性功率谱通过Mel滤波器组映射到Mel刻度。
 * 如果滤波器组未构建，自动使用默认参数创建。
 *
 * @param powerSpectrum 输入功率谱
 * @return Mel频带能量序列
 */
QVector<double> MelSpectrogram8::compute(const QVector<double>& powerSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (powerSpectrum.isEmpty()) {
        emit transformCompleted(0);
        return result;
    }

    /* 自动构建滤波器组 */
    if (!m_filterBankReady) {
        int numFftBins = powerSpectrum.size();
        int fftSize = (numFftBins - 1) * 2;
        setFilterBank(m_numMelBands, fftSize, m_sampleRate, m_lowFreq, m_highFreq);
    }

    int numFftBins = qMin(powerSpectrum.size(), (m_filterBank.isEmpty()) ? 0 : m_filterBank[0].size());

    result.resize(m_numMelBands);
    for (int m = 0; m < m_numMelBands; ++m) {
        double sum = 0.0;
        for (int i = 0; i < numFftBins; ++i)
            sum += powerSpectrum[i] * m_filterBank[m][i];
        result[m] = qLn(sum + 1e-10);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(1);
    return result;
}

/**
 * @brief 批量计算Mel频谱图
 *
 * 对多帧功率谱依次应用Mel滤波器组，输出完整的Mel频谱图矩阵。
 *
 * @param frames 分帧后的功率谱序列
 * @return Mel频谱图矩阵 (帧数 × Mel频带数)
 */
QVector<QVector<double>> MelSpectrogram8::computeSequence(
    const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;
    if (frames.isEmpty()) {
        emit transformCompleted(0);
        return result;
    }

    /* 自动构建滤波器组 */
    if (!m_filterBankReady && !frames.isEmpty()) {
        int numFftBins = frames[0].size();
        int fftSize = (numFftBins - 1) * 2;
        setFilterBank(m_numMelBands, fftSize, m_sampleRate, m_lowFreq, m_highFreq);
    }

    result.reserve(frames.size());
    for (const auto& frame : frames) {
        int numFftBins = qMin(frame.size(), (m_filterBank.isEmpty()) ? 0 : m_filterBank[0].size());

        QVector<double> melFrame(m_numMelBands, 0.0);
        for (int m = 0; m < m_numMelBands; ++m) {
            double sum = 0.0;
            for (int i = 0; i < numFftBins; ++i)
                sum += frame[i] * m_filterBank[m][i];
            melFrame[m] = qLn(sum + 1e-10);
        }
        result.append(melFrame);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(result.size());
    return result;
}
