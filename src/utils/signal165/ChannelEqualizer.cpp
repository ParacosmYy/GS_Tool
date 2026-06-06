/**
 * @file ChannelEqualizer.cpp
 * @brief ChannelEqualizer 实现
 *
 * 实现频域信道均衡：信道响应估计、ZF/MMSE逆滤波器构造、
 * overlap-save频域均衡和SNR计算。
 */

#include "utils/signal165/ChannelEqualizer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ChannelEqualizer::ChannelEqualizer(QObject* parent)
    : QObject(parent)
{
}

ChannelEqualizer::~ChannelEqualizer() = default;

void ChannelEqualizer::setEqualizerType(EqualizerType type)
{
    m_type = type;
    m_filterReady = false;
}

void ChannelEqualizer::setNoiseVariance(double variance)
{
    m_noiseVariance = qMax(1e-12, variance);
    m_filterReady = false;
}

void ChannelEqualizer::setFFTSize(int size)
{
    int p = 1;
    while (p < size) p <<= 1;
    m_fftSize = qMax(16, p);
    m_filterReady = false;
}

/**
 * @brief Cooley-Tukey FFT
 */
void ChannelEqualizer::fft(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    const int n = real.size();
    if (n <= 1) return;

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wR = qCos(angle);
        double wI = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nR = cR * wR - cI * wI;
                double nI = cR * wI + cI * wR;
                cR = nR; cI = nI;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

/**
 * @brief 构造频域均衡滤波器
 *
 * ZF:  H_eq(f) = 1 / H(f)
 * MMSE: H_eq(f) = conj(H(f)) / (|H(f)|^2 + noise_var/signal_var)
 */
void ChannelEqualizer::buildEqualizerFilter()
{
    if (m_channelIR.isEmpty()) return;

    /* Zero-pad channel IR to FFT size */
    QVector<double> hReal(m_fftSize, 0.0);
    QVector<double> hImag(m_fftSize, 0.0);

    int copyLen = qMin(m_channelIR.size(), m_fftSize);
    for (int i = 0; i < copyLen; ++i) {
        hReal[i] = m_channelIR[i];
    }

    fft(hReal, hImag, false);

    m_eqFilter.resize(m_fftSize);

    /* Estimate signal power from channel response */
    double signalPower = 0.0;
    for (int i = 0; i < copyLen; ++i) {
        signalPower += m_channelIR[i] * m_channelIR[i];
    }
    signalPower = qMax(1e-12, signalPower / copyLen);

    for (int k = 0; k < m_fftSize; ++k) {
        double Hr = hReal[k];
        double Hi = hImag[k];
        double Hmag2 = Hr * Hr + Hi * Hi;

        if (m_type == EqualizerType::ZeroForcing) {
            /* ZF: 1/H */
            if (Hmag2 > 1e-15) {
                double invDenom = 1.0 / Hmag2;
                m_eqFilter[k] = {Hr * invDenom, -Hi * invDenom};
            } else {
                m_eqFilter[k] = {0.0, 0.0};
            }
        } else {
            /* MMSE: conj(H) / (|H|^2 + noise_var/signal_var) */
            double regularization = m_noiseVariance / signalPower;
            double denom = Hmag2 + regularization;
            m_eqFilter[k] = {Hr / denom, -Hi / denom};
        }
    }

    m_filterReady = true;
}

/**
 * @brief 从训练序列估计信道脉冲响应
 *
 * 使用频域除法: H(f) = FFT(received) / FFT(transmitted)
 * IFFT得到信道脉冲响应。
 */
QVector<double> ChannelEqualizer::estimateChannel(const QVector<double>& transmitted,
                                                    const QVector<double>& received)
{
    const int n = qMin(transmitted.size(), received.size());
    if (n == 0) return QVector<double>();

    int fftLen = qMax(m_fftSize, n);
    /* Round to power of 2 */
    int p = 1;
    while (p < fftLen) p <<= 1;
    fftLen = p;

    QVector<double> tR(fftLen, 0.0), tI(fftLen, 0.0);
    QVector<double> rR(fftLen, 0.0), rI(fftLen, 0.0);

    for (int i = 0; i < n; ++i) {
        tR[i] = transmitted[i];
        rR[i] = received[i];
    }

    fft(tR, tI, false);
    fft(rR, rI, false);

    /* H(f) = R(f) / T(f) */
    QVector<double> hR(fftLen, 0.0), hI(fftLen, 0.0);
    for (int k = 0; k < fftLen; ++k) {
        double denom = tR[k] * tR[k] + tI[k] * tI[k];
        if (denom > 1e-15) {
            hR[k] = (rR[k] * tR[k] + rI[k] * tI[k]) / denom;
            hI[k] = (rI[k] * tR[k] - rR[k] * tI[k]) / denom;
        }
    }

    fft(hR, hI, true);

    /* Extract channel IR (truncate to reasonable length) */
    int irLen = qMin(n, 64);
    QVector<double> ir(irLen);
    for (int i = 0; i < irLen; ++i) {
        ir[i] = hR[i];
    }

    m_channelIR = ir;
    m_filterReady = false;
    return ir;
}

/**
 * @brief 设置已知信道响应
 */
void ChannelEqualizer::setChannelResponse(const QVector<double>& impulseResponse)
{
    m_channelIR = impulseResponse;
    m_filterReady = false;
}

/**
 * @brief 均衡接收信号(overlap-save频域卷积)
 */
QVector<double> ChannelEqualizer::equalize(const QVector<double>& received)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || m_channelIR.isEmpty()) return received;

    if (!m_filterReady) buildEqualizerFilter();
    if (!m_filterReady) return received;

    const int n = received.size();
    QVector<double> output(n, 0.0);
    int hopSize = m_fftSize / 2;

    for (int start = 0; start < n; start += hopSize) {
        int blockLen = qMin(m_fftSize, n - start + m_fftSize / 2);
        int blockStart = qMax(0, start - m_fftSize / 2);

        QVector<double> bR(m_fftSize, 0.0), bI(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && blockStart + i < n; ++i) {
            bR[i] = received[blockStart + i];
        }

        fft(bR, bI, false);

        /* Apply equalizer filter in frequency domain */
        for (int k = 0; k < m_fftSize; ++k) {
            double r = bR[k] * m_eqFilter[k].first - bI[k] * m_eqFilter[k].second;
            double im = bR[k] * m_eqFilter[k].second + bI[k] * m_eqFilter[k].first;
            bR[k] = r;
            bI[k] = im;
        }

        fft(bR, bI, true);

        /* Copy valid portion (discard first half due to circular conv) */
        for (int i = m_fftSize / 2; i < m_fftSize; ++i) {
            int outIdx = start + (i - m_fftSize / 2);
            if (outIdx < n) {
                output[outIdx] += bR[i];
            }
        }
    }

    /* Estimate SNR and MSE */
    double signalPower = 0.0;
    double errorPower = 0.0;
    for (int i = 0; i < n; ++i) {
        signalPower += output[i] * output[i];
    }
    errorPower = m_noiseVariance * n;
    m_stats.lastSNR = (errorPower > 1e-15) ? 10.0 * qLn(signalPower / errorPower) / qLn(10.0) : 100.0;
    m_stats.lastMSE = m_noiseVariance;

    m_stats.totalEqualized++;
    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEqualized > 0)
        ? m_timeSum / m_stats.totalEqualized : 0.0;

    emit equalizeCompleted(n, m_stats.lastSNR);
    return output;
}

/**
 * @brief 获取信道频率响应
 */
QVector<QPair<double, double>> ChannelEqualizer::channelFrequencyResponse() const
{
    if (m_channelIR.isEmpty()) return QVector<QPair<double, double>>();

    int fftLen = qMax(m_fftSize, m_channelIR.size());
    int p = 1;
    while (p < fftLen) p <<= 1;

    QVector<double> hR(p, 0.0), hI(p, 0.0);
    for (int i = 0; i < qMin(m_channelIR.size(), p); ++i) {
        hR[i] = m_channelIR[i];
    }

    /* Can't call static fft directly; compute magnitude manually */
    /* Use DFT for magnitude response */
    int respLen = p / 2 + 1;
    QVector<QPair<double, double>> response(respLen);
    for (int k = 0; k < respLen; ++k) {
        double realPart = 0.0, imagPart = 0.0;
        for (int n = 0; n < m_channelIR.size(); ++n) {
            double angle = -2.0 * M_PI * k * n / p;
            realPart += m_channelIR[n] * qCos(angle);
            imagPart += m_channelIR[n] * qSin(angle);
        }
        response[k] = {realPart, imagPart};
    }

    return response;
}

void ChannelEqualizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
