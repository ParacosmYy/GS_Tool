#include "RecursiveDFT5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化递归DFT引擎
 * @param parent 父对象指针
 */
RecursiveDFT5::RecursiveDFT5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void RecursiveDFT5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Cooley-Tukey递归FFT核心
 *
 * 将N点DFT分解为两个N/2点DFT（偶数/奇数下标），
 * 递归计算后通过蝶形运算合并结果。
 *
 * @param re 实部数组
 * @param im 虚部数组
 * @param depth 当前递归深度
 */
static void cooleyTukey(QVector<double>& re, QVector<double>& im, int depth, int& maxDepth)
{
    const int n = re.size();
    if (n <= 1) return;
    maxDepth = qMax(maxDepth, depth);

    /* 分离偶数和奇数下标 */
    QVector<double> evenRe(n / 2), evenIm(n / 2);
    QVector<double> oddRe(n / 2), oddIm(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        evenRe[i] = re[2 * i];
        evenIm[i] = im[2 * i];
        oddRe[i]  = re[2 * i + 1];
        oddIm[i]  = im[2 * i + 1];
    }

    /* 递归求解子问题 */
    cooleyTukey(evenRe, evenIm, depth + 1, maxDepth);
    cooleyTukey(oddRe, oddIm, depth + 1, maxDepth);

    /* 蝶形合并 */
    for (int k = 0; k < n / 2; ++k) {
        double angle = -2.0 * M_PI * k / n;
        double wRe = qCos(angle);
        double wIm = qSin(angle);
        double tRe = wRe * oddRe[k] - wIm * oddIm[k];
        double tIm = wRe * oddIm[k] + wIm * oddRe[k];
        re[k]         = evenRe[k] + tRe;
        im[k]         = evenIm[k] + tIm;
        re[k + n / 2] = evenRe[k] - tRe;
        im[k + n / 2] = evenIm[k] - tIm;
    }
}

/**
 * @brief 执行前向DFT变换
 *
 * 对输入信号执行递归Cooley-Tukey FFT。如果输入长度非2的幂，
 * 自动补零到最近的2的幂长度。
 *
 * @param samples 输入时域采样
 * @return 复数频谱 [实部, 虚部] 对
 */
QVector<QPair<double, double>> RecursiveDFT5::forward(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int n = samples.size();
    if (n == 0) {
        emit transformCompleted(0);
        return result;
    }

    /* 计算大于等于n的最小2的幂 */
    int fftLen = 1;
    while (fftLen < n) fftLen <<= 1;

    QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
    for (int i = 0; i < n; ++i) re[i] = samples[i];

    int maxDepth = 0;
    cooleyTukey(re, im, 1, maxDepth);

    result.reserve(fftLen);
    for (int i = 0; i < fftLen; ++i)
        result.append({re[i], im[i]});

    if (m_maxDepth > 0 && maxDepth > m_maxDepth)
        m_stats.maxRecursionDepth = m_maxDepth;
    else
        m_stats.maxRecursionDepth = qMax(m_stats.maxRecursionDepth, maxDepth);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(fftLen);
    return result;
}

/**
 * @brief 执行逆DFT变换
 *
 * 利用共轭技巧实现IDFT：对频谱取共轭，执行FFT，再取共轭并除以N。
 *
 * @param spectrum 复数频谱 [实部, 虚部] 对
 * @return 重建的时域采样
 */
QVector<double> RecursiveDFT5::inverse(const QVector<QPair<double, double>>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    const int n = spectrum.size();
    if (n == 0) {
        emit transformCompleted(0);
        return result;
    }

    int fftLen = 1;
    while (fftLen < n) fftLen <<= 1;

    /* 取共轭 */
    QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] =  spectrum[i].first;
        im[i] = -spectrum[i].second;
    }

    int maxDepth = 0;
    cooleyTukey(re, im, 1, maxDepth);

    result.reserve(fftLen);
    for (int i = 0; i < fftLen; ++i)
        result.append(re[i] / fftLen);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    m_stats.maxRecursionDepth = qMax(m_stats.maxRecursionDepth, maxDepth);

    emit transformCompleted(fftLen);
    return result;
}

/**
 * @brief 计算指定频率点的DFT
 *
 * 直接使用DFT定义公式计算单个频率bin，无需执行完整FFT。
 * 复杂度O(N)，适合只需少量频率点的场景。
 *
 * @param samples 时域采样
 * @param freqIndex 频率bin索引
 * @return 该频率点的复数值
 */
QPair<double, double> RecursiveDFT5::singleBin(const QVector<double>& samples, int freqIndex) const
{
    const int n = samples.size();
    if (n == 0 || freqIndex < 0) return {0.0, 0.0};

    double re = 0.0, im = 0.0;
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * freqIndex * k / n;
        re += samples[k] * qCos(angle);
        im += samples[k] * qSin(angle);
    }
    return {re, im};
}
