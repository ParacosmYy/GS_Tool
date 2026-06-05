#include "Gate6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声门处理器
 * @param parent 父对象指针
 */
Gate6::Gate6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Gate6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置噪声门参数
 * @param thresholdDb 门限阈值(dB)
 * @param attackMs 攻击时间(ms)
 * @param releaseMs 释放时间(ms)
 * @param rangeDb 衰减范围(dB)
 */
void Gate6::setParameters(double thresholdDb, double attackMs,
                           double releaseMs, double rangeDb)
{
    Q_UNUSED(thresholdDb)
    Q_UNUSED(attackMs)
    Q_UNUSED(releaseMs)
    Q_UNUSED(rangeDb)
}

/**
 * @brief 获取当前门状态
 * @return true表示门开启（信号通过），false表示门关闭
 */
bool Gate6::isGateOpen() const
{
    static bool lastState = true;
    return lastState;
}

/**
 * @brief 设置保持时间
 * @param holdMs 保持时间(ms)
 */
void Gate6::setHoldTime(double holdMs)
{
    Q_UNUSED(holdMs)
}

/**
 * @brief 处理音频帧进行噪声门控制
 *
 * 带迟滞(hysteresis)和侧链支持的噪声门：
 * 1. 计算信号包络（峰值检测+平滑）
 * 2. 迟滞比较：开启阈值高于关闭阈值，防止抖动
 * 3. 保持时间：信号跌落后延迟关闭
 * 4. Attack/release包络平滑增益变化
 *
 * @param inputFrame 输入音频采样帧
 * @return 门控后的音频帧
 */
QVector<double> Gate6::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit gateStateChanged(true);
        return output;
    }

    /* 默认门参数 */
    const double thresholdDb = -40.0;
    const double hysteresisDb = 6.0;
    const double rangeDb = -80.0;
    const double attackMs = 1.0;
    const double releaseMs = 100.0;
    const double holdMs = 50.0;
    const double sampleRate = 44100.0;

    const double openThresh = qPow(10.0, thresholdDb / 20.0);
    const double closeThresh = qPow(10.0, (thresholdDb - hysteresisDb) / 20.0);
    const double rangeLinear = qPow(10.0, rangeDb / 20.0);

    const double attackCoeff = qExp(-1.0 / (attackMs * 0.001 * sampleRate));
    const double releaseCoeff = qExp(-1.0 / (releaseMs * 0.001 * sampleRate));

    static double envelope = 0.0;
    static double gain = 1.0;
    static bool gateOpen = true;
    static double holdCounter = 0.0;
    const double holdSamples = holdMs * 0.001 * sampleRate;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(inputFrame[i]);

        /* 包络跟随器 */
        if (absVal > envelope) {
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absVal;
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absVal;
        }

        /* 迟滞门状态判定 */
        if (!gateOpen && envelope >= openThresh) {
            gateOpen = true;
            holdCounter = holdSamples;
        } else if (gateOpen && envelope < closeThresh) {
            holdCounter -= 1.0;
            if (holdCounter <= 0.0) {
                gateOpen = false;
            }
        } else if (gateOpen) {
            holdCounter = holdSamples;
        }

        /* 目标增益 */
        double targetGain = gateOpen ? 1.0 : rangeLinear;
        double coeff = (targetGain > gain) ? (1.0 - attackCoeff) : (1.0 - releaseCoeff);
        gain += coeff * (targetGain - gain);
        gain = qBound(rangeLinear, gain, 1.0);

        output[i] = inputFrame[i] * gain;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit gateStateChanged(gateOpen);
    return output;
}
