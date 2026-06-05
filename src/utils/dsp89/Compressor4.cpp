#include "Compressor4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态压缩器
 * @param parent 父对象指针
 */
Compressor4::Compressor4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置压缩阈值(dB)
 * @param thresholdDb 阈值电平，超过此值的信号将被压缩
 */
void Compressor4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置压缩比
 * @param ratio 压缩比率，值越大压缩越强(如4:1)
 */
void Compressor4::setRatio(double ratio)
{
    m_ratio = qMax(1.0, ratio);
}

/**
 * @brief 设置启动时间(ms)
 * @param attackMs 启动时间常数，控制压缩响应速度
 */
void Compressor4::setAttack(double attackMs)
{
    m_attack = qMax(0.1, attackMs);
}

/**
 * @brief 将线性幅度转换为分贝值
 * @param linear 线性幅度值
 * @return 分贝值
 */
static double linearToDb(double linear)
{
    return 20.0 * std::log10(qMax(1e-10, std::abs(linear)));
}

/**
 * @brief 将分贝值转换为线性幅度
 * @param db 分贝值
 * @return 线性幅度
 */
static double dbToLinear(double db)
{
    return std::pow(10.0, db / 20.0);
}

/**
 * @brief 对输入信号帧执行动态压缩
 *
 * 基于峰值检测计算增益缩减量，应用启动/释放平滑，
 * 对超过阈值的信号部分按压缩比进行增益衰减。
 *
 * @param input 输入信号帧
 * @return 压缩后的输出信号帧
 */
QVector<double> Compressor4::process(const QVector<double>& input)
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
    double attackCoeff = 1.0 - std::exp(-1.0 / (m_attack * 0.001 * 44100.0));
    double releaseCoeff = 1.0 - std::exp(-1.0 / (50.0 * 0.001 * 44100.0));
    double gainDb = 0.0;

    for (int i = 0; i < input.size(); ++i) {
        /* 计算输入信号电平(dB) */
        double inputDb = linearToDb(input[i]);

        /* 计算目标增益缩减量 */
        double targetGain = 0.0;
        if (inputDb > m_threshold) {
            double overDb = inputDb - m_threshold;
            targetGain = -overDb * (1.0 - 1.0 / m_ratio);
        }

        /* 启动/释放平滑 */
        double coeff = (targetGain < gainDb) ? attackCoeff : releaseCoeff;
        gainDb += coeff * (targetGain - gainDb);

        /* 应用增益 */
        output[i] = input[i] * dbToLinear(gainDb);
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
void Compressor4::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
