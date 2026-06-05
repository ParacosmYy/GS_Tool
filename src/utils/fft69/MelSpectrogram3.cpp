/**
 * @file MelSpectrogram3.cpp
 * @brief Mel频谱图计算器实现
 *
 * 实现从时域信号到Mel频谱图的转换，包括FFT变换、Mel滤波器组构建、
 * 功率谱计算。常用于语音识别和音频分析。
 */

#include "utils/fft69/MelSpectrogram3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数并构建Mel滤波器组
 * @param parent 父对象指针
 */
MelSpectrogram3::MelSpectrogram3(QObject* parent)
    : QObject(parent)
{
    buildMelFilterBank();
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void MelSpectrogram3::setSampleRate(double sr)
{
    m_sr = qBound(8000.0, sr, 192000.0);
    m_fmax = qMin(m_fmax, m_sr / 2.0);
    buildMelFilterBank();
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数，必须是2的幂
 */
void MelSpectrogram3::setFFTSize(int n)
{
    m_fftSize = qMax(64, n);
    buildMelFilterBank();
}

/**
 * @brief 设置Mel频带数量
 * @param bins Mel频带数
 */
void MelSpectrogram3::setNumMelBins(int bins)
{
    m_numBins = qBound(8, bins, 256);
    buildMelFilterBank();
}

/**
 * @brief 设置频率范围
 * @param fmin 最低频率(Hz)
 * @param fmax 最高频率(Hz)
 */
void MelSpectrogram3::setFreqRange(double fmin, double fmax)
{
    m_fmin = qBound(0.0, fmin, m_sr / 2.0);
    m_fmax = qBound(m_fmin + 1.0, fmax, m_sr / 2.0);
    buildMelFilterBank();
}

/**
 * @brief 计算信号的Mel频谱图
 * @param signal 输入时域信号
 * @return Mel频谱图，每行一个时间帧，每列一个Mel频带
 */
QVector<QVector<double>> MelSpectrogram3::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int hopSize = m_fftSize / 4;
    m_numFrames = qMax(1, (signal.size() - m_fftSize) / hopSize + 1);

    QVector<QVector<double>> melSpec(m_numFrames, QVector<double>(m_numBins, 0.0));

    // 预计算Hann窗
    QVector<double> window(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
    }

    for (int frame = 0; frame < m_numFrames; ++frame) {
        int start = frame * hopSize;

        // 提取帧并加窗
        QVector<double> frameData(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && (start + i) < signal.size(); ++i) {
            frameData[i] = signal[start + i] * window[i];
        }

        // 计算功率谱（简化DFT）
        int numBins = m_fftSize / 2 + 1;
        QVector<double> powerSpec(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < m_fftSize; ++n) {
                double angle = 2.0 * M_PI * k * n / m_fftSize;
                re += frameData[n] * qCos(angle);
                im -= frameData[n] * qSin(angle);
            }
            powerSpec[k] = (re * re + im * im) / m_fftSize;
        }

        // 应用Mel滤波器组
        if (m_filterBank.size() == m_numBins) {
            for (int m = 0; m < m_numBins; ++m) {
                if (m_filterBank[m].size() == numBins) {
                    double sum = 0.0;
                    for (int k = 0; k < numBins; ++k) {
                        sum += m_filterBank[m][k] * powerSpec[k];
                    }
                    melSpec[frame][m] = qMax(sum, 1e-10);
                }
            }
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalFrames += m_numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_numFrames, m_numBins);
    return melSpec;
}

/**
 * @brief 重置统计信息
 */
void MelSpectrogram3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 构建Mel三角滤波器组
 *
 * 在Mel刻度上均匀分布三角滤波器，然后转换回Hz刻度。
 * 每个滤波器是一个三角窗，中心频率对应一个Mel频带。
 */
void MelSpectrogram3::buildMelFilterBank()
{
    m_filterBank.clear();
    m_filterBank.resize(m_numBins);

    double melMin = hzToMel(m_fmin);
    double melMax = hzToMel(m_fmax);
    int numPoints = m_numBins + 2; // 三角滤波器需要额外的两端

    // 在Mel空间均匀分布中心频率
    QVector<double> melCenters(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        melCenters[i] = melMin + (melMax - melMin) * i / (numPoints - 1);
    }

    // 转换回Hz
    QVector<double> hzCenters(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        hzCenters[i] = melToHz(melCenters[i]);
    }

    // FFT频率对应的bin
    int numFFTBins = m_fftSize / 2 + 1;
    double fftFreqStep = m_sr / m_fftSize;

    for (int m = 0; m < m_numBins; ++m) {
        m_filterBank[m].resize(numFFTBins, 0.0);
        double fLeft = hzCenters[m];
        double fCenter = hzCenters[m + 1];
        double fRight = hzCenters[m + 2];

        for (int k = 0; k < numFFTBins; ++k) {
            double freq = k * fftFreqStep;
            if (freq >= fLeft && freq <= fCenter) {
                m_filterBank[m][k] = (freq - fLeft) / qMax(fCenter - fLeft, 1e-10);
            } else if (freq > fCenter && freq <= fRight) {
                m_filterBank[m][k] = (fRight - freq) / qMax(fRight - fCenter, 1e-10);
            }
        }
    }
}

/**
 * @brief Hz转Mel刻度
 * @param hz 频率(Hz)
 * @return Mel值
 */
double MelSpectrogram3::hzToMel(double hz) const
{
    return 2595.0 * qLog10(1.0 + hz / 700.0);
}

/**
 * @brief Mel刻度转Hz
 * @param mel Mel值
 * @return 频率(Hz)
 */
double MelSpectrogram3::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}
