/**
 * @file FractionalDelay.cpp
 * @brief 分数延迟滤波器实现 — Thiran全通插值 + 整数延迟线
 */

#include "utils/dsp8/FractionalDelay.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param order 阶数 @param maxDelay 最大延迟 @param parent 父对象 */
FractionalDelay::FractionalDelay(int order, int maxDelay, QObject* parent)
    : QObject(parent)
    , m_order(std::max(1, std::min(order, 8)))
    , m_maxDelay(std::max(1, maxDelay))
    , m_currentDelay(0.0)
    , m_intDelay(0)
    , m_fracDelay(0.0)
    , m_delayPos(0)
{
    /* 初始化延迟线和全通状态 */
    m_delayLine.assign(m_maxDelay + m_order + 1, 0.0);
    m_allpassState.assign(m_order + 1, 0.0);
    m_thiranA.resize(m_order + 1);
    computeThiranCoeffs(0.0, m_order);
    updateCoefficients();
}

/** @brief 计算Thiran全通系数 @param delay 分数延迟 @param order 阶数 @return 系数 */
std::vector<double> FractionalDelay::computeThiranCoeffs(double delay, int order)
{
    /* Thiran全通滤波器系数公式:
     * a_k = (-1)^k * C(N+1, k) * \prod_{m=0}^{N} (D - N + m) / (D - N + k + m)
     * 其中 D = 分数延迟, N = 阶数
     */
    std::vector<double> a(order + 1, 0.0);
    a[0] = 1.0;

    double D = delay + static_cast<double>(order) / 2.0;

    for (int k = 1; k <= order; ++k) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;

        /* 二项式系数 C(N+1, k) */
        double binom = 1.0;
        for (int i = 0; i < k; ++i) {
            binom *= static_cast<double>(order + 1 - i) / static_cast<double>(i + 1);
        }

        /* 乘积项 \prod_{m=0}^{N} (D-N+m)/(D-N+k+m) */
        double prod = 1.0;
        for (int m = 0; m <= order; ++m) {
            double numer = D - order + m;
            double denom = D - order + k + m;
            if (std::abs(denom) < 1e-12) {
                denom = 1e-12;
            }
            prod *= numer / denom;
        }

        a[k] = sign * binom * prod;
    }

    return a;
}

/** @brief 更新系数和延迟参数 */
void FractionalDelay::updateCoefficients()
{
    /* 计算整数和分数延迟部分 */
    double totalDelay = std::max(0.0, m_currentDelay);
    m_intDelay = static_cast<int>(std::floor(totalDelay));
    m_fracDelay = totalDelay - m_intDelay;

    /* 将分数延迟偏移以获得最佳Thiran近似 */
    /* Thiran滤波器在D ∈ [N/2 - 1, N/2 + 1]范围内精度最高 */
    double fracForThiran = m_fracDelay;

    /* 计算新的Thiran系数 */
    m_thiranA = computeThiranCoeffs(fracForThiran, m_order);
}

/** @brief 设置延迟 @param delay 延迟值 */
void FractionalDelay::setDelay(double delay)
{
    QElapsedTimer timer;
    timer.start();

    delay = std::max(0.0, std::min(delay, static_cast<double>(m_maxDelay)));
    if (std::abs(delay - m_currentDelay) < 1e-10) return;

    m_currentDelay = delay;
    updateCoefficients();

    m_stats.totalDelayChanges++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalSamplesProcessed + m_stats.totalDelayChanges;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit delayChanged(delay);
}

/** @brief 处理单个采样 @param input 输入 @return 延迟后的输出 */
double FractionalDelay::processOne(double input)
{
    QElapsedTimer timer;
    timer.start();

    /* Step 1: 写入整数延迟线 */
    m_delayLine[m_delayPos] = input;
    m_delayPos = (m_delayPos + 1) % static_cast<int>(m_delayLine.size());

    /* Step 2: 从整数延迟线读取(偏移整数延迟部分) */
    int readPos = m_delayPos - m_intDelay - m_order;
    if (readPos < 0) readPos += static_cast<int>(m_delayLine.size());
    double delayed = m_delayLine[readPos];

    /* Step 3: 通过Thiran全通滤波器进行分数延迟插值 */
    /* 全通滤波器: y[n] = a[N]*x[n] + \sum_{k=0}^{N-1} a[k]*(x[n-k] - y[n-N+k]) */
    /* 简化的一阶全通: y = -a * x_prev + x + a * y_prev */
    double allpassOut = delayed;
    for (int k = m_order; k >= 1; --k) {
        double x_n = allpassOut;
        double x_prev = m_allpassState[k - 1];
        double y_prev = m_allpassState[k];
        double a_k = m_thiranA[k];

        /* 一阶全通节: y[n] = a*x[n] + x[n-1] - a*y[n-1] */
        allpassOut = a_k * x_n + x_prev - a_k * y_prev;
        m_allpassState[k] = allpassOut;
    }
    /* 最后一个一阶节 */
    double x_0 = allpassOut;
    double x_prev0 = delayed;
    double y_prev0 = m_allpassState[0];
    allpassOut = m_thiranA[0] * x_0 + x_prev0 - m_thiranA[0] * y_prev0;

    /* 更新全通状态(移位) */
    for (int k = m_order; k >= 1; --k) {
        m_allpassState[k] = m_allpassState[k - 1];
    }
    m_allpassState[0] = delayed;

    m_stats.totalSamplesProcessed++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalSamplesProcessed + m_stats.totalDelayChanges;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return allpassOut;
}

/** @brief 处理QVector缓冲区 @param input 输入 @return 输出 */
QVector<double> FractionalDelay::process(const QVector<double>& input)
{
    QVector<double> output;
    output.reserve(input.size());
    for (double sample : input) {
        output.append(processOne(sample));
    }
    return output;
}

/** @brief 处理STL vector缓冲区 @param input 输入 @return 输出 */
std::vector<double> FractionalDelay::process(const std::vector<double>& input)
{
    std::vector<double> output;
    output.reserve(input.size());
    for (double sample : input) {
        output.push_back(processOne(sample));
    }
    return output;
}

/** @brief 重置滤波器状态 */
void FractionalDelay::reset()
{
    QElapsedTimer timer;
    timer.start();

    std::fill(m_delayLine.begin(), m_delayLine.end(), 0.0);
    std::fill(m_allpassState.begin(), m_allpassState.end(), 0.0);
    m_delayPos = 0;

    m_stats.totalResets++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalSamplesProcessed + m_stats.totalDelayChanges;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;
}

/** @brief 计算群延迟 @param normalizedFreq 归一化频率 @return 群延迟 */
double FractionalDelay::groupDelayAt(double normalizedFreq) const
{
    if (normalizedFreq <= 0.0 || normalizedFreq >= 0.5) {
        return m_currentDelay;
    }

    /* Thiran全通群延迟近似: D ≈ fracDelay + order/2 */
    /* 精确计算需要数值微分，此处使用近似 */
    double omega = 2.0 * M_PI * normalizedFreq;
    double gd = m_fracDelay + static_cast<double>(m_order) / 2.0;

    /* 一阶修正项 */
    double sum = 0.0;
    for (int k = 1; k <= m_order; ++k) {
        double a_k = m_thiranA[k];
        double arg = omega * k;
        double sinVal = std::sin(arg);
        double cosVal = std::cos(arg);
        /* 全通群延迟公式中的贡献 */
        double denom = 1.0 + a_k * a_k + 2.0 * a_k * cosVal;
        if (denom > 1e-12) {
            sum += k * (1.0 - a_k * a_k) * sinVal / denom;
        }
    }

    gd = static_cast<double>(m_intDelay) + gd - 2.0 * sum;
    return std::max(0.0, gd);
}

/** @brief 重置统计 */
void FractionalDelay::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
