#include "Expander3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态扩展器
 * @param parent 父对象指针
 */
Expander3::Expander3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置扩展阈值(dB)
 * @param thresholdDb 低于此值的信号将被扩展衰减
 */
void Expander3::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置扩展范围(dB)
 * @param rangeDb 最大扩展衰减量
 */
void Expander3::setRange(double rangeDb)
{
    m_range = qMax(0.0, rangeDb);
}

/**
 * @brief 线性幅度转分贝
 * @param linear 线性幅度
 * @return 分贝值
 */
static double linToDb(double linear)
{
    return 20.0 * std::log10(qMax(1e-10, std::abs(linear)));
}

/**
 * @brief 分贝转线性幅度
 * @param db 分贝值
 * @return 线性幅度
 */
static double dbToLin(double db)
{
    return std::pow(10.0, db / 20.0);
}

/**
 * @brief 对输入信号帧执行动态扩展
 *
 * 当信号电平低于阈值时，按扩展比率增大衰减量，
 * 使低电平信号更安静，扩展动态范围。
 *
 * @param input 输入信号帧
 * @return 扩展处理后的输出信号帧
 */
QVector<double> Expander3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return output;
    }

    output.resize(input.size());
    double attackCoeff = 1.0 - std::exp(-1.0 / (10.0 * 0.001 * 44100.0));
    double releaseCoeff = 1.0 - std::exp(-1.0 / (100.0 * 0.001 * 44100.0));
    double gainDb = 0.0;
    double ratio = 2.0; /* 扩展比率 */

    for (int i = 0; i < input.size(); ++i) {
        double inputDb = linToDb(input[i]);

        /* 计算目标增益 */
        double targetGain = 0.0;
        if (inputDb < m_threshold) {
            double underDb = m_threshold - inputDb;
            double expansion = underDb * (ratio - 1.0) / ratio;
            targetGain = -qMin(expansion, m_range);
        }

        /* 启动/释放平滑 */
        double coeff = (targetGain < gainDb) ? attackCoeff : releaseCoeff;
        gainDb += coeff * (targetGain - gainDb);

        /* 应用增益 */
        output[i] = input[i] * dbToLin(gainDb);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(output.size());
    return output;
}

/**
 * @brief 重置统计数据
 */
void Expander3::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
