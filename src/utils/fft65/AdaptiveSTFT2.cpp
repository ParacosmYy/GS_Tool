/**
 * @file AdaptiveSTFT2.cpp
 * @brief 自适应短时傅里叶变换实现（第2版）
 *
 * 根据信号局部特性自动选择窗口大小的STFT变体。
 * 支持能量模式和过零率模式的自适应窗口选择策略。
 * 前向变换生成多帧频谱数据，逆变换重建时域信号。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft65/AdaptiveSTFT2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化自适应STFT处理器
 * @param parent 父QObject对象指针
 */
AdaptiveSTFT2::AdaptiveSTFT2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最小窗口大小
 * @param n 最小窗口长度（采样点数），必须为2的幂
 */
void AdaptiveSTFT2::setMinWindowSize(int n)
{
    m_minWin = qMax(16, n);
}

/**
 * @brief 设置最大窗口大小
 * @param n 最大窗口长度（采样点数），必须为2的幂
 */
void AdaptiveSTFT2::setMaxWindowSize(int n)
{
    m_maxWin = qMax(m_minWin, n);
}

/**
 * @brief 设置自适应模式
 * @param mode 自适应策略："energy"基于能量, "zcr"基于过零率, "hybrid"混合模式
 */
void AdaptiveSTFT2::setAdaptationMode(const QString& mode)
{
    if (mode == "energy" || mode == "zcr" || mode == "hybrid") {
        m_mode = mode;
    }
}

/**
 * @brief 根据帧的局部特征选择窗口大小
 *
 * 能量模式：能量高则使用短窗口以捕捉瞬态
 * 过零率模式：过零率高使用短窗口（高频内容丰富）
 * 混合模式：综合两者
 *
 * @param frame 待分析的信号帧
 * @return 选择的窗口大小
 */
int AdaptiveSTFT2::chooseWindowSize(const QVector<double>& frame)
{
    if (frame.isEmpty()) return m_minWin;

    /* 计算帧能量 */
    double energy = 0.0;
    for (double s : frame) {
        energy += s * s;
    }
    energy /= frame.size();

    /* 计算过零率 */
    int zcr = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) {
            zcr++;
        }
    }
    double zcrRate = static_cast<double>(zcr) / frame.size();

    /* 归一化指标 */
    double normEnergy = qMin(1.0, energy * 100.0);
    double normZcr = qMin(1.0, zcrRate * 10.0);

    double indicator = 0.0;
    if (m_mode == "energy") {
        indicator = normEnergy;
    } else if (m_mode == "zcr") {
        indicator = normZcr;
    } else {
        /* hybrid */
        indicator = 0.5 * normEnergy + 0.5 * normZcr;
    }

    /* indicator越高 → 窗口越小（瞬态信号需要更高时间分辨率） */
    double ratio = 1.0 - qBound(0.0, indicator, 1.0);

    /* 映射到窗口大小范围 */
    double winLog = qLn(m_minWin) + ratio * (qLn(m_maxWin) - qLn(m_minWin));
    int winSize = static_cast<int>(qExp(winLog));

    /* 对齐到最近的2的幂 */
    int pow2 = 1;
    while (pow2 < winSize && pow2 < m_maxWin) pow2 *= 2;
    winSize = pow2;
    winSize = qBound(m_minWin, winSize, m_maxWin);

    return winSize;
}

/**
 * @brief 执行自适应前向STFT
 *
 * 将信号分段，每段根据局部特性选择不同窗口大小，
 * 然后对每段进行DFT分析。
 *
 * @param signal 输入时域信号
 * @return 每帧的频谱数据（幅度谱）
 */
QVector<QVector<double>> AdaptiveSTFT2::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> frames;
    m_winSizes.clear();

    if (signal.isEmpty()) {
        emit transformCompleted(0, 0);
        return frames;
    }

    int pos = 0;
    int totalFrames = 0;
    int winSum = 0;

    while (pos < signal.size()) {
        /* 确定当前分析帧 */
        int remaining = signal.size() - pos;
        int lookAhead = qMin(m_maxWin, remaining);
        QVector<double> probeFrame(lookAhead);
        for (int i = 0; i < lookAhead; ++i) {
            probeFrame[i] = signal[pos + i];
        }

        /* 自适应选择窗口大小 */
        int winSize = chooseWindowSize(probeFrame);
        winSize = qMin(winSize, remaining);
        m_winSizes.append(winSize);

        /* 提取帧并加Hann窗 */
        QVector<double> frame(winSize, 0.0);
        for (int i = 0; i < winSize; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (winSize - 1)));
            frame[i] = signal[pos + i] * w;
        }

        /* 计算DFT幅度谱 */
        int N = winSize;
        QVector<double> spectrum(N / 2 + 1, 0.0);
        for (int k = 0; k <= N / 2; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = 2.0 * M_PI * k * n / N;
                re += frame[n] * qCos(angle);
                im -= frame[n] * qSin(angle);
            }
            spectrum[k] = qSqrt(re * re + im * im);
        }

        frames.append(spectrum);
        winSum += winSize;
        totalFrames++;

        /* 50%重叠移动 */
        pos += winSize / 2;
        if (winSize / 2 == 0) pos++;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalFrames += totalFrames;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    int avgWin = totalFrames > 0 ? winSum / totalFrames : 0;
    emit transformCompleted(totalFrames, avgWin);
    return frames;
}

/**
 * @brief 执行逆STFT，从频谱帧重建时域信号
 *
 * 使用重叠相加法将不同窗口大小的帧拼接为连续信号。
 *
 * @param frames 前向变换产生的频谱帧集合
 * @return 重建的时域信号
 */
QVector<double> AdaptiveSTFT2::inverse(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> signal;

    if (frames.isEmpty() || m_winSizes.isEmpty()) {
        return signal;
    }

    /* 计算输出长度 */
    int totalLen = 0;
    for (int i = 0; i < m_winSizes.size() && i < frames.size(); ++i) {
        if (i == 0) {
            totalLen += m_winSizes[i];
        } else {
            totalLen += m_winSizes[i] / 2;
        }
    }
    signal.resize(totalLen, 0.0);

    int pos = 0;
    for (int f = 0; f < frames.size() && f < m_winSizes.size(); ++f) {
        int N = m_winSizes[f];
        const auto& spectrum = frames[f];

        /* 从幅度谱重建时域帧（近似逆DFT，忽略相位） */
        QVector<double> frame(N, 0.0);
        for (int n = 0; n < N; ++n) {
            double val = 0.0;
            for (int k = 0; k <= N / 2 && k < spectrum.size(); ++k) {
                double angle = 2.0 * M_PI * k * n / N;
                val += spectrum[k] * qCos(angle) / N;
            }
            /* Hann窗归一化 */
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
            frame[n] = val / (w + 1e-10);
        }

        /* 重叠相加 */
        for (int i = 0; i < N && (pos + i) < signal.size(); ++i) {
            signal[pos + i] += frame[i];
        }

        if (f == 0) {
            pos += N;
        } else {
            pos += N / 2;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalTransforms);

    return signal;
}

/**
 * @brief 获取当前统计信息
 * @return 变换统计结构
 */
AdaptiveSTFT2::Stats AdaptiveSTFT2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void AdaptiveSTFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
