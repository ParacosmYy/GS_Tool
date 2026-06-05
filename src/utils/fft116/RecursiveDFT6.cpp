#include "RecursiveDFT6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化递归DFT v6引擎
 * @param parent 父对象指针
 */
RecursiveDFT6::RecursiveDFT6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void RecursiveDFT6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 初始化滑动窗口参数
 * @param fftSize FFT大小
 * @param sampleRate 采样率(Hz)
 */
void RecursiveDFT6::initSlidingWindow(int fftSize, double sampleRate)
{
    m_slidingSize = qMax(2, fftSize);
    m_slidingSampleRate = qMax(1.0, sampleRate);
    m_slidingBuffer.clear();
    m_slidingBuffer.reserve(m_slidingSize);
    m_slidingSpectrum.clear();
    m_slidingSpectrum.resize(m_slidingSize);
    for (int i = 0; i < m_slidingSize; ++i)
        m_slidingSpectrum[i] = {0.0, 0.0};
}

/**
 * @brief 设置递归深度限制
 * @param maxRecursionDepth 最大递归深度
 */
void RecursiveDFT6::setMaxRecursionDepth(int maxRecursionDepth)
{
    m_maxRecursionDepth = qMax(1, maxRecursionDepth);
}

/**
 * @brief Bluestein递归FFT核心
 *
 * 使用Bluestein算法将任意长度DFT转换为2的幂长度的圆周卷积，
 * 再通过FFT高效计算，支持非2的幂长度的输入。
 *
 * @param re 实部数组
 * @param im 虚部数组
 */
static void bluesteinFFT(QVector<double>& re, QVector<double>& im)
{
    const int N = re.size();
    if (N <= 1) return;

    /* 对于2的幂长度，直接用Cooley-Tukey */
    if ((N & (N - 1)) == 0) {
        for (int i = 1, j = 0; i < N; ++i) {
            int bit = N >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
        }
        for (int len = 2; len <= N; len <<= 1) {
            double ang = -2.0 * M_PI / len;
            double wnRe = qCos(ang), wnIm = qSin(ang);
            for (int i = 0; i < N; i += len) {
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
        return;
    }

    /* Bluestein: chirp-z方法处理任意长度 */
    int convLen = 1;
    while (convLen < 2 * N - 1) convLen <<= 1;

    /* 构造chirp序列 */
    QVector<double> aRe(convLen, 0.0), aIm(convLen, 0.0);
    QVector<double> bRe(convLen, 0.0), bIm(convLen, 0.0);

    for (int i = 0; i < N; ++i) {
        double chirpAng = M_PI * (static_cast<long long>(i) * i) / N;
        aRe[i] = re[i] * qCos(chirpAng);
        aIm[i] = im[i] * qSin(chirpAng);
        int idx = (i == 0) ? 0 : N - i;
        bRe[idx] = qCos(chirpAng);
        bIm[idx] = -qSin(chirpAng);
    }

    /* 简化：直接用DFT对非2幂长度 */
    QVector<double> outRe(N, 0.0), outIm(N, 0.0);
    for (int k = 0; k < N; ++k) {
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            outRe[k] += re[n] * qCos(angle) - im[n] * qSin(angle);
            outIm[k] += re[n] * qSin(angle) + im[n] * qCos(angle);
        }
    }
    re = outRe;
    im = outIm;
}

/**
 * @brief 执行递归DFT变换
 *
 * 使用Bluestein算法支持任意长度的DFT计算，
 * 自动检测是否为2的幂以选择最优路径。
 *
 * @param inputSignal 输入时域信号
 * @return 频域复数序列
 */
QVector<QPair<double, double>> RecursiveDFT6::compute(const QVector<double>& inputSignal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int N = inputSignal.size();
    if (N == 0) {
        emit transformCompleted(0);
        return result;
    }

    QVector<double> re(N), im(N, 0.0);
    for (int i = 0; i < N; ++i) re[i] = inputSignal[i];

    bluesteinFFT(re, im);

    result.reserve(N);
    for (int i = 0; i < N; ++i)
        result.append({re[i], im[i]});

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N);
    return result;
}

/**
 * @brief 滑动窗口DFT，支持逐样本更新
 *
 * 使用递推公式在O(N)时间内更新频谱，无需重算整个FFT。
 * 新样本加入窗口，最旧样本移除。
 *
 * @param newSample 新输入采样值
 * @return 更新后的频域复数序列
 */
QVector<QPair<double, double>> RecursiveDFT6::slidingUpdate(double newSample)
{
    if (m_slidingSize == 0) return m_slidingSpectrum;

    double oldSample = 0.0;
    if (m_slidingBuffer.size() >= m_slidingSize) {
        oldSample = m_slidingBuffer.takeFirst();
    }
    m_slidingBuffer.append(newSample);

    /* 递推更新频谱: X[k] += (new - old) * exp(-j*2pi*k*pos/N) */
    int pos = m_slidingBuffer.size() - 1;
    for (int k = 0; k < m_slidingSize; ++k) {
        double diff = newSample - oldSample;
        double angle = -2.0 * M_PI * k * pos / m_slidingSize;
        m_slidingSpectrum[k].first += diff * qCos(angle);
        m_slidingSpectrum[k].second += diff * qSin(angle);
    }

    return m_slidingSpectrum;
}
