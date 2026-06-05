#include "GoertzelAlgorithm8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Goertzel算法引擎
 * @param parent 父对象指针
 */
GoertzelAlgorithm8::GoertzelAlgorithm8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GoertzelAlgorithm8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置目标检测频率列表
 * @param frequencies 频率列表(Hz)
 */
void GoertzelAlgorithm8::setTargetFrequencies(const QVector<double>& frequencies)
{
    m_targetFreqs = frequencies;
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void GoertzelAlgorithm8::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief Goertzel二阶IIR滤波核心
 *
 * 对输入信号在指定频率bin上运行二阶递归滤波器，
 * 最后一步提取复数DFT值。
 *
 * @param input 输入时域信号
 * @param bin DFT频率bin索引
 * @return 复数DFT值 (实部, 虚部)
 */
static QPair<double, double> goertzelFilter(const QVector<double>& input, int bin)
{
    const int N = input.size();
    if (N == 0) return {0.0, 0.0};

    double coeff = 2.0 * qCos(2.0 * M_PI * bin / N);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = input[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 提取复数结果 */
    double re = s1 - s2 * qCos(2.0 * M_PI * bin / N);
    double im = s2 * qSin(2.0 * M_PI * bin / N);
    return {re, im};
}

/**
 * @brief 对输入信号计算所有目标频率的幅值和相位
 *
 * 对每个目标频率，将其映射到DFT bin索引后执行Goertzel滤波。
 *
 * @param input 输入时域信号
 * @return 各目标频率的 (幅值, 相位) 对
 */
QVector<QPair<double, double>> GoertzelAlgorithm8::compute(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int N = input.size();
    if (N == 0 || m_targetFreqs.isEmpty()) {
        emit computationCompleted(0);
        return result;
    }

    result.reserve(m_targetFreqs.size());
    for (double freq : m_targetFreqs) {
        double bin = freq * N / m_sampleRate;
        auto complex = goertzelFilter(input, qRound(bin));
        double magnitude = qSqrt(complex.first * complex.first + complex.second * complex.second);
        double phase = qAtan2(complex.second, complex.first);
        result.append({magnitude, phase});
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(result.size());
    return result;
}

/**
 * @brief 单频点快速检测
 *
 * 仅计算指定频率的幅值，跳过相位计算，适用于DTMF等实时场景。
 *
 * @param input 输入时域信号
 * @param targetFreq 目标频率(Hz)
 * @return 幅值
 */
double GoertzelAlgorithm8::computeSingle(const QVector<double>& input, double targetFreq)
{
    const int N = input.size();
    if (N == 0) return 0.0;

    double bin = targetFreq * N / m_sampleRate;
    int binIdx = qRound(bin);
    binIdx = qBound(0, binIdx, N - 1);

    double coeff = 2.0 * qCos(2.0 * M_PI * binIdx / N);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        double s0 = input[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double re = s1 - s2 * qCos(2.0 * M_PI * binIdx / N);
    double im = s2 * qSin(2.0 * M_PI * binIdx / N);
    return qSqrt(re * re + im * im);
}
