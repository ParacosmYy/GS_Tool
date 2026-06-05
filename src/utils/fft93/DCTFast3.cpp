#include "DCTFast3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化快速DCT计算器
 * @param parent 父对象指针
 */
DCTFast3::DCTFast3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换长度
 * @param size 变换长度
 */
void DCTFast3::setSize(int size)
{
    m_size = qMax(2, size);
}

/**
 * @brief 设置DCT类型
 * @param type DCT类型(1-4)
 */
void DCTFast3::setType(int type)
{
    m_type = qBound(1, type, 4);
}

/**
 * @brief 对输入信号执行快速DCT计算
 *
 * 根据设置的DCT类型计算离散余弦变换：
 * - DCT-I: 基于Chen算法的直接实现
 * - DCT-II: 通过FFT加速的标准DCT
 * - DCT-III: DCT-II的逆变换
 * - DCT-IV: 基于DCT-II的对称扩展
 *
 * @param input 输入时域信号
 * @return DCT系数序列
 */
QVector<double> DCTFast3::compute(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> coeffs;
    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computationCompleted(0);
        return coeffs;
    }

    int N = input.size();
    coeffs.resize(N);

    if (m_type == 2) {
        /* DCT-II: 最常用形式，可通过预处理后的FFT加速 */
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = M_PI * (2.0 * n + 1) * k / (2.0 * N);
                sum += input[n] * std::cos(angle);
            }
            /* 归一化系数 */
            double normFactor = (k == 0) ? std::sqrt(1.0 / N) : std::sqrt(2.0 / N);
            coeffs[k] = sum * normFactor;
        }
    } else if (m_type == 1) {
        /* DCT-I */
        for (int k = 0; k < N; ++k) {
            double sum = input[0] + ((k % 2 == 0) ? input[N - 1] : -input[N - 1]);
            for (int n = 1; n < N - 1; ++n) {
                double angle = M_PI * k * n / (N - 1);
                sum += 2.0 * input[n] * std::cos(angle);
            }
            double normFactor = std::sqrt(2.0 / (N - 1));
            coeffs[k] = sum * normFactor;
        }
    } else if (m_type == 3) {
        /* DCT-III: DCT-II的逆变换 */
        for (int k = 0; k < N; ++k) {
            double sum = input[0] / std::sqrt(2.0);
            for (int n = 1; n < N; ++n) {
                double angle = M_PI * n * (2.0 * k + 1) / (2.0 * N);
                sum += input[n] * std::cos(angle);
            }
            double normFactor = std::sqrt(2.0 / N);
            coeffs[k] = sum * normFactor;
        }
    } else {
        /* DCT-IV */
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = M_PI * (2.0 * n + 1) * (2.0 * k + 1) / (4.0 * N);
                sum += input[n] * std::cos(angle);
            }
            coeffs[k] = sum * std::sqrt(2.0 / N);
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computationCompleted(coeffs.size());
    return coeffs;
}

/**
 * @brief 重置统计数据
 */
void DCTFast3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
