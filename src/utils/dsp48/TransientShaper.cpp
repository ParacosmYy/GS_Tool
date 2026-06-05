/**
 * @file TransientShaper.cpp
 * @brief 瞬态塑形器 — 攻击增强+平滑控制 实现
 *
 * 通过双包络检测器（快/慢）之间的差值检测瞬态成分，
 * 然后根据攻击/持续增益参数对瞬态进行增强或抑制。
 * 支持多通道处理、包络输出和峰值统计。
 * 所有处理均带统计追踪和 QElapsedTimer 计时。
 */

#include "dsp48/TransientShaper.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 *
 * 初始化采样率为 44100Hz，攻击增益 +6dB，持续增益 0dB，
 * 灵敏度 0.5，包络状态归零。
 *
 * @param parent 父QObject
 */
TransientShaper::TransientShaper(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_attack(6.0)
    , m_sustain(0.0)
    , m_sensitivity(0.5)
    , m_fastEnv(0.0)
    , m_slowEnv(0.0)
{
}

/**
 * @brief 设置采样率
 *
 * 采样率影响包络时间常数的计算。更改采样率不会重置内部
 * 包络状态，但会在下一次 process() 调用时使用新的系数。
 *
 * @param sampleRate 采样率（Hz），最小值 1.0
 */
void TransientShaper::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置攻击增益
 *
 * 正值增强瞬态（使鼓声更"尖锐"），负值抑制瞬态（使鼓声更"柔和"）。
 * 典型范围：-20dB ~ +20dB。
 *
 * @param attackDb 攻击增益（dB），正值增强瞬态，负值抑制
 */
void TransientShaper::setAttack(double attackDb)
{
    m_attack = attackDb;
}

/**
 * @brief 设置持续增益
 *
 * 控制非瞬态部分（持续音、尾音）的增益。
 * 正值增加持续音量，负值减小。
 *
 * @param sustainDb 持续增益（dB），影响非瞬态部分
 */
void TransientShaper::setSustain(double sustainDb)
{
    m_sustain = sustainDb;
}

/**
 * @brief 设置瞬态检测灵敏度
 *
 * 灵敏度越高，越容易检测到微弱的瞬态。
 * 0.0 = 仅检测最强瞬态，1.0 = 检测几乎所有瞬态。
 *
 * @param sensitivity 灵敏度 [0.0, 1.0]
 */
void TransientShaper::setSensitivity(double sensitivity)
{
    m_sensitivity = qBound(0.0, sensitivity, 1.0);
}

/**
 * @brief 处理输入信号，进行瞬态塑形
 *
 * 完整算法流程：
 * 1. 计算输入信号的绝对值包络
 * 2. 使用两个不同时间常数的包络跟踪器：
 *    - 快包络（~5ms）跟踪快速瞬态
 *    - 慢包络（~50ms）跟踪平均能量
 * 3. 瞬态强度 = 快包络 - 慢包络（正值部分）
 * 4. 归一化瞬态强度并与阈值比较
 * 5. 在攻击增益和持续增益之间平滑插值
 * 6. 应用增益曲线到原始信号
 *
 * 时间常数采用非对称 attack/release 设计：
 * attack 系数较大（快速响应），release 系数较小（缓慢恢复），
 * 确保瞬态检测的时序精度。
 *
 * 增益插值使用 smoothstep 函数避免硬切换噪声。
 *
 * @param input 输入音频采样序列
 * @return 经过瞬态塑形后的采样序列
 */
QVector<double> TransientShaper::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return {};

    m_transientEnv.resize(n);
    QVector<double> output(n);

    /* 包络时间常数：快包络 ~5ms，慢包络 ~50ms */
    const double fastTimeConstant = 0.005;
    const double slowTimeConstant = 0.050;
    const double fastCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * fastTimeConstant));
    const double slowCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * slowTimeConstant));

    /* 攻击/持续的线性增益（dB 到线性转换） */
    const double attackLin = qPow(10.0, m_attack / 20.0);
    const double sustainLin = qPow(10.0, m_sustain / 20.0);

    /* 检测阈值：灵敏度越高，阈值越低，检测越敏感 */
    const double threshold = 0.01 * (1.0 - m_sensitivity * 0.9);

    /* attack/release 非对称系数 */
    const double fastAttackCoeff = fastCoeff * 3.0;   /* 快速攻击 */
    const double fastReleaseCoeff = fastCoeff * 0.5;  /* 较慢释放 */
    const double slowAttackCoeff = slowCoeff;          /* 标准攻击 */
    const double slowReleaseCoeff = slowCoeff * 0.3;   /* 慢释放 */

    int detectedCount = 0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);

        /* 快包络更新（attack/release 非对称） */
        if (absSample > m_fastEnv) {
            m_fastEnv += fastAttackCoeff * (absSample - m_fastEnv);
        } else {
            m_fastEnv += fastReleaseCoeff * (absSample - m_fastEnv);
        }

        /* 慢包络更新（attack/release 非对称） */
        if (absSample > m_slowEnv) {
            m_slowEnv += slowAttackCoeff * (absSample - m_slowEnv);
        } else {
            m_slowEnv += slowReleaseCoeff * (absSample - m_slowEnv);
        }

        /* 瞬态强度 = 快慢包络差的正值部分 */
        double transient = m_fastEnv - m_slowEnv;
        transient = qMax(0.0, transient);
        m_transientEnv[i] = transient;

        /* 归一化瞬态强度（相对于慢包络） */
        double maxEnv = qMax(m_slowEnv, 1e-10);
        double normTransient = transient / maxEnv;

        /* 计算增益曲线 */
        double gain = sustainLin;
        if (normTransient > threshold) {
            /* 线性映射到 [0, 1] */
            double t = qBound(0.0, (normTransient - threshold) / (1.0 - threshold + 1e-10), 1.0);
            /* smoothstep 平滑插值：避免增益硬切换产生噪声 */
            double smoothT = t * t * (3.0 - 2.0 * t);
            /* 在持续增益和攻击增益之间插值 */
            gain = sustainLin + smoothT * (attackLin - sustainLin);

            /* 发射瞬态检测信号（限流：最多100次） */
            if (smoothT > 0.5 && detectedCount < 100) {
                emit transientDetected(static_cast<double>(i), smoothT);
                detectedCount++;
                m_stats.transientCount++;
            }
        }

        output[i] = input[i] * gain;
    }

    /* 统计更新 */
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    return output;
}

/**
 * @brief 重置所有统计数据和内部状态
 *
 * 清空包络状态（快/慢包络归零）、瞬态包络缓存，
 * 以及所有累计统计计数器。
 */
void TransientShaper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_fastEnv = 0.0;
    m_slowEnv = 0.0;
    m_transientEnv.clear();
}
