/**
 * @file Cepstrum.cpp
 * @brief 倒谱分析实现 — FFT/IFFT + 基频检测 + 同态滤波
 */

#include "utils/signal14/Cepstrum.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
Cepstrum::Cepstrum(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
{
}

/** @brief 设置采样率 @param rate 采样率 (Hz) */
void Cepstrum::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算倒谱
 * @param data 时域输入数据
 * @param type 倒谱类型
 * @return 倒谱序列
 *
 * 实倒谱: c[n] = IDFT{log|DFT{x[n]}|}
 * 复倒谱: c[n] = IDFT{log(DFT{x[n]})}
 */
QVector<double> Cepstrum::compute(const QVector<double>& data,
                                  CepstrumType type)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 4) return {};

    /* 补零到 2 的幂 */
    int fftSize = nextPowerOf2(n);

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = data[i];
    }

    /* 正变换 */
    fft(real, imag);

    if (type == CepstrumType::Real) {
        /* 实倒谱: 取幅度对数 */
        for (int i = 0; i < fftSize; ++i) {
            double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
            real[i] = qLn(qMax(mag, 1e-30));
            imag[i] = 0.0;
        }
    } else {
        /* 复倒谱: 取复对数 log(z) = log|z| + j*arg(z) */
        for (int i = 0; i < fftSize; ++i) {
            double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
            double phase = qAtan2(imag[i], real[i]);
            real[i] = qLn(qMax(mag, 1e-30));
            imag[i] = phase;
        }
    }

    /* 逆变换得到倒谱 */
    ifft(real, imag);

    QVector<double> cepstrum(n);
    for (int i = 0; i < n; ++i) {
        cepstrum[i] = real[i];
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit cepstrumReady(n, type);
    return cepstrum;
}

/**
 * @brief 从倒谱检测基频
 * @param cepstrum 倒谱序列
 * @param minFreqHz 搜索下限频率
 * @param maxFreqHz 搜索上限频率
 * @return 基频检测结果
 *
 * 在倒谱的 "倒频率" 轴上搜索基音周期对应的峰值。
 * 倒频率 = 采样率 / 基频(Hz)。
 */
Cepstrum::PitchResult Cepstrum::detectPitch(const QVector<double>& cepstrum,
                                            double minFreqHz,
                                            double maxFreqHz) const
{
    PitchResult result;

    if (cepstrum.size() < 4 || m_sampleRate <= 0) {
        return result;
    }

    /* 计算倒频率搜索范围 (采样点数) */
    int minQuef = qMax(1, static_cast<int>(m_sampleRate / maxFreqHz));
    int maxQuef = qMin(cepstrum.size() - 1,
                       static_cast<int>(m_sampleRate / minFreqHz));

    if (minQuef >= maxQuef) {
        return result;
    }

    /* 在搜索范围内找最大峰值 */
    double peakValue = -1e30;
    int peakIndex = minQuef;

    for (int i = minQuef; i <= maxQuef; ++i) {
        if (cepstrum[i] > peakValue) {
            peakValue = cepstrum[i];
            peakIndex = i;
        }
    }

    /* 计算置信度: 峰值相对于周围均值的突出程度 */
    double meanVal = 0.0;
    int count = 0;
    for (int i = minQuef; i <= maxQuef; ++i) {
        meanVal += cepstrum[i];
        ++count;
    }
    if (count > 0) meanVal /= count;

    double stdDev = 0.0;
    for (int i = minQuef; i <= maxQuef; ++i) {
        double diff = cepstrum[i] - meanVal;
        stdDev += diff * diff;
    }
    if (count > 1) stdDev = qSqrt(stdDev / (count - 1));

    result.period = static_cast<double>(peakIndex);
    result.frequency = m_sampleRate / static_cast<double>(peakIndex);
    result.confidence = (stdDev > 0) ? qMin(1.0, (peakValue - meanVal) / (3.0 * stdDev)) : 0.0;
    result.valid = (result.confidence > 0.3 && result.frequency >= minFreqHz && result.frequency <= maxFreqHz);

    return result;
}

/**
 * @brief 低频倒谱 liftering (提取声道响应)
 * @param cepstrum 倒谱序列
 * @param cutoffQuefrency 截断倒频率
 * @return liftering 后的倒谱
 */
QVector<double> Cepstrum::lowpassLifter(const QVector<double>& cepstrum,
                                        int cutoffQuefrency) const
{
    int n = cepstrum.size();
    int cutoff = qBound(1, cutoffQuefrency, n);
    QVector<double> filtered(n, 0.0);

    for (int i = 0; i < cutoff && i < n; ++i) {
        filtered[i] = cepstrum[i];
    }
    /* 对称部分 (倒谱是实信号的 IDFT，具有对称性) */
    for (int i = n - cutoff + 1; i < n; ++i) {
        filtered[i] = cepstrum[i];
    }

    return filtered;
}

/**
 * @brief 倒谱平滑
 * @param data 时域输入数据
 * @param keepCoefficients 保留的倒谱系数个数
 * @return 平滑后的时域数据
 */
QVector<double> Cepstrum::smooth(const QVector<double>& data,
                                 int keepCoefficients)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 4) return data;

    /* 计算倒谱 */
    QVector<double> cep = compute(data, CepstrumType::Real);

    /* 截断高频倒谱系数 */
    int keep = qBound(1, keepCoefficients, n);
    QVector<double> truncated(n, 0.0);
    for (int i = 0; i < keep; ++i) {
        truncated[i] = cep[i];
    }
    for (int i = n - keep + 1; i < n; ++i) {
        truncated[i] = cep[i];
    }

    /* 反变换回时域: FFT -> exp -> IFFT */
    int fftSize = nextPowerOf2(n);
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = truncated[i];
    }

    fft(real, imag);
    /* exp 恢复频谱 */
    for (int i = 0; i < fftSize; ++i) {
        double mag = qExp(real[i]);
        double phase = imag[i];
        real[i] = mag * qCos(phase);
        imag[i] = mag * qSin(phase);
    }
    ifft(real, imag);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = real[i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return result;
}

/** @brief 重置统计信息 */
void Cepstrum::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 基 2 FFT
 * @param real 实部
 * @param imag 虚部
 */
void Cepstrum::fft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    if (n <= 1) return;
    imag.resize(n);
    imag.fill(0.0);

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }

    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tRe = curRe * real[o] - curIm * imag[o];
                double tIm = curRe * imag[o] + curIm * real[o];
                real[o] = real[e] - tRe; imag[o] = imag[e] - tIm;
                real[e] += tRe; imag[e] += tIm;
                double nRe = curRe * wRe - curIm * wIm;
                double nIm = curRe * wIm + curIm * wRe;
                curRe = nRe; curIm = nIm;
            }
        }
    }
}

/**
 * @brief 基 2 IFFT
 * @param real 实部
 * @param imag 虚部
 */
void Cepstrum::ifft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    /* 共轭 */
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
}

/** @brief 计算大于等于 n 的最小 2 的幂 */
int Cepstrum::nextPowerOf2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}
