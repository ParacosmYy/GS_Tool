#include "ZoomFFT7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化缩放FFT引擎
 * @param parent 父对象指针
 */
ZoomFFT7::ZoomFFT7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ZoomFFT7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置抽取因子
 * @param decimationFactor 抽取倍数
 */
void ZoomFFT7::setDecimationFactor(int decimationFactor)
{
    m_decimationFactor = qMax(1, decimationFactor);
}

/**
 * @brief 就地基2 FFT
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im)
{
    const int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wnRe = qCos(ang), wnIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double wRe = 1.0, wIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = wRe * re[i+j+len/2] - wIm * im[i+j+len/2];
                double tIm = wRe * im[i+j+len/2] + wIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double nw = wRe * wnRe - wIm * wnIm;
                wIm = wRe * wnIm + wIm * wnRe;
                wRe = nw;
            }
        }
    }
}

/**
 * @brief 执行缩放FFT变换
 *
 * 步骤：频率搬移(复数调制) → 低通滤波 → 抽取 → FFT。
 * 实现窄带信号的高分辨率频谱分析。
 *
 * @param inputSignal 输入时域信号
 * @param centerFreqHz 中心频率(Hz)
 * @param bandwidthHz 分析带宽(Hz)
 * @return 细分辨后的频域复数序列
 */
QVector<QPair<double, double>> ZoomFFT7::compute(
    const QVector<double>& inputSignal, double centerFreqHz, double bandwidthHz)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int N = inputSignal.size();
    if (N == 0) {
        emit zoomCompleted(0);
        return result;
    }

    /* 步骤1：复数调制 - 将中心频率搬移到零频 */
    double sampleRate = 2.0 * bandwidthHz;
    QVector<double> re(N), im(N, 0.0);
    for (int i = 0; i < N; ++i) {
        double angle = -2.0 * M_PI * centerFreqHz * i / sampleRate;
        re[i] = inputSignal[i] * qCos(angle);
        im[i] = inputSignal[i] * qSin(angle);
    }

    /* 步骤2：低通滤波（简单移动平均） */
    int filterLen = qMax(1, m_decimationFactor);
    QVector<double> filtered(N, 0.0), filteredIm(N, 0.0);
    for (int i = 0; i < N; ++i) {
        double sumRe = 0.0, sumIm = 0.0;
        int count = 0;
        for (int j = 0; j < filterLen && i - j >= 0; ++j) {
            sumRe += re[i - j];
            sumIm += im[i - j];
            count++;
        }
        filtered[i] = sumRe / count;
        filteredIm[i] = sumIm / count;
    }

    /* 步骤3：抽取 */
    int decimatedLen = N / m_decimationFactor;
    if (decimatedLen < 1) decimatedLen = 1;

    /* 补零到2的幂 */
    int fftLen = 1;
    while (fftLen < decimatedLen) fftLen <<= 1;

    QVector<double> fftRe(fftLen, 0.0), fftIm(fftLen, 0.0);
    for (int i = 0; i < decimatedLen; ++i) {
        fftRe[i] = filtered[i * m_decimationFactor];
        fftIm[i] = filteredIm[i * m_decimationFactor];
    }

    /* 步骤4：FFT */
    radix2FFT(fftRe, fftIm);

    result.reserve(fftLen);
    for (int i = 0; i < fftLen; ++i)
        result.append({fftRe[i], fftIm[i]});

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit zoomCompleted(fftLen);
    return result;
}

/**
 * @brief 获取指定频段的幅度谱
 * @param zoomResult 缩放FFT复数结果
 * @return 频率-幅度对序列
 */
QVector<QPair<double, double>> ZoomFFT7::getMagnitudeSpectrum(
    const QVector<QPair<double, double>>& zoomResult)
{
    QVector<QPair<double, double>> spectrum;
    const int n = zoomResult.size();
    if (n == 0) return spectrum;

    spectrum.reserve(n);
    for (int i = 0; i < n; ++i) {
        double mag = qSqrt(zoomResult[i].first * zoomResult[i].first +
                           zoomResult[i].second * zoomResult[i].second);
        spectrum.append({static_cast<double>(i) / n, mag});
    }
    return spectrum;
}
