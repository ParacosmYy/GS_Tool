#include "EnvelopeDetect10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化包络检测器
 * @param parent 父对象指针
 */
EnvelopeDetect10::EnvelopeDetect10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void EnvelopeDetect10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置包络平滑滤波器截止频率
 * @param cutoffHz 截止频率(Hz)
 * @param sampleRate 采样率(Hz)
 */
void EnvelopeDetect10::setSmoothing(double cutoffHz, double sampleRate)
{
    Q_UNUSED(cutoffHz)
    Q_UNUSED(sampleRate)
}

/**
 * @brief 使用Hilbert变换提取信号包络
 *
 * 通过FFT构造解析信号并取模值：
 * 1. 计算输入信号的DFT
 * 2. 正频率分量加倍，负频率分量置零
 * 3. 逆DFT得到解析信号
 * 4. 取解析信号模值作为瞬时包络
 *
 * @param signal 输入音频信号
 * @return 包络幅度序列
 */
QVector<double> EnvelopeDetect10::hilbertEnvelope(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    QVector<double> envelope(n, 0.0);
    if (n < 4) {
        emit detectionCompleted(0);
        return envelope;
    }

    /* DFT */
    QVector<double> reFreq(n, 0.0), imFreq(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double rSum = 0.0, iSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            rSum += signal[i] * qCos(angle);
            iSum += signal[i] * qSin(angle);
        }
        reFreq[k] = rSum;
        imFreq[k] = iSum;
    }

    /* 构造解析信号频谱 */
    for (int k = 1; k < n / 2; ++k) {
        reFreq[k] *= 2.0;
        imFreq[k] *= 2.0;
    }
    for (int k = n / 2 + 1; k < n; ++k) {
        reFreq[k] = 0.0;
        imFreq[k] = 0.0;
    }

    /* 逆DFT */
    for (int i = 0; i < n; ++i) {
        double rSum = 0.0, iSum = 0.0;
        for (int k = 0; k < n; ++k) {
            double angle = 2.0 * M_PI * k * i / n;
            rSum += reFreq[k] * qCos(angle) - imFreq[k] * qSin(angle);
            iSum += reFreq[k] * qSin(angle) + imFreq[k] * qCos(angle);
        }
        envelope[i] = qSqrt((rSum / n) * (rSum / n) + (iSum / n) * (iSum / n));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(n);
    return envelope;
}

/**
 * @brief 使用峰值检测法提取包络
 *
 * 攻击-释放(Attack-Release)包络跟踪器：
 * - 信号上升时使用快攻击时间常数快速响应
 * - 信号下降时使用慢释放时间常数平滑衰减
 *
 * @param signal 输入信号
 * @param attackMs 攻击时间(ms)
 * @param releaseMs 释放时间(ms)
 * @param sampleRate 采样率(Hz)
 * @return 包络序列
 */
QVector<double> EnvelopeDetect10::peakEnvelope(const QVector<double>& signal,
                                                double attackMs,
                                                double releaseMs,
                                                double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    QVector<double> envelope(n, 0.0);
    if (n == 0 || sampleRate <= 0) {
        emit detectionCompleted(0);
        return envelope;
    }

    const double attackCoeff = qExp(-1.0 / (attackMs * sampleRate / 1000.0));
    const double releaseCoeff = qExp(-1.0 / (releaseMs * sampleRate / 1000.0));

    double env = 0.0;
    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(signal[i]);
        if (absVal > env) {
            env = attackCoeff * env + (1.0 - attackCoeff) * absVal;
        } else {
            env = releaseCoeff * env + (1.0 - releaseCoeff) * absVal;
        }
        envelope[i] = env;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(n);
    return envelope;
}

/**
 * @brief 使用RMS方法提取包络
 *
 * 滑动窗口RMS计算：
 * RMS[i] = sqrt(sum(x[i-w:i]^2) / w)
 *
 * @param signal 输入信号
 * @param windowSize RMS窗口大小(采样点数)
 * @return RMS包络序列
 */
QVector<double> EnvelopeDetect10::rmsEnvelope(const QVector<double>& signal,
                                               int windowSize)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    QVector<double> envelope(n, 0.0);
    if (n == 0) {
        emit detectionCompleted(0);
        return envelope;
    }

    windowSize = qBound(1, windowSize, n);
    const int halfWin = windowSize / 2;

    /* 初始窗口累积 */
    double sumSq = 0.0;
    for (int i = 0; i < qMin(halfWin, n); ++i) {
        sumSq += signal[i] * signal[i];
    }

    for (int i = 0; i < n; ++i) {
        /* 添加右侧样本 */
        int right = i + halfWin;
        if (right < n) sumSq += signal[right] * signal[right];

        /* 移除左侧样本 */
        int left = i - halfWin - 1;
        if (left >= 0) sumSq -= signal[left] * signal[left];

        int count = qMin(i + halfWin, n - 1) - qMax(i - halfWin, 0) + 1;
        count = qMax(1, count);
        envelope[i] = qSqrt(qMax(0.0, sumSq) / count);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(n);
    return envelope;
}
