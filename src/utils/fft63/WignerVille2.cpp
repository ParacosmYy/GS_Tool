/**
 * @file WignerVille2.cpp
 * @brief Wigner-Ville时频分布实现 — 二次型时频分析
 *
 * 计算信号的Wigner-Ville分布(WVD)，提供高分辨率的时频表示。
 * 支持加窗伪WVD以抑制交叉项干扰，输出时间-频率二维分布矩阵。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft63/WignerVille2.h"

#include <QElapsedTimer>
#include <QStringList>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
WignerVille2::WignerVille2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置信号长度
 * @param n 信号长度(采样点数)，自动向上取整到偶数
 */
void WignerVille2::setSignalLength(int n)
{
    m_n = qMax(4, n);
    if (m_n % 2 != 0) m_n++; /* 确保偶数长度以简化计算 */
}

/**
 * @brief 设置窗函数类型
 * @param type 窗函数名称: "hann"(默认), "hamming", "rect"
 */
void WignerVille2::setWindowType(const QString& type)
{
    if (type == "hann" || type == "hamming" || type == "rect") {
        m_winType = type;
    }
}

/**
 * @brief 计算Wigner-Ville分布
 *
 * 对输入信号计算WVD: W(t,f) = integral s(t+tau/2)*s*(t-tau/2)*exp(-j2pi*f*tau) dtau
 * 使用伪WVD加窗抑制交叉项。
 *
 * @param signal 输入信号
 * @return 时频分布矩阵，行对应时间，列对应频率
 */
QVector<QVector<double>> WignerVille2::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    if (N < 4) {
        return {};
    }

    /* 实际使用的长度 */
    const int n = qMin(N, m_n);
    const int halfN = n / 2;

    /* 生成窗函数 */
    QVector<double> window = generateWindow(n);

    /* 输出矩阵: 时间bin数 x 频率bin数 */
    const int timeBins = n;
    const int freqBins = n;
    QVector<QVector<double>> wvd(timeBins, QVector<double>(freqBins, 0.0));

    /* 对每个时间点计算瞬时自相关并进行DFT */
    for (int t = 0; t < n; ++t) {
        /* 构造瞬时自相关序列 r(t, tau) = s(t+tau/2) * s(t-tau/2) */
        QVector<double> kernel(freqBins, 0.0);

        for (int tau = 0; tau < freqBins; ++tau) {
            int halfTau = tau / 2;
            int idxPlus = t + halfTau;
            int idxMinus = t - halfTau;

            /* 边界处理: 镜像扩展 */
            if (idxPlus >= n) idxPlus = 2 * n - 2 - idxPlus;
            if (idxMinus < 0) idxMinus = -idxMinus;

            idxPlus = qBound(0, idxPlus, n - 1);
            idxMinus = qBound(0, idxMinus, n - 1);

            kernel[tau] = signal[idxPlus] * signal[idxMinus] * window[tau];
        }

        /* 对自相关序列执行DFT得到频率分布 */
        for (int f = 0; f < freqBins; ++f) {
            double re = 0.0;
            for (int tau = 0; tau < freqBins; ++tau) {
                double angle = -2.0 * M_PI * static_cast<double>(f) * static_cast<double>(tau)
                             / static_cast<double>(freqBins);
                re += kernel[tau] * qCos(angle);
            }
            wvd[t][f] = re;
        }
    }

    /* 更新统计信息 */
    m_stats.totalDistributions++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDistributions;

    emit distributionComputed(n, timeBins);
    return wvd;
}

/**
 * @brief 获取最大频率
 * @return 最大可分析频率(假设44100Hz采样率)
 */
double WignerVille2::maxFrequency() const
{
    return 22050.0; /* 奈奎斯特频率 = 44100/2 */
}

/**
 * @brief 重置所有统计数据
 */
void WignerVille2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 生成指定长度的窗函数
 *
 * 支持三种窗函数:
 * - hann: Hann窗，主瓣宽、旁瓣低，适合一般时频分析
 * - hamming: Hamming窗，旁瓣最低，适合频谱分析
 * - rect: 矩形窗，主瓣最窄但旁瓣高
 *
 * @param n 窗函数长度
 * @return 窗函数系数向量
 */
QVector<double> WignerVille2::generateWindow(int n) const
{
    QVector<double> w(n, 1.0);
    if (m_winType == "hann") {
        /* Hann窗: 0.5 * (1 - cos(2*pi*n/(N-1))) */
        for (int i = 0; i < n; ++i) {
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * static_cast<double>(i)
                      / static_cast<double>(n - 1)));
        }
    } else if (m_winType == "hamming") {
        /* Hamming窗: 0.54 - 0.46 * cos(2*pi*n/(N-1)) */
        for (int i = 0; i < n; ++i) {
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * static_cast<double>(i)
                      / static_cast<double>(n - 1));
        }
    }
    /* "rect"窗为全1，已初始化 */
    return w;
}

/**
 * @brief 计算WVD的时间边缘特性
 *
 * 对WVD沿频率轴积分应恢复原始信号功率。
 * 用于验证WVD计算的正确性。
 *
 * @param wvd Wigner-Ville分布矩阵
 * @return 时间边缘(每行的能量总和)
 */
QVector<double> WignerVille2::timeMarginal(const QVector<QVector<double>>& wvd) const
{
    const int timeBins = wvd.size();
    QVector<double> marginal(timeBins, 0.0);
    for (int t = 0; t < timeBins; ++t) {
        double sum = 0.0;
        const int freqBins = wvd[t].size();
        for (int f = 0; f < freqBins; ++f) {
            sum += wvd[t][f];
        }
        marginal[t] = sum;
    }
    return marginal;
}

/**
 * @brief 计算WVD的频率边缘特性
 *
 * 对WVD沿时间轴积分应恢复功率谱密度。
 * 用于验证WVD的能量守恒特性。
 *
 * @param wvd Wigner-Ville分布矩阵
 * @return 频率边缘(每列的能量总和)
 */
QVector<double> WignerVille2::freqMarginal(const QVector<QVector<double>>& wvd) const
{
    if (wvd.isEmpty()) return {};
    const int timeBins = wvd.size();
    const int freqBins = wvd[0].size();
    QVector<double> marginal(freqBins, 0.0);
    for (int f = 0; f < freqBins; ++f) {
        for (int t = 0; t < timeBins; ++t) {
            marginal[f] += wvd[t][f];
        }
    }
    return marginal;
}

/**
 * @brief 获取当前配置参数摘要
 * @return 参数描述字符串列表
 */
QStringList WignerVille2::parameterSummary() const
{
    QStringList summary;
    summary << QString("信号长度: %1").arg(m_n);
    summary << QString("窗函数: %1").arg(m_winType);
    return summary;
}
