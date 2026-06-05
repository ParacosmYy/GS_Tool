/**
 * @file FormantTracker.cpp
 * @brief 共振峰跟踪实现 — LPC分析/根求解/轨迹平滑/F1-F2映射
 */

#include "utils/signal32/FormantTracker.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <complex>

/** @brief 构造函数 @param parent 父对象 */
FormantTracker::FormantTracker(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void FormantTracker::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置LPC阶数 @param order 阶数 */
void FormantTracker::setLPCOrder(int order)
{
    m_lpcOrder = qMax(2, order);
}

/** @brief 设置最大共振峰数 @param max 最大共振峰数 */
void FormantTracker::setMaxFormants(int max)
{
    m_maxFormants = qBound(1, max, 10);
}

/** @brief 分析单帧信号的共振峰 @param frame 一帧音频数据 @return 共振峰列表 */
QVector<FormantTracker::Formant> FormantTracker::analyzeFrame(
    const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Formant> result;

    if (frame.size() < m_lpcOrder + 1) return result;

    /* 第一步: LPC分析获取预测系数 */
    QVector<double> lpc = lpcAnalysis(frame);

    /* 第二步: 求LPC多项式的根 (转换为解析信号平面上的极点) */
    int order = lpc.size();
    QVector<QPair<double, double>> roots;

    /* 使用Durand-Kerner方法求解多项式根 */
    int nRoots = order - 1;
    if (nRoots <= 0) return result;

    /* 初始化根的猜测值(均匀分布在单位圆上) */
    std::vector<std::complex<double>> z(nRoots);
    for (int i = 0; i < nRoots; ++i) {
        double angle = 2.0 * M_PI * i / nRoots + 0.1;
        z[i] = std::complex<double>(0.5 * qCos(angle), 0.5 * qSin(angle));
    }

    /* Durand-Kerner迭代 */
    for (int iter = 0; iter < 100; ++iter) {
        double maxChange = 0.0;
        for (int i = 0; i < nRoots; ++i) {
            /* 计算多项式在z[i]处的值 */
            std::complex<double> pz(lpc[0], 0.0);
            for (int j = 1; j < order; ++j) {
                pz = pz * z[i] + std::complex<double>(lpc[j], 0.0);
            }

            /* 计算其他根的乘积 */
            std::complex<double> denom(1.0, 0.0);
            for (int j = 0; j < nRoots; ++j) {
                if (j != i) denom *= (z[i] - z[j]);
            }

            if (std::abs(denom) > 1e-12) {
                std::complex<double> delta = pz / denom;
                z[i] -= delta;
                maxChange = qMax(maxChange, std::abs(delta));
            }
        }
        if (maxChange < 1e-10) break;
    }

    /* 提取物理上有意义的根(单位圆内的共轭对) */
    for (int i = 0; i < nRoots; ++i) {
        double mag = std::abs(z[i]);
        double re = z[i].real();
        double im = z[i].imag();
        /* 只保留虚部为正(共轭对的一半)且在单位圆内的根 */
        if (im > 0 && mag < 1.0) {
            roots.append(qMakePair(re, im));
        }
    }

    /* 第三步: 根转共振峰频率和带宽 */
    result = rootsToFormants(roots);

    /* 按频率排序并截取前m_maxFormants个 */
    std::sort(result.begin(), result.end(),
              [](const Formant& a, const Formant& b) {
                  return a.frequency < b.frequency;
              });
    if (result.size() > m_maxFormants) {
        result.resize(m_maxFormants);
    }

    m_stats.totalFrames++;
    m_stats.totalFormantsFound += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFrames));

    emit frameAnalyzed(m_stats.totalFrames, result.size());
    return result;
}

/** @brief 分析完整音频序列 @param audio 音频数据 @param frameSize 帧长 @param hopSize 步长 @return 每帧的共振峰列表 */
QVector<QVector<FormantTracker::Formant>> FormantTracker::analyze(
    const QVector<double>& audio, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<Formant>> allFormants;

    if (audio.isEmpty() || frameSize <= 0 || hopSize <= 0) return allFormants;

    int pos = 0;
    while (pos + frameSize <= audio.size()) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = audio[pos + i];
        }

        /* 加窗(Hamming窗) */
        for (int i = 0; i < frameSize; ++i) {
            double w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (frameSize - 1));
            frame[i] *= w;
        }

        allFormants.append(analyzeFrame(frame));
        pos += hopSize;
    }

    m_timeSum += timer.elapsed();
    return allFormants;
}

/** @brief LPC分析(自相关法+Levinson-Durbin递归) @param frame 加窗帧 @return LPC系数 */
QVector<double> FormantTracker::lpcAnalysis(const QVector<double>& frame)
{
    int n = frame.size();
    int p = qMin(m_lpcOrder, n - 1);

    /* 计算自相关函数 */
    QVector<double> r(p + 1, 0.0);
    for (int k = 0; k <= p; ++k) {
        for (int i = 0; i < n - k; ++i) {
            r[k] += frame[i] * frame[i + k];
        }
    }

    /* Levinson-Durbin递归 */
    QVector<double> a(p + 1, 0.0);
    a[0] = 1.0;
    double e = r[0];
    if (e < 1e-15) return a;

    for (int i = 1; i <= p; ++i) {
        double lambda = 0.0;
        for (int j = 1; j < i; ++j) {
            lambda += a[j] * r[i - j];
        }
        lambda = (r[i] - lambda) / e;

        /* 更新系数 */
        QVector<double> aNew = a;
        for (int j = 1; j < i; ++j) {
            aNew[j] = a[j] - lambda * a[i - j];
        }
        aNew[i] = -lambda;
        a = aNew;

        e *= (1.0 - lambda * lambda);
        if (e < 1e-15) break;
    }

    return a;
}

/** @brief 将LPC根转换为共振峰参数 @param roots 复数根对(实部,虚部) @return 共振峰列表 */
QVector<FormantTracker::Formant> FormantTracker::rootsToFormants(
    const QVector<QPair<double, double>>& roots)
{
    QVector<Formant> formants;

    for (const auto& root : roots) {
        double re = root.first;
        double im = root.second;
        double mag = qSqrt(re * re + im * im);
        double angle = qAtan2(im, re);

        /* 频率 = angle * sampleRate / (2*pi) */
        double freq = angle * m_sampleRate / (2.0 * M_PI);

        /* 带宽 = -ln(mag) * sampleRate / pi */
        double bw = -qLn(qMax(1e-10, mag)) * m_sampleRate / M_PI;

        /* 只保留语音频率范围内的共振峰(80~5500Hz) */
        if (freq > 80.0 && freq < 5500.0 && bw < 1000.0) {
            Formant f;
            f.frequency = freq;
            f.bandwidth = bw;
            f.amplitude = 1.0 / qMax(1e-6, bw);
            formants.append(f);
        }
    }

    return formants;
}

/** @brief 重置统计 */
void FormantTracker::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
