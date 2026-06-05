/**
 * @file StereoEnhance2.cpp
 * @brief 立体声增强处理器实现
 *
 * 实现立体声宽度控制、声像调节和低频单声道混合功能。
 * 通过Mid/Side处理实现精确的立体声场控制。
 */

#include "utils/dsp68/StereoEnhance2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
StereoEnhance2::StereoEnhance2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param w 宽度因子，0.0=单声道，1.0=原始，>1.0=增强
 */
void StereoEnhance2::setWidth(double w)
{
    m_width = qBound(0.0, w, 3.0);
}

/**
 * @brief 设置声像位置
 * @param pan 声像值，-1.0=全左，0.0=中央，1.0=全右
 */
void StereoEnhance2::setPan(double pan)
{
    m_pan = qBound(-1.0, pan, 1.0);
}

/**
 * @brief 设置低频单声道混合截止频率
 * @param freq 截止频率(Hz)，低于此频率的信号合并为单声道
 */
void StereoEnhance2::setBassMonoFreq(double freq)
{
    m_bassFreq = qBound(20.0, freq, 500.0);
}

/**
 * @brief 处理立体声音频数据
 * @param input 输入数据，input[0]=左声道，input[1]=右声道
 * @return 处理后的立体声数据
 */
QVector<QVector<double>> StereoEnhance2::process(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> output;
    if (input.size() < 2 || input[0].isEmpty()) return output;

    const int N = input[0].size();
    output.resize(2);
    output[0].resize(N);
    output[1].resize(N);

    // Mid/Side变换并应用宽度
    // M = (L+R)/sqrt(2), S = (L-R)/sqrt(2)
    // L' = (M + width*S)/sqrt(2), R' = (M - width*S)/sqrt(2)
    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = (i < input[1].size()) ? input[1][i] : 0.0;

        // 计算Mid和Side
        double M = (L + R) * M_SQRT1_2;
        double S = (L - R) * M_SQRT1_2;

        // 应用宽度：缩放Side分量
        S *= m_width;

        // 重建左右声道
        double newL = (M + S) * M_SQRT1_2;
        double newR = (M - S) * M_SQRT1_2;

        // 应用声像（constant power pan law）
        double panAngle = (m_pan + 1.0) * 0.25 * M_PI;
        double panL = qCos(panAngle);
        double panR = qSin(panAngle);
        newL *= panL;
        newR *= panR;

        output[0][i] = newL;
        output[1][i] = newR;
    }

    // 低频单声道化：简化一阶低通滤波器提取低频
    double alpha = qExp(-2.0 * M_PI * m_bassFreq / 44100.0);
    double bassL = 0.0, bassR = 0.0;
    for (int i = 0; i < N; ++i) {
        double bassMono = (output[0][i] + output[1][i]) * 0.5;
        bassL = alpha * bassL + (1.0 - alpha) * output[0][i];
        bassR = alpha * bassR + (1.0 - alpha) * output[1][i];
        double monoBass = (bassL + bassR) * 0.5;
        output[0][i] = output[0][i] - bassL + monoBass;
        output[1][i] = output[1][i] - bassR + monoBass;
    }

    // 计算相关系数
    double sumXY = 0.0, sumXX = 0.0, sumYY = 0.0;
    for (int i = 0; i < N; ++i) {
        double L = output[0][i];
        double R = output[1][i];
        sumXY += L * R;
        sumXX += L * L;
        sumYY += R * R;
    }
    double denom = qSqrt(sumXX * sumYY);
    m_corr = (denom > 1e-12) ? sumXY / denom : 0.0;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_width);
    return output;
}

/**
 * @brief 重置统计信息
 */
void StereoEnhance2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
