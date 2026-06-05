#include "Phaser6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化移相器处理器
 * @param parent 父对象指针
 */
Phaser6::Phaser6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Phaser6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置全通滤波器级联级数
 * @param stages 级数(2~12)
 */
void Phaser6::setStageCount(int stages)
{
    m_stages = qBound(2, stages, 12);
}

/**
 * @brief 设置LFO调制参数
 * @param rateHz LFO速率(Hz)
 * @param depth 调制深度(0.0~1.0)
 */
void Phaser6::setLFOParams(double rateHz, double depth)
{
    m_lfoRate = qMax(0.01, rateHz);
    m_lfoDepth = qBound(0.0, depth, 1.0);
}

/**
 * @brief 设置干湿混合比
 * @param mix 混合比(0.0=全干, 1.0=全湿)
 */
void Phaser6::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/**
 * @brief 对输入音频帧执行移相处理
 *
 * 处理流程：
 * 1. LFO产生调制信号（正弦波扫描）
 * 2. 调制信号控制级联全通滤波器的截止频率
 * 3. 全通输出与干信号混合产生梳状滤波效果
 * 4. 产生经典的相位抵消/增强效果
 *
 * 全通滤波器传递函数：y[n] = -c*x[n] + x[n-1] + c*y[n-1]
 * 其中c受LFO调制，在0~1之间变化。
 *
 * @param samples 输入音频帧
 * @return 移相处理后的音频帧
 */
QVector<double> Phaser6::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    if (n == 0) {
        emit processingCompleted(0);
        return output;
    }

    /* 全通滤波器状态 */
    QVector<double> apState(m_stages, 0.0);

    /* LFO相位（跨调用保持） */
    static double lfoPhase = 0.0;

    /* LFO相位增量 */
    const double phaseInc = 2.0 * M_PI * m_lfoRate / 44100.0;

    for (int i = 0; i < n; ++i) {
        /* 生成LFO调制信号 */
        double lfoValue = 0.5 * (1.0 + qSin(lfoPhase));
        lfoPhase += phaseInc;
        if (lfoPhase >= 2.0 * M_PI) lfoPhase -= 2.0 * M_PI;

        /* LFO映射到全通系数范围[0.1, 0.9] */
        double coeff = 0.1 + 0.8 * lfoValue * m_lfoDepth + 0.1 * (1.0 - m_lfoDepth);

        /* 级联全通滤波器 */
        double y = samples[i];
        for (int s = 0; s < m_stages; ++s) {
            double x = y;
            double prev = apState[s];
            apState[s] = y;
            /* 一阶全通：y[n] = c*(x[n] - y[n-1]) + x[n-1] */
            y = coeff * (x - prev) + prev;
        }

        /* 干湿混合 */
        output[i] = samples[i] * (1.0 - m_mix) + y * m_mix;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(n);
    return output;
}
