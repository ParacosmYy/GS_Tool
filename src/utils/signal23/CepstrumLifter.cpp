/**
 * @file CepstrumLifter.cpp
 * @brief 倒谱提升器实现 — 实倒谱/复倒谱/低通高通提升/MFCC/倒谱距离
 */

#include "utils/signal23/CepstrumLifter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CepstrumLifter::CepstrumLifter(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_timeSum(0.0)
{
}

/** @brief 计算实倒谱 @param signal 输入信号 @return 倒谱序列 */
QVector<double> CepstrumLifter::computeRealCepstrum(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) return {};

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int i = 0; i < n; ++i) re[i] = signal[i];

    /* FFT */
    fft(re, im);

    /* 取对数幅度: log|X(f)| */
    for (int i = 0; i < fftSize; ++i) {
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        re[i] = qLn(qMax(mag, 1e-30));
        im[i] = 0.0;
    }

    /* IFFT得到实倒谱 */
    ifft(re, im);

    QVector<double> cepstrum(n);
    for (int i = 0; i < n; ++i) {
        cepstrum[i] = re[i];
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalLiftings;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLiftings);

    emit cepstrumComputed(n);
    return cepstrum;
}

/** @brief 计算复倒谱 @param signal 输入信号 @return (实部, 虚部) */
QPair<QVector<double>, QVector<double>> CepstrumLifter::computeComplexCepstrum(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int i = 0; i < n; ++i) re[i] = signal[i];

    fft(re, im);

    /* 取复对数: ln(X) = ln|X| + j*arg(X) */
    for (int i = 0; i < fftSize; ++i) {
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        double phase = qAtan2(im[i], re[i]);
        re[i] = qLn(qMax(mag, 1e-30));
        im[i] = phase;
    }

    /* 相位展开(简化: 相邻相位差限制在[-pi,pi]) */
    for (int i = 1; i < fftSize; ++i) {
        double diff = im[i] - im[i - 1];
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        im[i] = im[i - 1] + diff;
    }

    ifft(re, im);

    QVector<double> cepRe(n), cepIm(n);
    for (int i = 0; i < n; ++i) {
        cepRe[i] = re[i];
        cepIm[i] = im[i];
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalLiftings;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLiftings);

    return {cepRe, cepIm};
}

/** @brief 低通倒滤波(频谱包络) @param cepstrum 倒谱 @param cutoff 截止quefrency @return 提升后倒谱 */
QVector<double> CepstrumLifter::lowPassLifter(
    const QVector<double>& cepstrum, int cutoff)
{
    QElapsedTimer timer;
    timer.start();

    int n = cepstrum.size();
    QVector<double> lifted(n, 0.0);

    /* 保留低倒频率部分(0到cutoff) */
    for (int i = 0; i < qMin(cutoff + 1, n); ++i) {
        lifted[i] = cepstrum[i];
    }
    /* 对称部分 */
    for (int i = qMax(0, n - cutoff); i < n; ++i) {
        lifted[i] = cepstrum[i];
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLiftings);

    emit lifteringComplete(static_cast<int>(LifterType::LowPass), cutoff);
    return lifted;
}

/** @brief 高通倒滤波(激励源) @param cepstrum 倒谱 @param cutoff 截止quefrency @return 提升后倒谱 */
QVector<double> CepstrumLifter::highPassLifter(
    const QVector<double>& cepstrum, int cutoff)
{
    QElapsedTimer timer;
    timer.start();

    int n = cepstrum.size();
    QVector<double> lifted(n, 0.0);

    /* 保留高倒频率部分(超过cutoff) */
    for (int i = cutoff + 1; i < n - cutoff - 1; ++i) {
        lifted[i] = cepstrum[i];
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLiftings);

    emit lifteringComplete(static_cast<int>(LifterType::HighPass), cutoff);
    return lifted;
}

/** @brief 带通倒滤波 @param cepstrum 倒谱 @param lowCut 低截止 @param highCut 高截止 @return 提升后倒谱 */
QVector<double> CepstrumLifter::bandPassLifter(
    const QVector<double>& cepstrum, int lowCut, int highCut)
{
    int n = cepstrum.size();
    QVector<double> lifted(n, 0.0);

    for (int i = lowCut; i <= qMin(highCut, n - 1); ++i) {
        lifted[i] = cepstrum[i];
    }
    for (int i = qMax(0, n - highCut - 1); i <= qMin(n - 1, n - lowCut - 1); ++i) {
        lifted[i] = cepstrum[i];
    }

    emit lifteringComplete(static_cast<int>(LifterType::BandPass), highCut);
    return lifted;
}

/** @brief 倒谱距离 @param c1 倒谱1 @param c2 倒谱2 @return 距离值 */
double CepstrumLifter::cepstralDistance(const QVector<double>& c1,
                                        const QVector<double>& c2) const
{
    int n = qMin(c1.size(), c2.size());
    if (n == 0) return 0.0;

    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = c1[i] - c2[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/** @brief 计算Mel倒谱系数(MFCC) @param signal 输入信号 @param numCoeffs 系数数 @return MFCC */
QVector<double> CepstrumLifter::melCepstrum(const QVector<double>& signal,
                                             int numCoeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) return {};

    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    /* 加窗(Hanning) */
    QVector<double> windowed(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        windowed[i] = signal[i] * w;
    }

    /* FFT */
    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int i = 0; i < fftSize; ++i) re[i] = windowed[i];
    fft(re, im);

    /* 功率谱 */
    int halfN = fftSize / 2;
    QVector<double> powerSpec(halfN);
    for (int i = 0; i < halfN; ++i) {
        powerSpec[i] = (re[i] * re[i] + im[i] * im[i]) / fftSize;
    }

    /* Mel滤波器组 */
    int numFilters = 26;
    QVector<double> melEnergies(numFilters, 0.0);

    double maxFreq = m_sampleRate / 2.0;
    double maxMel = hzToMel(maxFreq);
    for (int f = 0; f < numFilters; ++f) {
        double melLow = maxMel * f / (numFilters + 1);
        double melCenter = maxMel * (f + 1) / (numFilters + 1);
        double melHigh = maxMel * (f + 2) / (numFilters + 1);

        int binLow = static_cast<int>(melToHz(melLow) / maxFreq * halfN);
        int binCenter = static_cast<int>(melToHz(melCenter) / maxFreq * halfN);
        int binHigh = static_cast<int>(melToHz(melHigh) / maxFreq * halfN);

        binLow = qBound(0, binLow, halfN - 1);
        binCenter = qBound(0, binCenter, halfN - 1);
        binHigh = qBound(0, binHigh, halfN - 1);

        for (int b = binLow; b <= binHigh; ++b) {
            double weight = 0.0;
            if (b <= binCenter && binCenter > binLow) {
                weight = static_cast<double>(b - binLow) / (binCenter - binLow);
            } else if (b > binCenter && binHigh > binCenter) {
                weight = static_cast<double>(binHigh - b) / (binHigh - binCenter);
            }
            melEnergies[f] += weight * powerSpec[b];
        }
        melEnergies[f] = qLn(qMax(melEnergies[f], 1e-30));
    }

    /* DCT得到MFCC */
    QVector<double> mfcc(numCoeffs);
    for (int k = 0; k < numCoeffs; ++k) {
        double sum = 0.0;
        for (int f = 0; f < numFilters; ++f) {
            sum += melEnergies[f] * qCos(M_PI * k * (f + 0.5) / numFilters);
        }
        mfcc[k] = sum;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalLiftings;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLiftings);

    emit cepstrumComputed(numCoeffs);
    return mfcc;
}

/** @brief 从倒谱重建信号 @param cepstrum 倒谱 @return 重建信号 */
QVector<double> CepstrumLifter::reconstructSignal(const QVector<double>& cepstrum)
{
    int n = cepstrum.size();
    if (n == 0) return {};

    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int i = 0; i < n; ++i) re[i] = cepstrum[i];

    /* 指数化: 从对数域回到频域 */
    fft(re, im);
    for (int i = 0; i < fftSize; ++i) {
        double expRe = qExp(re[i]);
        re[i] = expRe * qCos(im[i]);
        im[i] = expRe * qSin(im[i]);
    }

    ifft(re, im);

    QVector<double> signal(n);
    for (int i = 0; i < n; ++i) {
        signal[i] = re[i];
    }
    return signal;
}

/** @brief 基2 FFT(原地) @param re 实部 @param im 虚部 */
void CepstrumLifter::fft(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    im.fill(0.0);

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
        }
    }

    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tRe = cRe * re[o] - cIm * im[o];
                double tIm = cRe * im[o] + cIm * re[o];
                re[o] = re[e] - tRe; im[o] = im[e] - tIm;
                re[e] += tRe; im[e] += tIm;
                double nr = cRe * wRe - cIm * wIm;
                cIm = cRe * wIm + cIm * wRe; cRe = nr;
            }
        }
    }
}

/** @brief 基2 IFFT(原地) @param re 实部 @param im 虚部 */
void CepstrumLifter::ifft(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    fft(re, im);
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] = -im[i] / n;
    }
}

/** @brief Hz转Mel @param hz 频率(Hz) @return Mel值 */
double CepstrumLifter::hzToMel(double hz) const
{
    return 2595.0 * qLog10(1.0 + hz / 700.0);
}

/** @brief Mel转Hz @param mel Mel值 @return 频率(Hz) */
double CepstrumLifter::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

void CepstrumLifter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
