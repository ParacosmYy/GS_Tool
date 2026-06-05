#include "ConstantQ7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化常数Q变换引擎
 * @param parent 父对象指针
 */
ConstantQ7::ConstantQ7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ConstantQ7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置频率范围
 * @param minFreq 最低频率(Hz)
 * @param maxFreq 最高频率(Hz)
 */
void ConstantQ7::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = qMax(1.0, minFreq);
    m_maxFreq = qMax(m_minFreq + 1.0, maxFreq);
}

/**
 * @brief 设置每八度频率箱数
 * @param bpo 每八度频率箱数
 */
void ConstantQ7::setBinsPerOctave(int bpo)
{
    m_binsPerOctave = qMax(1, bpo);
}

/**
 * @brief 计算指定频率点的窗函数DFT
 *
 * 对输入信号施加Hann窗后计算单个频率点的DFT。
 *
 * @param input 输入信号
 * @param freq 目标频率(Hz)
 * @param windowSize 窗口长度
 * @return 复数DFT值
 */
static QPair<double, double> windowedDFT(const QVector<double>& input,
                                          double freq, int windowSize)
{
    const int N = qMin(windowSize, input.size());
    double re = 0.0, im = 0.0;
    for (int n = 0; n < N; ++n) {
        double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * n / N));
        double angle = -2.0 * M_PI * freq * n / N;
        re += hann * input[n] * qCos(angle);
        im += hann * input[n] * qSin(angle);
    }
    return {re, im};
}

/**
 * @brief 对输入信号执行CQT变换
 *
 * 计算对数分布的中心频率，对每个频段应用对应长度的
 * 窗函数DFT，实现恒Q值的频谱分析。
 *
 * @param input 输入时域信号
 * @return 复数频谱系数
 */
QVector<QPair<double, double>> ConstantQ7::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    if (input.isEmpty()) {
        emit transformCompleted(0);
        return result;
    }

    /* Q因子：所有频段的Q值恒定 */
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);

    /* 计算总频段数 */
    int numOctaves = static_cast<int>(qCeil(qLn(m_maxFreq / m_minFreq) / qLn(2.0)));
    int totalBins = numOctaves * m_binsPerOctave;

    result.reserve(totalBins);
    for (int k = 0; k < totalBins; ++k) {
        /* 中心频率按对数分布 */
        double centerFreq = m_minFreq * qPow(2.0, static_cast<double>(k) / m_binsPerOctave);
        if (centerFreq > m_maxFreq) break;

        /* 窗口长度与频率成反比（恒Q特性） */
        int windowSize = qMax(1, static_cast<int>(qRound(Q * input.size() * 2.0 * centerFreq / (centerFreq + m_maxFreq))));

        auto complex = windowedDFT(input, centerFreq, windowSize);
        result.append(complex);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(result.size());
    return result;
}
