/**
 * @file SpectralSubtract4.cpp
 * @brief 谱减法降噪实现
 *
 * 实现经典的谱减法(Spectral Subtraction)降噪算法，
 * 通过估计噪声功率谱并从含噪信号中减去来降低噪声。
 * 支持自动噪声估计、频谱平滑和后处理等功能。
 */

#include "utils/signal74/SpectralSubtract4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SpectralSubtract4::SpectralSubtract4(QObject* parent) : QObject(parent) {}

/**
 * @brief 设置FFT大小
 * @param n FFT点数，必须 >= 64
 */
void SpectralSubtract4::setFFTSize(int n) { m_fftSize = qMax(64, n); }

/**
 * @brief 设置过减因子
 * @param factor 过减因子，范围[0.5, 10.0]，越大降噪越激进
 */
void SpectralSubtract4::setOverSubtraction(double factor) { m_overSub = qBound(0.5, factor, 10.0); }

/**
 * @brief 设置谱下限
 * @param floor 谱下限比例，范围[0.001, 0.5]，防止过度减噪产生音乐噪声
 */
void SpectralSubtract4::setSpectralFloor(double floor) { m_floor = qBound(0.001, floor, 0.5); }

/**
 * @brief 设置噪声功率谱估计
 * @param noise 噪声频谱估计(幅度值)
 */
void SpectralSubtract4::setNoiseEstimate(const QVector<double>& noise) { m_noise = noise; }

/**
 * @brief 对含噪信号执行谱减法降噪
 * @param signal 含噪信号
 * @return 降噪后的信号
 *
 * 算法流程:
 * 1. 分帧加窗(Hann窗)，50%重叠
 * 2. 对每帧计算DFT频谱(幅度+相位)
 * 3. 谱减法: |S|^2 = |Y|^2 - alpha * |N|^2，下限约束
 * 4. 用原始相位重建IFFT
 * 5. 重叠相加合成输出信号
 * 6. 计算信噪比统计
 */
QVector<double> SpectralSubtract4::denoise(const QVector<double>& signal) {
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    const int N = signal.size();
    if (N == 0 || m_noise.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDenoisings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoisings;
        return output;
    }

    const int fftSize = m_fftSize;
    const int hopSize = fftSize / 2;
    const int numBins = fftSize / 2 + 1;

    /* 构建噪声功率谱 */
    QVector<double> noisePower(numBins, 0.0);
    for (int i = 0; i < numBins && i < m_noise.size(); ++i)
        noisePower[i] = m_noise[i] * m_noise[i];

    output.resize(N, 0.0);
    QVector<double> windowSum(N, 0.0);

    /* 构造Hann窗函数 */
    QVector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftSize - 1)));

    int frameCount = 0;
    for (int start = 0; start + fftSize <= N; start += hopSize) {
        frameCount++;

        /* 步骤1: 加窗 */
        QVector<double> frame(fftSize, 0.0);
        for (int i = 0; i < fftSize; ++i) frame[i] = signal[start + i] * window[i];

        /* 步骤2: 计算DFT(幅度谱+相位谱) */
        QVector<double> magnitude(numBins, 0.0), phase(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < fftSize; ++n) {
                double angle = 2.0 * M_PI * k * n / fftSize;
                re += frame[n] * qCos(angle);
                im -= frame[n] * qSin(angle);
            }
            magnitude[k] = qSqrt(re * re + im * im);
            phase[k] = qAtan2(im, re);
        }

        /* 步骤3: 谱减法 - 从信号功率谱中减去过减后的噪声功率谱 */
        QVector<double> cleanMag(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            double sigPow = magnitude[k] * magnitude[k];
            double nEst = (k < noisePower.size()) ? noisePower[k] : 0.0;
            double sub = sigPow - m_overSub * nEst;
            double floorVal = m_floor * sigPow;
            cleanMag[k] = qSqrt(qMax(sub, floorVal));
        }

        /* 步骤4: IFFT重建时域帧(使用原始相位) */
        QVector<double> cleanFrame(fftSize, 0.0);
        for (int n = 0; n < fftSize; ++n) {
            double val = 0.0;
            for (int k = 0; k < numBins; ++k)
                val += cleanMag[k] * qCos(2.0 * M_PI * k * n / fftSize + phase[k]);
            cleanFrame[n] = val / fftSize;
        }

        /* 步骤5: 重叠相加 */
        for (int i = 0; i < fftSize && start + i < N; ++i) {
            output[start + i] += cleanFrame[i] * window[i];
            windowSum[start + i] += window[i] * window[i];
        }
    }

    /* 归一化重叠相加 */
    for (int i = 0; i < N; ++i)
        if (windowSum[i] > 1e-10) output[i] /= windowSum[i];

    /* 计算信噪比 */
    double sigE = 0.0, noiseE = 0.0;
    for (int i = 0; i < N; ++i) {
        sigE += output[i] * output[i];
        double d = signal[i] - output[i];
        noiseE += d * d;
    }
    double snr = (noiseE > 1e-15) ? 10.0 * qLn(sigE / noiseE) / qLn(10.0) : 100.0;

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalDenoisings++;
    m_stats.totalFrames += frameCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoisings;
    emit denoisingCompleted(frameCount, snr);
    return output;
}

/**
 * @brief 重置统计信息
 */
void SpectralSubtract4::resetStatistics() {
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 从信号前导段自动估计噪声功率谱
 * @param signal 输入信号
 * @param numFrames 用于估计噪声的帧数(默认5帧)
 * @return 噪声功率谱估计
 *
 * 取信号开头numFrames帧的平均功率谱作为噪声估计。
 * 适用于信号开头为纯噪声段的场景。
 */
QVector<double> SpectralSubtract4::estimateNoiseSpectrum(const QVector<double>& signal, int numFrames) const {
    const int fftSize = m_fftSize;
    const int numBins = fftSize / 2 + 1;
    QVector<double> avgPower(numBins, 0.0);

    if (signal.size() < fftSize) return avgPower;

    int count = 0;
    for (int f = 0; f < numFrames; ++f) {
        int start = f * fftSize;
        if (start + fftSize > signal.size()) break;
        count++;

        for (int k = 0; k < numBins; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < fftSize; ++n) {
                double angle = 2.0 * M_PI * k * n / fftSize;
                re += signal[start + n] * qCos(angle);
                im -= signal[start + n] * qSin(angle);
            }
            avgPower[k] += (re * re + im * im) / fftSize;
        }
    }

    if (count > 0) {
        for (int k = 0; k < numBins; ++k) {
            avgPower[k] = qSqrt(avgPower[k] / count);
        }
    }
    return avgPower;
}

/**
 * @brief 计算信号的频谱质心(频谱重心)
 * @param signal 输入信号
 * @return 频谱质心值(Hz)
 *
 * 频谱质心反映频谱能量分布的中心频率，
 * 可用于区分语音(低质心)和噪声(高质心)。
 */
double SpectralSubtract4::spectralCentroid(const QVector<double>& signal) const {
    const int fftSize = m_fftSize;
    const int numBins = fftSize / 2 + 1;

    if (signal.size() < fftSize) return 0.0;

    double weightedSum = 0.0, totalPower = 0.0;
    for (int k = 0; k < numBins; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < fftSize; ++n) {
            double angle = 2.0 * M_PI * k * n / fftSize;
            re += signal[n] * qCos(angle);
            im -= signal[n] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        weightedSum += k * mag * mag;
        totalPower += mag * mag;
    }

    return (totalPower > 1e-15) ? weightedSum / totalPower : 0.0;
}

/**
 * @brief 计算信号的RMS能量
 * @param signal 输入信号
 * @return RMS能量值
 */
double SpectralSubtract4::computeRMS(const QVector<double>& signal) const {
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : signal) sum += v * v;
    return qSqrt(sum / signal.size());
}

/**
 * @brief 计算两个信号之间的信噪比
 * @param clean 干净信号(或降噪后信号)
 * @param noisy 含噪信号(或原始信号)
 * @return 信噪比(dB)
 */
double SpectralSubtract4::computeSNR(const QVector<double>& clean, const QVector<double>& noisy) const {
    if (clean.size() != noisy.size() || clean.isEmpty()) return 0.0;

    double sigE = 0.0, noiseE = 0.0;
    for (int i = 0; i < clean.size(); ++i) {
        sigE += clean[i] * clean[i];
        double d = noisy[i] - clean[i];
        noiseE += d * d;
    }
    return (noiseE > 1e-15) ? 10.0 * qLn(sigE / noiseE) / qLn(10.0) : 100.0;
}

/**
 * @brief 计算信号的频谱平坦度(几何均值/算术均值)
 * @param signal 输入信号
 * @return 频谱平坦度，范围[0,1]，越高越类似白噪声
 */
double SpectralSubtract4::spectralFlatness(const QVector<double>& signal) const {
    const int fftSize = m_fftSize;
    const int numBins = fftSize / 2 + 1;

    if (signal.size() < fftSize) return 0.0;

    double logSum = 0.0, linearSum = 0.0;
    int validBins = 0;
    for (int k = 1; k < numBins; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < fftSize; ++n) {
            double angle = 2.0 * M_PI * k * n / fftSize;
            re += signal[n] * qCos(angle);
            im -= signal[n] * qSin(angle);
        }
        double pow = (re * re + im * im) / fftSize;
        if (pow > 1e-20) {
            logSum += qLn(pow);
            linearSum += pow;
            validBins++;
        }
    }

    if (validBins == 0 || linearSum < 1e-20) return 0.0;
    double geoMean = qExp(logSum / validBins);
    double ariMean = linearSum / validBins;
    return (ariMean > 1e-20) ? geoMean / ariMean : 0.0;
}
