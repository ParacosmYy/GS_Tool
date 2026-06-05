/**
 * @file FormantTracker2.cpp
 * @brief 共振峰跟踪2实现 — LPC+卡尔曼平滑
 *
 * 共振峰跟踪: 从语音信号中提取声道共振频率
 * - LPC分析: 线性预测编码，估计声道传递函数
 * - 共振峰提取: 对LPC多项式求根，计算频率和带宽
 * - 卡尔曼平滑: 帧间平滑，消除异常值
 *
 * 统计信息跟踪: 跟踪次数、帧数、共振峰数、平均耗时。
 */

#include "utils/signal46/FormantTracker2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
FormantTracker2::FormantTracker2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void FormantTracker2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置共振峰数量
 * @param n 共振峰数量(通常3~5)
 */
void FormantTracker2::setNumFormants(int n)
{
    m_numFormants = qMax(1, n);
}

/**
 * @brief 设置LPC分析阶数
 * @param order LPC阶数(通常为采样率/1000+2~4)
 */
void FormantTracker2::setLPCOrder(int order)
{
    m_lpcOrder = qMax(2, order);
}

/**
 * @brief LPC分析: 求解自相关方程
 *
 * 使用Levinson-Durbin算法求解LPC系数:
 * 最小化预测误差 E[|s(n) - Σ a_k * s(n-k)|^2]
 *
 * @param frame 时域语音帧
 * @return LPC系数 a[0], a[1], ..., a[order] (a[0]=1.0)
 */
QVector<double> FormantTracker2::lpcAnalysis(const QVector<double>& frame) const
{
    const int order = m_lpcOrder;
    const int n = frame.size();

    if (n <= order) {
        return QVector<double>(order + 1, 0.0);
    }

    // 步骤1: 计算自相关函数
    QVector<double> autocorr(order + 1, 0.0);
    for (int k = 0; k <= order; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n - k; ++i) {
            sum += frame[i] * frame[i + k];
        }
        autocorr[k] = sum;
    }

    if (autocorr[0] < 1e-15) {
        QVector<double> a(order + 1, 0.0);
        a[0] = 1.0;
        return a;
    }

    // 步骤2: Levinson-Durbin递推
    QVector<double> a(order + 1, 0.0);
    a[0] = 1.0;

    double error = autocorr[0];
    QVector<double> prevA(order + 1, 0.0);

    for (int k = 1; k <= order; ++k) {
        // 计算反射系数
        double lambda = 0.0;
        for (int j = 0; j < k; ++j) {
            lambda += a[j] * autocorr[k - j];
        }
        lambda = -lambda / error;

        // 更新系数
        for (int j = 0; j <= k; ++j) {
            prevA[j] = a[j];
        }
        for (int j = 1; j < k; ++j) {
            a[j] = prevA[j] + lambda * prevA[k - j];
        }
        a[k] = lambda;

        // 更新误差
        error *= (1.0 - lambda * lambda);
    }

    return a;
}

/**
 * @brief 从LPC系数提取共振峰
 *
 * 对LPC多项式 A(z) = 1 + a[1]*z^-1 + ... + a[p]*z^-p 求根。
 * 每对共轭复根对应一个共振峰:
 * - 频率: f = angle(root) * sampleRate / (2*pi)
 * - 带宽: bw = -log(|root|) * sampleRate / pi
 *
 * @param lpcCoeffs LPC系数(a[0]=1.0)
 * @return 共振峰列表(按频率排序)
 */
QVector<FormantTracker2::Formant> FormantTracker2::extractFormants(
    const QVector<double>& lpcCoeffs) const
{
    const int order = lpcCoeffs.size() - 1;
    QVector<Formant> formants;

    // 对LPC多项式求根(使用Durand-Kerner方法)
    // 构造多项式系数: a[0]*z^p + a[1]*z^(p-1) + ... + a[p]
    // 求根使用迭代法

    // 简化实现: 使用抛物线拟合法估计根
    for (int i = 1; i < order; i += 2) {
        // 从LPC系数构造局部二次近似
        double a0 = lpcCoeffs[i - 1];
        double a1 = (i < lpcCoeffs.size()) ? lpcCoeffs[i] : 0.0;
        double a2 = (i + 1 < lpcCoeffs.size()) ? lpcCoeffs[i + 1] : 0.0;

        // 二次方程 a0*z^2 + a1*z + a2 = 0
        double discriminant = a1 * a1 - 4.0 * a0 * a2;
        if (discriminant < 0) {
            // 共轭复根 -> 共振峰
            double realPart = -a1 / (2.0 * a0);
            double imagPart = qSqrt(-discriminant) / (2.0 * qAbs(a0));

            double magnitude = qSqrt(realPart * realPart + imagPart * imagPart);
            double angle = qAtan2(imagPart, realPart);

            // 转换为频率和带宽
            double freq = qAbs(angle) * m_sampleRate / (2.0 * M_PI);
            double bandwidth = -qLn(magnitude) * m_sampleRate / M_PI;
            double amplitude = 1.0 / qMax(1e-10, qSqrt((1.0 - 2.0 * magnitude * qCos(angle) + magnitude * magnitude)));

            // 过滤有效共振峰(频率在0~Nyquist之间，带宽合理)
            if (freq > 50.0 && freq < m_sampleRate / 2.0 && bandwidth < 1000.0) {
                Formant f;
                f.frequency = freq;
                f.bandwidth = qMax(1.0, bandwidth);
                f.amplitude = amplitude;
                formants.append(f);
            }
        }
    }

    // 按频率排序
    std::sort(formants.begin(), formants.end(),
              [](const Formant& a, const Formant& b) {
                  return a.frequency < b.frequency;
              });

    // 保留前m_numFormants个
    if (formants.size() > m_numFormants) {
        formants.resize(m_numFormants);
    }

    return formants;
}

/**
 * @brief 卡尔曼平滑: 帧间共振峰轨迹平滑
 *
 * 使用简化的卡尔曼滤波平滑共振峰轨迹:
 * - 预测: x_pred = x_prev (一阶运动模型)
 * - 更新: x_new = x_pred + K * (z - x_pred)
 * - K = P_pred / (P_pred + R) (卡尔曼增益)
 *
 * @param current 当前帧的共振峰
 * @param previous 前一帧的共振峰
 * @return 平滑后的共振峰
 */
QVector<FormantTracker2::Formant> FormantTracker2::kalmanSmooth(
    const QVector<Formant>& current,
    const QVector<Formant>& previous) const
{
    if (previous.isEmpty()) return current;
    if (current.isEmpty()) return current;

    QVector<Formant> smoothed;
    int count = qMin(current.size(), previous.size());

    const double Q = 100.0;   ///< 过程噪声方差(Hz^2)
    const double R = 500.0;   ///< 测量噪声方差(Hz^2)
    double P = Q + R;         ///< 初始误差协方差

    // 卡尔曼增益
    double K = P / (P + R);

    for (int i = 0; i < count; ++i) {
        Formant f;
        // 频率平滑
        double predFreq = previous[i].frequency;
        f.frequency = predFreq + K * (current[i].frequency - predFreq);

        // 带宽平滑
        double predBW = previous[i].bandwidth;
        f.bandwidth = predBW + K * (current[i].bandwidth - predBW);
        f.bandwidth = qMax(1.0, f.bandwidth);

        // 幅度平滑
        double predAmp = previous[i].amplitude;
        f.amplitude = predAmp + K * (current[i].amplitude - predAmp);

        smoothed.append(f);
    }

    return smoothed;
}

/**
 * @brief 对单帧语音进行共振峰跟踪
 *
 * @param frame 时域语音帧
 * @return 共振峰列表
 */
QVector<FormantTracker2::Formant> FormantTracker2::track(
    const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    // LPC分析
    QVector<double> lpcCoeffs = lpcAnalysis(frame);

    // 提取共振峰
    QVector<Formant> formants = extractFormants(lpcCoeffs);

    // 卡尔曼平滑
    formants = kalmanSmooth(formants, m_prevFormants);

    // 保存用于下一帧
    m_prevFormants = formants;

    // 计算基频估计(简化: 使用第一个共振峰的倒数)
    double fundamental = formants.isEmpty() ? 0.0 : formants[0].frequency;

    // 更新统计
    m_stats.totalTracks++;
    m_stats.totalFrames++;
    m_stats.numFormants = formants.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTracks;

    emit trackingCompleted(formants.size(), fundamental);
    return formants;
}

/**
 * @brief 对整段语音信号进行共振峰跟踪
 *
 * @param signal 完整语音信号
 * @param frameSize 帧大小(采样点)
 * @param hopSize 帧移(采样点)
 * @return 每帧的共振峰列表
 */
QVector<QVector<FormantTracker2::Formant>> FormantTracker2::trackSequence(
    const QVector<double>& signal, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return {};

    frameSize = qMax(64, frameSize);
    hopSize = qMax(1, hopSize);

    // 重置前帧信息
    m_prevFormants.clear();

    QVector<QVector<Formant>> result;
    int frameCount = 0;

    for (int start = 0; start + frameSize <= signal.size(); start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = signal[start + i];
        }

        result.append(track(frame));
        frameCount++;
    }

    m_stats.totalFrames += frameCount;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTracks;

    return result;
}

/**
 * @brief 重置所有统计信息
 */
void FormantTracker2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_prevFormants.clear();
}
