#include "Tonality6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音调性分析器
 * @param parent 父对象指针
 */
Tonality6::Tonality6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Tonality6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算音调性指标
 *
 * 通过自相关函数分析信号的周期性：
 * 1. 计算信号的自相关函数
 * 2. 寻找第一个显著峰值（排除零延迟）
 * 3. 峰值与零延迟值的比率即为音调性
 * 4. 值域[0, 1]，1表示纯音调，0表示纯噪声
 *
 * @param signal 输入音频信号
 * @return 音调性值 [0, 1]
 */
double Tonality6::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    if (n < 16) {
        emit analysisCompleted(0);
        return 0.0;
    }

    /* 计算自相关，最大延迟取信号长度的一半 */
    int maxLag = n / 2;
    QVector<double> acf = autocorrelation(signal, maxLag);

    if (acf.isEmpty() || acf[0] < 1e-20) {
        emit analysisCompleted(0);
        return 0.0;
    }

    /* 寻找第一个显著峰（排除零延迟附近的区域） */
    int searchStart = qMax(2, n / 20);
    double maxPeak = 0.0;
    for (int i = searchStart; i < maxLag; ++i) {
        if (acf[i] > maxPeak) {
            maxPeak = acf[i];
        }
    }

    /* 归一化得到音调性 */
    double tonality = qBound(0.0, maxPeak / acf[0], 1.0);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    int tonalComponents = (tonality > 0.5) ? 1 : 0;
    emit analysisCompleted(tonalComponents);
    return tonality;
}

/**
 * @brief 检测信号中的音调频率
 *
 * 通过自相关函数的峰值位置确定信号的基频：
 * 1. 计算自相关函数
 * 2. 寻找所有显著峰值
 * 3. 将峰值位置转换为频率
 *
 * @param signal 输入音频信号
 * @param sampleRate 采样率(Hz)
 * @return 检测到的音调频率列表(Hz)
 */
QVector<double> Tonality6::detectTonalFrequencies(
    const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> frequencies;
    const int n = signal.size();
    if (n < 16 || sampleRate <= 0) {
        emit analysisCompleted(0);
        return frequencies;
    }

    int maxLag = n / 2;
    QVector<double> acf = autocorrelation(signal, maxLag);

    if (acf.isEmpty() || acf[0] < 1e-20) {
        emit analysisCompleted(0);
        return frequencies;
    }

    /* 归一化自相关 */
    for (int i = 0; i < acf.size(); ++i) {
        acf[i] /= acf[0];
    }

    /* 寻找峰值：超过阈值且大于相邻值 */
    const double peakThreshold = 0.3;
    int searchStart = qMax(2, static_cast<int>(sampleRate / 5000.0));

    for (int i = searchStart; i < maxLag - 1; ++i) {
        if (acf[i] > peakThreshold
            && acf[i] >= acf[i - 1]
            && acf[i] >= acf[i + 1]) {
            double freq = sampleRate / i;
            if (freq >= 20.0 && freq <= sampleRate / 2.0) {
                frequencies.append(freq);
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    emit analysisCompleted(frequencies.size());
    return frequencies;
}

/**
 * @brief 计算自相关函数
 *
 * 标准自相关计算：R(tau) = sum(x[i]*x[i+tau])
 *
 * @param signal 输入信号
 * @param maxLag 最大延迟点数
 * @return 自相关值序列
 */
QVector<double> Tonality6::autocorrelation(const QVector<double>& signal, int maxLag)
{
    const int n = signal.size();
    maxLag = qMin(maxLag, n);
    QVector<double> acf(maxLag, 0.0);

    for (int tau = 0; tau < maxLag; ++tau) {
        double sum = 0.0;
        for (int i = 0; i < n - tau; ++i) {
            sum += signal[i] * signal[i + tau];
        }
        acf[tau] = sum;
    }

    return acf;
}
