/**
 * @file ShortTimeFFT3.cpp
 * @brief 短时FFT3实现 — 重叠保存+相位声码器
 *
 * 实现短时傅里叶变换(STFT)及其逆变换:
 * - 支持多种窗函数(Hann/Hamming/Blackman/Rectangular)
 * - 重叠保存法确保信号完美重建
 * - 相位声码器实现时间拉伸
 *
 * 统计信息跟踪: 变换次数、帧数、FFT大小、平均耗时。
 */

#include "utils/fft44/ShortTimeFFT3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数并构建窗函数
 * @param parent 父对象指针
 */
ShortTimeFFT3::ShortTimeFFT3(QObject* parent)
    : QObject(parent)
{
    buildWindow();
}

/**
 * @brief 设置FFT大小
 * @param size FFT大小(自动向上取整到2的幂)
 */
void ShortTimeFFT3::setFFTSize(int size)
{
    m_fftSize = qMax(64, size);
    // 向上取整到2的幂
    int power = 1;
    while (power < m_fftSize) power <<= 1;
    m_fftSize = power;
    // 默认hop为FFT大小的1/4
    m_hopSize = m_fftSize / 4;
    buildWindow();
}

/**
 * @brief 设置帧移大小
 * @param hop 帧移采样数
 */
void ShortTimeFFT3::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置窗函数类型
 * @param type 窗函数名称: "hann"/"hamming"/"blackman"/"rectangular"
 */
void ShortTimeFFT3::setWindow(const QString& type)
{
    m_windowType = type.toLower();
    buildWindow();
}

/**
 * @brief 构建窗函数
 *
 * 支持四种窗:
 * - Hann: w[n] = 0.5(1 - cos(2*pi*n/(N-1)))
 * - Hamming: w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
 * - Blackman: w[n] = 0.42 - 0.5*cos(...) + 0.08*cos(...)
 * - Rectangular: 全1
 */
void ShortTimeFFT3::buildWindow()
{
    m_window.resize(m_fftSize);
    const int N = m_fftSize;

    for (int n = 0; n < N; ++n) {
        if (m_windowType == "hann") {
            m_window[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
        } else if (m_windowType == "hamming") {
            m_window[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
        } else if (m_windowType == "blackman") {
            m_window[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (N - 1))
                          + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
        } else {
            // rectangular
            m_window[n] = 1.0;
        }
    }
}

/**
 * @brief 对信号帧施加窗函数
 * @param frame 输入帧
 * @return 加窗后的帧
 */
QVector<double> ShortTimeFFT3::applyWindow(const QVector<double>& frame) const
{
    QVector<double> windowed(frame.size());
    for (int i = 0; i < frame.size() && i < m_window.size(); ++i) {
        windowed[i] = frame[i] * m_window[i];
    }
    return windowed;
}

/**
 * @brief 正向STFT变换
 *
 * 将信号分帧、加窗、FFT:
 * 1. 按hopSize步进分帧
 * 2. 对每帧施加窗函数
 * 3. 进行FFT变换
 *
 * @param signal 输入信号
 * @return 每帧的复数频谱(实部序列，虚部交替存储)
 */
QVector<QVector<double>> ShortTimeFFT3::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) {
        return {};
    }

    QVector<QVector<double>> result;
    const int sigLen = signal.size();
    int frameCount = 0;

    for (int start = 0; start + m_fftSize <= sigLen; start += m_hopSize) {
        // 提取帧
        QVector<double> frame(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && (start + i) < sigLen; ++i) {
            frame[i] = signal[start + i];
        }

        // 加窗
        frame = applyWindow(frame);

        // FFT (实部和虚部交替存储: [re0,im0,re1,im1,...])
        QVector<double> realPart(m_fftSize, 0.0);
        QVector<double> imagPart(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize; ++i) {
            realPart[i] = frame[i];
        }

        // 简易DFT
        QVector<double> spectrum(m_fftSize * 2, 0.0);
        for (int k = 0; k < m_fftSize; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < m_fftSize; ++n) {
                double angle = -2.0 * M_PI * k * n / m_fftSize;
                re += realPart[n] * qCos(angle);
                im += realPart[n] * qSin(angle);
            }
            spectrum[k * 2] = re;
            spectrum[k * 2 + 1] = im;
        }

        result.append(spectrum);
        frameCount++;
    }

    // 更新统计信息
    m_stats.totalTransforms++;
    m_stats.totalFrames += frameCount;
    m_stats.fftSize = m_fftSize;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(frameCount, m_fftSize);
    return result;
}

/**
 * @brief 逆STFT变换
 *
 * 从频谱帧重建时域信号:
 * 1. 对每帧进行IFFT
 * 2. 重叠相加(Overlap-Add)
 * 3. 归一化窗函数增益
 *
 * @param frames STFT帧序列
 * @return 重建的时域信号
 */
QVector<double> ShortTimeFFT3::inverse(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    if (frames.isEmpty()) {
        return {};
    }

    const int outputLen = (frames.size() - 1) * m_hopSize + m_fftSize;
    QVector<double> output(outputLen, 0.0);
    QVector<double> windowSum(outputLen, 0.0);

    for (int f = 0; f < frames.size(); ++f) {
        const auto& spectrum = frames[f];
        const int offset = f * m_hopSize;

        // IDFT
        QVector<double> frame(m_fftSize, 0.0);
        for (int n = 0; n < m_fftSize; ++n) {
            double val = 0.0;
            for (int k = 0; k < m_fftSize; ++k) {
                if (k * 2 + 1 < spectrum.size()) {
                    double angle = 2.0 * M_PI * k * n / m_fftSize;
                    val += spectrum[k * 2] * qCos(angle)
                         - spectrum[k * 2 + 1] * qSin(angle);
                }
            }
            frame[n] = val / m_fftSize;
        }

        // 加窗 + 重叠相加
        for (int i = 0; i < m_fftSize && (offset + i) < outputLen; ++i) {
            output[offset + i] += frame[i] * m_window[i];
            windowSum[offset + i] += m_window[i] * m_window[i];
        }
    }

    // 归一化窗函数增益
    for (int i = 0; i < outputLen; ++i) {
        if (windowSum[i] > 1e-10) {
            output[i] /= windowSum[i];
        }
    }

    m_stats.totalTransforms++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return output;
}

/**
 * @brief 相位声码器 — 时间拉伸
 *
 * 通过修改帧间相位增量实现时间拉伸:
 * 1. 正向STFT
 * 2. 调整每帧的相位增量 (scale = hop_analysis / hop_synthesis)
 * 3. 逆STFT
 *
 * @param signal 输入信号
 * @param stretchFactor 拉伸因子 (>1 变慢, <1 变快)
 * @return 拉伸后的信号
 */
QVector<double> ShortTimeFFT3::phaseVocoder(const QVector<double>& signal,
                                             double stretchFactor)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || stretchFactor <= 0.0) {
        return signal;
    }

    stretchFactor = qBound(0.25, stretchFactor, 4.0);

    // 分析帧序列
    QVector<QVector<double>> frames = forward(signal);
    if (frames.isEmpty()) {
        return signal;
    }

    const int numBins = m_fftSize / 2 + 1;
    const int synthHop = qMax(1, static_cast<int>(m_hopSize / stretchFactor));
    const int newNumFrames = static_cast<int>(
        frames.size() * stretchFactor);

    // 相位累加器
    QVector<double> phaseAccum(numBins, 0.0);
    // 初始化第一帧的相位
    for (int k = 0; k < numBins; ++k) {
        if (k * 2 + 1 < frames[0].size()) {
            phaseAccum[k] = qAtan2(frames[0][k * 2 + 1], frames[0][k * 2]);
        }
    }

    // 构建合成帧
    QVector<QVector<double>> synthFrames;
    synthFrames.reserve(newNumFrames);

    // 第一帧直接使用
    synthFrames.append(frames[0]);

    double frameIndex = 0.0;
    for (int i = 1; i < newNumFrames; ++i) {
        frameIndex += 1.0 / stretchFactor;
        int srcFrame = qMin(static_cast<int>(frameIndex),
                            frames.size() - 1);
        int prevFrame = qMax(0, srcFrame - 1);

        QVector<double> newFrame(m_fftSize * 2, 0.0);

        for (int k = 0; k < numBins; ++k) {
            // 当前帧和前一帧的相位
            double prevPhase = (prevFrame < frames.size() && k * 2 + 1 < frames[prevFrame].size())
                ? qAtan2(frames[prevFrame][k * 2 + 1], frames[prevFrame][k * 2]) : 0.0;
            double currPhase = (srcFrame < frames.size() && k * 2 + 1 < frames[srcFrame].size())
                ? qAtan2(frames[srcFrame][k * 2 + 1], frames[srcFrame][k * 2]) : 0.0;

            // 相位差
            double deltaPhase = currPhase - prevPhase;
            // 期望相位增量
            double expectedPhase = 2.0 * M_PI * k * m_hopSize / m_fftSize;
            // 偏差
            double deviation = deltaPhase - expectedPhase;
            // 归一化到 [-pi, pi]
            while (deviation > M_PI) deviation -= 2.0 * M_PI;
            while (deviation < -M_PI) deviation += 2.0 * M_PI;
            // 真实频率
            double trueFreq = expectedPhase + deviation;

            // 累加相位
            phaseAccum[k] += trueFreq;

            // 幅度取源帧的幅度
            double mag = 0.0;
            if (srcFrame < frames.size() && k * 2 + 1 < frames[srcFrame].size()) {
                double re = frames[srcFrame][k * 2];
                double im = frames[srcFrame][k * 2 + 1];
                mag = qSqrt(re * re + im * im);
            }

            newFrame[k * 2] = mag * qCos(phaseAccum[k]);
            newFrame[k * 2 + 1] = mag * qSin(phaseAccum[k]);
        }

        // 对称填充 (共轭对称)
        for (int k = numBins; k < m_fftSize; ++k) {
            int mirror = m_fftSize - k;
            if (mirror * 2 + 1 < newFrame.size()) {
                newFrame[k * 2] = newFrame[mirror * 2];
                newFrame[k * 2 + 1] = -newFrame[mirror * 2 + 1];
            }
        }

        synthFrames.append(newFrame);
    }

    // 使用合成hop进行逆变换
    int savedHop = m_hopSize;
    m_hopSize = synthHop;
    QVector<double> output = inverse(synthFrames);
    m_hopSize = savedHop;

    m_stats.totalTransforms++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return output;
}

/**
 * @brief 重置所有统计信息
 */
void ShortTimeFFT3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.fftSize = m_fftSize;
    m_timeSum = 0.0;
}
