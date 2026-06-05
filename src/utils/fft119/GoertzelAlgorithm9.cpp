#include "GoertzelAlgorithm9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Goertzel v9引擎
 * @param parent 父对象指针
 */
GoertzelAlgorithm9::GoertzelAlgorithm9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GoertzelAlgorithm9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Goertzel二阶IIR核心计算
 *
 * 对输入信号在指定频率bin上运行二阶递归滤波器，
 * 复杂度O(N)，仅需3个状态变量。
 *
 * @param signal 输入信号
 * @param bin DFT频率bin索引
 * @return 复数DFT值 (实部, 虚部)
 */
static QPair<double, double> goertzelCore(const QVector<double>& signal, int bin)
{
    const int N = signal.size();
    if (N == 0) return {0.0, 0.0};

    double coeff = 2.0 * qCos(2.0 * M_PI * bin / N);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        double s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double re = s1 - s2 * qCos(2.0 * M_PI * bin / N);
    double im = s2 * qSin(2.0 * M_PI * bin / N);
    return {re, im};
}

/**
 * @brief 计算指定频率的DFT值
 *
 * 将目标频率映射到DFT bin索引后执行Goertzel滤波。
 *
 * @param signal 输入时域信号
 * @param targetFreqHz 目标频率(Hz)
 * @param sampleRate 采样率(Hz)
 * @return 复数DFT值 (实部, 虚部)
 */
QPair<double, double> GoertzelAlgorithm9::compute(
    const QVector<double>& signal, double targetFreqHz, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    if (N == 0 || sampleRate <= 0.0) return {0.0, 0.0};

    int bin = qRound(targetFreqHz * N / sampleRate);
    bin = qBound(0, bin, N - 1);
    auto result = goertzelCore(signal, bin);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(1);
    return result;
}

/**
 * @brief 批量计算多个目标频率的DFT值
 *
 * 对每个目标频率独立运行Goertzel滤波器，
 * 总复杂度O(K*N)，K为频率数。
 *
 * @param signal 输入信号
 * @param targetFreqs 目标频率列表(Hz)
 * @param sampleRate 采样率(Hz)
 * @return 各频率的复数DFT值
 */
QVector<QPair<double, double>> GoertzelAlgorithm9::computeMultiFreq(
    const QVector<double>& signal, const QVector<double>& targetFreqs, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> results;
    const int N = signal.size();
    if (N == 0 || targetFreqs.isEmpty() || sampleRate <= 0.0) {
        emit computationCompleted(0);
        return results;
    }

    results.reserve(targetFreqs.size());
    for (double freq : targetFreqs) {
        int bin = qRound(freq * N / sampleRate);
        bin = qBound(0, bin, N - 1);
        results.append(goertzelCore(signal, bin));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(results.size());
    return results;
}

/**
 * @brief 计算指定频率的功率
 *
 * 使用Goertzel算法计算频率bin的复数DFT值，取功率谱值。
 *
 * @param signal 输入信号
 * @param targetFreqHz 目标频率(Hz)
 * @param sampleRate 采样率(Hz)
 * @return 功率值
 */
double GoertzelAlgorithm9::computePower(
    const QVector<double>& signal, double targetFreqHz, double sampleRate)
{
    const int N = signal.size();
    if (N == 0 || sampleRate <= 0.0) return 0.0;

    int bin = qRound(targetFreqHz * N / sampleRate);
    bin = qBound(0, bin, N - 1);
    auto c = goertzelCore(signal, bin);
    return c.first * c.first + c.second * c.second;
}

/**
 * @brief 检测信号中是否存在指定频率
 *
 * 计算目标频率的功率，与信号总功率比较，判断是否超过阈值。
 *
 * @param signal 输入信号
 * @param targetFreqHz 目标频率(Hz)
 * @param sampleRate 采样率(Hz)
 * @param thresholdDb 检测阈值(dB)
 * @return 是否检测到该频率
 */
bool GoertzelAlgorithm9::detectTone(const QVector<double>& signal,
    double targetFreqHz, double sampleRate, double thresholdDb)
{
    const int N = signal.size();
    if (N == 0 || sampleRate <= 0.0) return false;

    /* 计算目标频率功率 */
    double power = computePower(signal, targetFreqHz, sampleRate);

    /* 计算信号总功率 */
    double totalPower = 0.0;
    for (int i = 0; i < N; ++i)
        totalPower += signal[i] * signal[i];

    if (totalPower < 1e-30) return false;

    /* 转换为dB并比较 */
    double powerDb = 10.0 * qLn(power / totalPower + 1e-30) / qLn(10.0);
    return powerDb > thresholdDb;
}
