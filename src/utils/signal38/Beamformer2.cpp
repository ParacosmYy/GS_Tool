/**
 * @file Beamformer2.cpp
 * @brief 波束成形实现 - 延迟求和与MVDR波束形成器
 *
 * 延迟求和(Delay-Sum): 通过对阵元信号施加时延补偿，
 * 使目标方向信号同相叠加而噪声被抑制。
 * MVDR: 最小方差无失真响应波束形成器，在保持目标方向增益为1
 * 的约束下最小化输出功率。
 */

#include "utils/signal38/Beamformer2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数(4通道, 44100Hz)
 * @param parent 父QObject
 */
Beamformer2::Beamformer2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置通道数
 * @param n 麦克风通道数量，必须 >= 1
 */
void Beamformer2::setNumChannels(int n)
{
    m_channels = qMax(1, n);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void Beamformer2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置导向角度
 * @param degrees 目标信号入射角度(度)，0=正前方
 */
void Beamformer2::setSteeringAngle(double degrees)
{
    m_angle = degrees;
}

/**
 * @brief 设置声速
 * @param speed 声速(m/s)，默认343
 */
void Beamformer2::setSpeedOfSound(double speed)
{
    m_speed = qMax(1.0, speed);
}

/**
 * @brief 设置麦克风间距
 * @param meters 相邻麦克风间距(米)
 */
void Beamformer2::setMicSpacing(double meters)
{
    m_spacing = qMax(0.001, meters);
}

/**
 * @brief 计算每个通道的采样延迟数
 *
 * 对于线性阵列，第m个麦克风相对于参考点的延迟为:
 * tau(m) = m * d * sin(theta) / c
 *
 * @return 每个通道的延迟(采样点数)
 */
QVector<double> Beamformer2::computeDelays() const
{
    QVector<double> delays(m_channels, 0.0);
    double theta = qDegreesToRadians(m_angle);

    for (int m = 0; m < m_channels; ++m) {
        /* 线性阵列: 第m个阵元的位置 = m * spacing */
        double tau = m * m_spacing * qSin(theta) / m_speed;
        delays[m] = tau * m_sampleRate; /* 转为采样点数 */
    }

    /* 补偿使所有延迟非负 */
    double minDelay = *std::min_element(delays.begin(), delays.end());
    for (auto& d : delays)
        d -= minDelay;

    return delays;
}

/**
 * @brief 延迟求和波束形成
 *
 * 对每个通道信号施加分数延迟(线性插值)后叠加，
 * 目标方向信号同相叠加获得增益。
 *
 * @param frames 输入数据，每行一个通道的采样序列(长度必须一致)
 * @return 波束形成后的单通道输出
 */
QVector<double> Beamformer2::processDelaySum(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    const int ch = qMin(m_channels, frames.size());
    const int len = (ch > 0) ? frames[0].size() : 0;

    QVector<double> output(len, 0.0);

    if (ch < 1 || len < 1) {
        m_stats.totalFrames++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;
        emit beamformComplete(ch);
        return output;
    }

    QVector<double> delays = computeDelays();

    /* 延迟求和: 对每个采样点，对各通道施加延迟后叠加 */
    for (int i = 0; i < len; ++i) {
        double sum = 0.0;
        for (int c = 0; c < ch; ++c) {
            double delayedIdx = i - delays[c];
            /* 线性插值实现分数延迟 */
            if (delayedIdx < 0.0) {
                /* 延迟超出范围，补零 */
                continue;
            }
            int idx0 = static_cast<int>(delayedIdx);
            double frac = delayedIdx - idx0;

            double sample = 0.0;
            if (idx0 < len) {
                if (idx0 + 1 < len)
                    sample = frames[c][idx0] * (1.0 - frac) + frames[c][idx0 + 1] * frac;
                else
                    sample = frames[c][idx0] * (1.0 - frac);
            }
            sum += sample;
        }
        output[i] = sum / ch; /* 归一化 */
    }

    m_stats.totalFrames++;
    m_stats.totalChannelsProcessed += ch;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit beamformComplete(ch);
    return output;
}

/**
 * @brief MVDR波束形成
 *
 * 最小方差无失真响应(Minimum Variance Distortionless Response):
 * w = R^-1 * a / (a^H * R^-1 * a)
 * 其中R为协方差矩阵，a为导向向量。
 *
 * @param frames 输入数据，每行一个通道
 * @return 波束形成后的单通道输出
 */
QVector<double> Beamformer2::processMVDR(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    const int ch = qMin(m_channels, frames.size());
    const int len = (ch > 0) ? frames[0].size() : 0;

    QVector<double> output(len, 0.0);

    if (ch < 2 || len < ch) {
        /* 通道不足或数据太短，退化为延迟求和 */
        output = processDelaySum(frames);
        return output;
    }

    /* 第一步: 估计协方差矩阵 R */
    QVector<QVector<double>> R(ch, QVector<double>(ch, 0.0));
    for (int i = 0; i < len; ++i) {
        for (int m = 0; m < ch; ++m) {
            for (int n = 0; n < ch; ++n) {
                R[m][n] += frames[m][i] * frames[n][i];
            }
        }
    }
    for (int m = 0; m < ch; ++m)
        for (int n = 0; n < ch; ++n)
            R[m][n] /= len;

    /* 对角加载(正则化)防止奇异 */
    double diagLoad = 1e-6;
    for (int m = 0; m < ch; ++m)
        R[m][m] += diagLoad;

    /* 第二步: 构建导向向量 a */
    double theta = qDegreesToRadians(m_angle);
    QVector<double> a(ch, 0.0);
    for (int m = 0; m < ch; ++m) {
        double tau = m * m_spacing * qSin(theta) / m_speed;
        double phase = -2.0 * M_PI * tau * m_sampleRate / m_sampleRate;
        a[m] = qCos(phase);
    }

    /* 第三步: 求解 R^-1 * a (使用高斯消元) */
    /* 构建增广矩阵 [R | a] */
    QVector<QVector<double>> aug(ch, QVector<double>(ch + 1, 0.0));
    for (int m = 0; m < ch; ++m) {
        for (int n = 0; n < ch; ++n)
            aug[m][n] = R[m][n];
        aug[m][ch] = a[m];
    }

    /* 高斯消元 */
    for (int col = 0; col < ch; ++col) {
        /* 选主元 */
        int pivot = col;
        for (int row = col + 1; row < ch; ++row) {
            if (qAbs(aug[row][col]) > qAbs(aug[pivot][col]))
                pivot = row;
        }
        if (pivot != col) std::swap(aug[pivot], aug[col]);

        if (qAbs(aug[col][col]) < 1e-30) continue;

        /* 消元 */
        for (int row = col + 1; row < ch; ++row) {
            double factor = aug[row][col] / aug[col][col];
            for (int j = col; j <= ch; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    /* 回代 */
    QVector<double> RinvA(ch, 0.0);
    for (int i = ch - 1; i >= 0; --i) {
        double sum = aug[i][ch];
        for (int j = i + 1; j < ch; ++j)
            sum -= aug[i][j] * RinvA[j];
        RinvA[i] = (qAbs(aug[i][i]) > 1e-30) ? sum / aug[i][i] : 0.0;
    }

    /* 第四步: 计算 w = R^-1 * a / (a^T * R^-1 * a) */
    double denom = 0.0;
    for (int m = 0; m < ch; ++m)
        denom += a[m] * RinvA[m];

    QVector<double> w(ch, 0.0);
    if (qAbs(denom) > 1e-30) {
        for (int m = 0; m < ch; ++m)
            w[m] = RinvA[m] / denom;
    } else {
        /* 退化为均匀权重 */
        for (int m = 0; m < ch; ++m)
            w[m] = 1.0 / ch;
    }

    /* 第五步: 应用权重 */
    for (int i = 0; i < len; ++i) {
        double sum = 0.0;
        for (int c = 0; c < ch; ++c)
            sum += w[c] * frames[c][i];
        output[i] = sum;
    }

    m_stats.totalFrames++;
    m_stats.totalChannelsProcessed += ch;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit beamformComplete(ch);
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void Beamformer2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
