/**
 * @file TrigonometricInterp.cpp
 * @brief 三角插值引擎实现 — FFT驱动的周期信号插值
 */

#include "utils/interp6/TrigonometricInterp.h"

#include <QtMath>
#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
TrigonometricInterp::TrigonometricInterp(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置插值模式 @param mode 模式 */
void TrigonometricInterp::setMode(Mode mode)
{
    m_mode = mode;
}

/** @brief 对数据进行三角插值 @param data 采样数据 @param outputSize 输出点数 @return 插值结果 */
QVector<double> TrigonometricInterp::interpolate(const QVector<double>& data,
                                                  int outputSize)
{
    if (data.size() < 2) return data;

    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int outLen = (outputSize > 0) ? outputSize : n * 2;

    /* 选择FFT长度(>=n, 2的幂) */
    int fftLen = nextPowerOf2(n);

    /* 零填充到fftLen */
    QVector<QComplexDouble> freq(fftLen);
    for (int i = 0; i < n; ++i) {
        freq[i] = QComplexDouble(data[i], 0.0);
    }

    /* 前向FFT */
    switch (m_mode) {
    case Mode::FFT:
    case Mode::DFT:
        fft(freq);
        m_stats.totalFftsExecuted += 1;
        break;
    case Mode::SlowDFT:
        slowDft(freq);
        m_stats.totalFftsExecuted += 1;
        break;
    }

    /* 将频谱插入到更大的FFT长度 */
    int outFftLen = nextPowerOf2(outLen);

    QVector<QComplexDouble> expanded(outFftLen);
    int halfN = fftLen / 2;

    /* 保留DC和正频率 */
    expanded[0] = freq[0];
    for (int k = 1; k <= halfN; ++k) {
        expanded[k] = freq[k];
    }

    /* 负频率放到高频端 */
    for (int k = 1; k < halfN; ++k) {
        expanded[outFftLen - k] = freq[fftLen - k];
    }

    /* 中间补零 */
    for (int k = halfN + 1; k < outFftLen - halfN; ++k) {
        expanded[k] = QComplexDouble(0.0, 0.0);
    }

    /* 逆FFT */
    fft(expanded, true);
    m_stats.totalFftsExecuted += 1;

    /* 提取结果 */
    double scale = static_cast<double>(n) / static_cast<double>(fftLen);
    QVector<double> result(outLen);
    for (int i = 0; i < outLen; ++i) {
        result[i] = expanded[i % outFftLen].real() * scale;
    }

    /* 统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalInterpolations;
    m_stats.totalPointsGenerated += outLen;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    emit interpolationComplete(n, outLen);
    return result;
}

/** @brief 在指定位置插值 @param data 采样数据 @param positions 位置[0,1) @return 插值结果 */
QVector<double> TrigonometricInterp::interpolateAt(const QVector<double>& data,
                                                    const QVector<double>& positions)
{
    if (data.size() < 2) return QVector<double>(positions.size(), 0.0);

    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int fftLen = nextPowerOf2(n);

    /* 计算FFT */
    QVector<QComplexDouble> freq(fftLen);
    for (int i = 0; i < n; ++i) {
        freq[i] = QComplexDouble(data[i], 0.0);
    }
    fft(freq);
    m_stats.totalFftsExecuted += 1;

    /* 在每个位置用DFT求值 */
    QVector<double> result;
    result.reserve(positions.size());

    for (double pos : positions) {
        /* 归一化位置到[0, 2*PI) */
        double t = pos * 2.0 * M_PI;
        QComplexDouble sum(0.0, 0.0);

        int halfLen = fftLen / 2;
        /* DC分量 */
        sum += freq[0];

        /* 正频率 */
        for (int k = 1; k <= halfLen; ++k) {
            double angle = static_cast<double>(k) * t;
            QComplexDouble twiddle(qCos(angle), qSin(angle));
            sum += freq[k] * twiddle;
        }

        /* 负频率 */
        for (int k = halfLen + 1; k < fftLen; ++k) {
            int negK = k - fftLen;
            double angle = static_cast<double>(negK) * t;
            QComplexDouble twiddle(qCos(angle), qSin(angle));
            sum += freq[k] * twiddle;
        }

        result.append(sum.real() / fftLen);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalInterpolations;
    m_stats.totalPointsGenerated += positions.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    return result;
}

/** @brief 提取频谱信息 @param data 采样数据 @param sampleRate 采样率 @return 频谱 */
TrigonometricInterp::SpectrumInfo TrigonometricInterp::analyzeSpectrum(
    const QVector<double>& data, double sampleRate)
{
    SpectrumInfo info;
    if (data.size() < 2) return info;

    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int fftLen = nextPowerOf2(n);

    QVector<QComplexDouble> freq(fftLen);
    for (int i = 0; i < n; ++i) {
        freq[i] = QComplexDouble(data[i], 0.0);
    }
    fft(freq);
    m_stats.totalFftsExecuted += 1;

    int halfLen = fftLen / 2;
    info.frequencies.resize(halfLen);
    info.magnitudes.resize(halfLen);
    info.phases.resize(halfLen);

    info.dcComponent = freq[0].real() / fftLen;
    double maxMag = 0.0;

    for (int k = 0; k < halfLen; ++k) {
        double f = static_cast<double>(k) * sampleRate / fftLen;
        info.frequencies[k] = f;

        double mag = 2.0 * std::abs(freq[k]) / fftLen;
        info.magnitudes[k] = mag;

        double phase = std::arg(freq[k]);
        info.phases[k] = phase;

        if (k > 0 && mag > maxMag) {
            maxMag = mag;
            info.dominantFreq = f;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalInterpolations;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    return info;
}

/** @brief 去趋势后插值 @param data 原始数据 @param outputSize 输出点数 @return 插值结果 */
QVector<double> TrigonometricInterp::detrendedInterpolate(
    const QVector<double>& data, int outputSize)
{
    if (data.size() < 3) return data;

    /* 去除线性趋势 */
    QVector<double> detrended = detrend(data);

    /* 三角插值 */
    QVector<double> result = interpolate(detrended, outputSize);

    /* 重新添加线性趋势 */
    int n = data.size();
    int outLen = result.size();

    /* 最小二乘拟合线性趋势 */
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        sumX += x;
        sumY += data[i];
        sumXY += x * data[i];
        sumX2 += x * x;
    }
    double denom = n * sumX2 - sumX * sumX;
    double slope = (qFuzzyIsNull(denom)) ? 0.0 : (n * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / n;

    /* 将趋势添加回插值结果 */
    for (int i = 0; i < outLen; ++i) {
        double t = static_cast<double>(i) * static_cast<double>(n - 1)
                   / static_cast<double>(outLen - 1);
        result[i] += slope * t + intercept;
    }

    return result;
}

/** @brief 重置统计 */
void TrigonometricInterp::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT @param data 复数数据 @param inverse 逆变换 */
void TrigonometricInterp::fft(QVector<QComplexDouble>& data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }

    /* 蝶形运算 */
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        QComplexDouble wLen(qCos(angle), qSin(angle));

        for (int i = 0; i < n; i += len) {
            QComplexDouble w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                QComplexDouble u = data[i + j];
                QComplexDouble v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w = w * wLen;
            }
        }
    }

    if (inverse) {
        for (auto& d : data) {
            d /= static_cast<double>(n);
        }
    }
}

/** @brief 慢速DFT @param data 复数数据 @param inverse 逆变换 */
void TrigonometricInterp::slowDft(QVector<QComplexDouble>& data, bool inverse)
{
    int n = data.size();
    QVector<QComplexDouble> result(n);

    double sign = inverse ? 1.0 : -1.0;
    for (int k = 0; k < n; ++k) {
        QComplexDouble sum(0.0, 0.0);
        for (int j = 0; j < n; ++j) {
            double angle = sign * 2.0 * M_PI * k * j / n;
            QComplexDouble twiddle(qCos(angle), qSin(angle));
            sum += data[j] * twiddle;
        }
        result[k] = inverse ? sum / static_cast<double>(n) : sum;
    }

    data = result;
}

/** @brief 下一个2的幂 @param n 输入 @return 2幂 */
int TrigonometricInterp::nextPowerOf2(int n)
{
    if (n <= 1) return 1;
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/** @brief 去除线性趋势 @param data 数据 @return 去趋势数据 */
QVector<double> TrigonometricInterp::detrend(const QVector<double>& data) const
{
    int n = data.size();
    if (n < 2) return data;

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        sumX += x;
        sumY += data[i];
        sumXY += x * data[i];
        sumX2 += x * x;
    }

    double denom = n * sumX2 - sumX * sumX;
    if (qFuzzyIsNull(denom)) return data;

    double slope = (n * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / n;

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = data[i] - (slope * i + intercept);
    }
    return result;
}
