/**
 * @file SpectralLeakage2.cpp
 * @brief 频谱泄漏分析工具实现
 *
 * 分析窗函数的频谱泄漏特性，计算等效噪声带宽(ENBW)、
 * 标量损耗(SCALOSS)和最差情况处理增益等指标。
 * 支持Hann、Hamming、Blackman、FlatTop、Kaiser等窗函数。
 */

#include "utils/signal75/SpectralLeakage2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认窗大小1024点，窗类型Hann。
 * 内部状态初始化为零，统计计数器清零。
 */
SpectralLeakage2::SpectralLeakage2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析窗口大小
 * @param n 窗口长度(采样点数)，最小16
 */
void SpectralLeakage2::setWindowSize(int n)
{
    m_winSize = qMax(16, n);
}

/**
 * @brief 设置窗函数类型
 * @param type 窗函数名称
 */
void SpectralLeakage2::setWindowType(const QString& type)
{
    m_winType = type.toLower();
}

/**
 * @brief 生成指定类型的窗函数
 * @param n 窗口长度
 * @param type 窗函数类型名称
 * @return 窗函数系数向量
 *
 * 支持的窗函数:
 * - hann: w[n] = 0.5*(1 - cos(2pi*n/(N-1)))
 * - hamming: w[n] = 0.54 - 0.46*cos(2pi*n/(N-1))
 * - blackman: w[n] = 0.42 - 0.5*cos(...) + 0.08*cos(2*...)
 * - flattop: 平顶窗，幅度精度最优
 * - rectangular: 矩形窗，w[n] = 1
 * - kaiser: Kaiser窗(beta=8.0近似)
 */
QVector<double> SpectralLeakage2::generateWindow(int n, const QString& type)
{
    QVector<double> w(n, 1.0);
    QString t = type.toLower();

    if (t == "rectangular") {
        /* 矩形窗: 全1，频谱分辨率最高但旁瓣最大 */
        return w;
    }

    for (int i = 0; i < n; ++i) {
        double phase = 2.0 * M_PI * i / (n - 1);

        if (t == "hann") {
            /* Hann窗: 旁瓣-31.5dB，主瓣宽度2bin */
            w[i] = 0.5 * (1.0 - qCos(phase));
        } else if (t == "hamming") {
            /* Hamming窗: 旁瓣-42.7dB，非零端点 */
            w[i] = 0.54 - 0.46 * qCos(phase);
        } else if (t == "blackman") {
            /* Blackman窗: 旁瓣-58dB，主瓣宽度3bin */
            w[i] = 0.42 - 0.5 * qCos(phase) + 0.08 * qCos(2.0 * phase);
        } else if (t == "flattop") {
            /* 平顶窗: 幅度精度最高(0.01dB)，主瓣宽度最宽 */
            w[i] = 0.21557895 - 0.41663158 * qCos(phase)
                   + 0.277263158 * qCos(2.0 * phase)
                   - 0.083578947 * qCos(3.0 * phase)
                   + 0.006947368 * qCos(4.0 * phase);
        } else if (t == "kaiser") {
            /* Kaiser窗(beta=8.0): 可调旁瓣衰减 */
            double alpha = (n - 1) / 2.0;
            double beta = 8.0;
            double x = (i - alpha) / alpha;
            /* I0(beta) 近似计算 */
            double i0beta = 1.0;
            double term = 1.0;
            for (int k = 1; k <= 25; ++k) {
                term *= (beta / 2.0) * (beta / 2.0) / (k * k);
                i0beta += term;
            }
            double arg = beta * qSqrt(qMax(0.0, 1.0 - x * x));
            double i0arg = 1.0;
            term = 1.0;
            for (int k = 1; k <= 25; ++k) {
                term *= (arg / 2.0) * (arg / 2.0) / (k * k);
                i0arg += term;
            }
            w[i] = (arg > 0) ? i0arg / i0beta : 1.0 / i0beta;
        } else {
            /* 未知类型默认使用Hann */
            w[i] = 0.5 * (1.0 - qCos(phase));
        }
    }

    return w;
}

/**
 * @brief 计算等效噪声带宽(ENBW)
 * @return ENBW值(以bin为单位)
 *
 * ENBW = N * sum(w^2) / (sum(w))^2
 *
 * 表示窗函数等效于多宽的矩形窗带宽。
 * ENBW越大，频谱分辨率越低但幅度精度越高。
 * Hann窗ENBW = 1.5 bin, Blackman窗ENBW = 2.0 bin
 * Rectangular窗ENBW = 1.0 bin
 */
double SpectralLeakage2::computeENBW()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w = generateWindow(m_winSize, m_winType);
    double sumW = 0.0, sumW2 = 0.0;

    for (int i = 0; i < w.size(); ++i) {
        sumW += w[i];
        sumW2 += w[i] * w[i];
    }

    double enbw = (qAbs(sumW) > 1e-300) ? (m_winSize * sumW2 / (sumW * sumW)) : 1.0;

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalWindows++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(enbw, 0.0);
    return enbw;
}

/**
 * @brief 计算标量损耗(SCALOSS)
 * @return SCALOSS值(dB)
 *
 * SCALOSS = 10*log10(sum(w^2) / (max(w))^2)
 *
 * 表示使用窗函数后信号幅度的最大损耗。
 * 该值越接近0dB表示幅度精度越高。
 * FlatTop窗: ~0.01dB, Hann窗: ~-1.42dB
 */
double SpectralLeakage2::computeSCALOSS()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w = generateWindow(m_winSize, m_winType);

    double maxW = *std::max_element(w.begin(), w.end());
    double sumW2 = 0.0;
    for (int i = 0; i < w.size(); ++i) {
        sumW2 += w[i] * w[i];
    }

    double scaloss = 0.0;
    if (maxW > 1e-300 && sumW2 > 0) {
        scaloss = 10.0 * qLn(sumW2 / (maxW * maxW * w.size())) / qLn(10.0);
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalWindows++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(0.0, scaloss);
    return scaloss;
}

/**
 * @brief 重置统计信息
 */
void SpectralLeakage2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
