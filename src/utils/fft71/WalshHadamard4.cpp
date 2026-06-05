/**
 * @file WalshHadamard4.cpp
 * @brief Walsh-Hadamard变换实现
 *
 * 实现快速Walsh-Hadamard变换(FWHT)，支持自然序和序列序
 * (Walsh序)变换，以及逆变换。适用于信号处理和纠错编码。
 */

#include "utils/fft71/WalshHadamard4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
WalshHadamard4::WalshHadamard4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换大小（必须是2的幂）
 * @param n 变换大小
 */
void WalshHadamard4::setSize(int n)
{
    // 向上取整到最近的2的幂
    int p = 1;
    while (p < n && p < 8192) p <<= 1;
    m_n = p;
}

/**
 * @brief 前向Walsh-Hadamard变换
 * @param data 输入数据（长度自动补齐到2的幂）
 * @return 变换结果
 */
QVector<double> WalshHadamard4::forward(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    int n = result.size();

    // 补齐到2的幂
    int p = 1;
    while (p < n) p <<= 1;
    if (p != n) {
        result.resize(p, 0.0);
        n = p;
    }

    fwht(result);

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n);
    return result;
}

/**
 * @brief 逆Walsh-Hadamard变换
 * @param data 输入变换系数
 * @return 逆变换结果
 */
QVector<double> WalshHadamard4::inverse(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    int n = result.size();

    int p = 1;
    while (p < n) p <<= 1;
    if (p != n) {
        result.resize(p, 0.0);
        n = p;
    }

    fwht(result);

    // 逆变换需要除以N
    for (double& v : result) v /= n;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return result;
}

/**
 * @brief 将结果转换为序列序（Walsh序）
 * @param data 自然序变换结果
 * @return 序列序排列的结果
 */
QVector<double> WalshHadamard4::sequencyOrder(const QVector<double>& data) const
{
    int n = data.size();
    QVector<int> perm = sequencyPermutation(n);
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        result[i] = data[perm[i]];
    }
    return result;
}

/**
 * @brief 重置统计信息
 */
void WalshHadamard4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 快速Walsh-Hadamard变换（原地计算）
 * @param data 输入输出数据
 *
 * 使用蝶形运算的FWHT，时间复杂度O(N*logN)。
 */
void WalshHadamard4::fwht(QVector<double>& data)
{
    int n = data.size();
    for (int h = 1; h < n; h <<= 1) {
        for (int i = 0; i < n; i += (h << 1)) {
            for (int j = i; j < i + h; ++j) {
                double x = data[j];
                double y = data[j + h];
                data[j] = x + y;
                data[j + h] = x - y;
            }
        }
    }
}

/**
 * @brief 生成序列序排列
 * @param n 变换大小
 * @return 排列索引向量
 *
 * 使用Gray码重排实现自然序到序列序的转换。
 */
QVector<int> WalshHadamard4::sequencyPermutation(int n) const
{
    QVector<int> perm(n);
    // 使用位反转+Gray码生成序列序
    int bits = 0;
    int temp = n;
    while (temp > 1) { bits++; temp >>= 1; }

    for (int i = 0; i < n; ++i) {
        // 位反转
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        // Gray码转换
        perm[i] = rev ^ (rev >> 1);
    }

    return perm;
}
