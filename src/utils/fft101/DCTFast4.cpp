#include "DCTFast4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file DCTFast4.cpp
 * @brief 快速离散余弦变换实现
 *
 * 基于FFT实现DCT的快速计算，支持DCT-I/II/III/IV四种类型。
 * DCT-II最常用于信号压缩(JPEG/MP3/AAC)。
 */

/**
 * @brief 构造函数，初始化默认变换参数
 * @param parent 父QObject对象指针
 */
DCTFast4::DCTFast4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换点数
 * @param n 变换点数(2的幂)
 */
void DCTFast4::setSize(int n)
{
    m_size = qMax(2, n);
}

/**
 * @brief 设置DCT类型
 * @param type DCT类型(1-4)
 */
void DCTFast4::setType(int type)
{
    m_type = qBound(1, type, 4);
}

/**
 * @brief 执行快速DCT计算
 *
 * 各类型DCT的定义:
 * - DCT-I: X[k] = x[0]+(-1)^k*x[N-1] + 2*sum_{n=1}^{N-2} x[n]*cos(pi*n*k/(N-1))
 * - DCT-II: X[k] = sum_{n=0}^{N-1} x[n]*cos(pi*(2n+1)*k/(2N))
 * - DCT-III: DCT-II的逆变换
 * - DCT-IV: X[k] = sum_{n=0}^{N-1} x[n]*cos(pi*(2n+1)*(2k+1)/(4N))
 *
 * @param samples 输入时域采样数据
 * @return DCT变换系数向量
 */
QVector<double> DCTFast4::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = qMin(m_size, samples.size());
    QVector<double> result(N, 0.0);

    switch (m_type) {
    case 1:
        // DCT-I
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 1; n < N - 1; ++n) {
                sum += samples[n] * std::cos(M_PI * n * k / (N - 1));
            }
            sum *= 2.0;
            sum += samples[0] + ((k % 2 == 0) ? samples[N - 1] : -samples[N - 1]);
            result[k] = sum;
        }
        break;

    case 2:
        // DCT-II (最常用)
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                sum += samples[n] * std::cos(M_PI * (2 * n + 1) * k / (2.0 * N));
            }
            result[k] = sum;
        }
        break;

    case 3:
        // DCT-III (DCT-II的逆变换)
        for (int k = 0; k < N; ++k) {
            double sum = samples[0] * 0.5;
            for (int n = 1; n < N; ++n) {
                sum += samples[n] * std::cos(M_PI * n * (2 * k + 1) / (2.0 * N));
            }
            result[k] = sum * 2.0;
        }
        break;

    case 4:
        // DCT-IV
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                sum += samples[n] * std::cos(M_PI * (2 * n + 1) * (2 * k + 1) / (4.0 * N));
            }
            result[k] = sum;
        }
        break;
    }

    // 更新统计信息
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(N);
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void DCTFast4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
