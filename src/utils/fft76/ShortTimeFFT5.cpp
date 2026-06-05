/**
 * @file ShortTimeFFT5.cpp
 * @brief Walsh-Hadamard变换实现
 *
 * 实现快速Walsh-Hadamard变换(FWHT)，支持自然序和序列序
 * (Walsh序)变换，以及逆变换。适用于信号处理和纠错编码。
 */

#include "utils/fft76/ShortTimeFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
ShortTimeFFT5::ShortTimeFFT5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换大小（必须是2的幂）
 * @param n 变换大小
 */
void ShortTimeFFT5::setSize(int n)
{
    /* 向上取整到最近的2的幂 */
    int p = 1;
    while (p < n && p < 8192) p <<= 1;
    m_n = p;
}

/**
 * @brief 前向Walsh-Hadamard变换
 * @param data 输入数据（长度自动补齐到2的幂）
 * @return 变换结果
 *
 * 执行Walsh-Hadamard变换，结果为自然序(Hadamard序)。
 * 变换后可用sequencyOrder()转换为序列序。
 */
QVector<double> ShortTimeFFT5::forward(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    int n = result.size();

    /* 补齐到2的幂 */
    int p = 1;
    while (p < n) p <<= 1;
    if (p != n) {
        result.resize(p, 0.0);
        n = p;
    }

    fwht(result);

    /* 更新统计信息 */
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
 *
 * 逆变换与前向变换结构相同，仅额外除以N进行归一化。
 */
QVector<double> ShortTimeFFT5::inverse(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    int n = result.size();

    /* 补齐到2的幂 */
    int p = 1;
    while (p < n) p <<= 1;
    if (p != n) {
        result.resize(p, 0.0);
        n = p;
    }

    /* 执行FWHT(正变换和逆变换结构相同) */
    fwht(result);

    /* 逆变换需要除以N进行归一化 */
    double norm = 1.0 / static_cast<double>(n);
    for (double& v : result) {
        v *= norm;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n);
    return result;
}

/**
 * @brief 将结果转换为序列序（Walsh序）
 * @param data 自然序变换结果
 * @return 序列序排列的结果
 *
 * 通过Gray码位反转将Hadamard序转换为Walsh序(序列序)，
 * 使变换系数按零交叉数排列。
 */
QVector<double> ShortTimeFFT5::sequencyOrder(const QVector<double>& data) const
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
void ShortTimeFFT5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 快速Walsh-Hadamard变换（原地计算）
 * @param data 输入输出数据
 *
 * 使用蝶形运算的FWHT，时间复杂度O(N*logN)。
 * 基本运算: x' = x + y, y' = x - y
 * 逐层递归分解，每层将数据分成两组进行蝶形运算。
 */
void ShortTimeFFT5::fwht(QVector<double>& data)
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
 * 先进行位反转，再应用Gray码变换。
 */
QVector<int> ShortTimeFFT5::sequencyPermutation(int n) const
{
    QVector<int> perm(n);
    int bits = 0;
    int temp = n;
    while (temp > 1) { bits++; temp >>= 1; }

    for (int i = 0; i < n; ++i) {
        /* 步骤1: 位反转 */
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        /* 步骤2: Gray码转换 */
        perm[i] = rev ^ (rev >> 1);
    }

    return perm;
}

/**
 * @brief 计算Walsh功率谱
 * @param data 输入数据
 * @return 功率谱（变换系数的平方）
 */
QVector<double> ShortTimeFFT5::powerSpectrum(const QVector<double>& data) const
{
    QVector<double> fwd = forward(data);
    for (double& v : fwd) v = v * v;
    return fwd;
}

/**
 * @brief 使用WHT进行自相关计算
 * @param data 输入信号
 * @return 自相关函数
 *
 * 利用Wiener-Khinchin定理：自相关 = IWHT(WHT(x)^2)
 */
QVector<double> ShortTimeFFT5::autoCorrelation(const QVector<double>& data) const
{
    // 前向变换
    QVector<double> fwd = forward(data);

    // 功率谱（取平方）
    for (double& v : fwd) v = v * v;

    // 逆变换得到自相关
    return inverse(fwd);
}

/**
 * @brief 计算序列的Walsh序号
 * @param data 输入序列
 * @return Walsh序号（过零次数）
 */
int ShortTimeFFT5::walshOrder(const QVector<double>& data) const
{
    if (data.size() < 2) return 0;
    int crossings = 0;
    for (int i = 1; i < data.size(); ++i) {
        if ((data[i] >= 0.0) != (data[i - 1] >= 0.0)) crossings++;
    }
    return crossings;
}
