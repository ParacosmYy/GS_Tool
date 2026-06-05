/**
 * @file WalshHadamard.cpp
 * @brief Walsh-Hadamard变换实现 — 快速WHT+序列序重排+信号相关
 */

#include "utils/fft23/WalshHadamard.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
WalshHadamard::WalshHadamard(QObject* parent)
    : QObject(parent)
    , m_ordering(Ordering::Natural)
{
}

void WalshHadamard::setOrdering(Ordering ordering) { m_ordering = ordering; }

/**
 * @brief 正向Walsh-Hadamard变换
 * @param input 输入信号(长度必须为2的幂)
 * @return 变换系数
 */
QVector<double> WalshHadamard::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n < 2) return input;

    /* 补零到2的幂 */
    int size = 1;
    while (size < n) size *= 2;

    QVector<double> data(size, 0.0);
    for (int i = 0; i < n; ++i) data[i] = input[i];

    /* 执行快速WHT */
    fastWHT(data);

    /* 按请求的排序方式重排 */
    if (m_ordering == Ordering::Sequency) {
        reorderToSequency(data);
    } else if (m_ordering == Ordering::Dyadic) {
        reorderToDyadic(data);
    }

    /* 归一化: 1/sqrt(N) */
    double norm = 1.0 / qSqrt(static_cast<double>(size));
    for (auto& v : data) v *= norm;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += static_cast<quint64>(size);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(size);
    return data;
}

/**
 * @brief 逆向Walsh-Hadamard变换
 * @param coefficients WHT系数
 * @return 时域信号
 */
QVector<double> WalshHadamard::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    int n = coefficients.size();
    if (n < 2) return coefficients;

    /* 如果是序列序，先反排回自然序 */
    QVector<double> data = coefficients;
    if (m_ordering == Ordering::Sequency) {
        reorderToSequency(data);
    } else if (m_ordering == Ordering::Dyadic) {
        reorderToDyadic(data);
    }

    /* 逆归一化 */
    double norm = qSqrt(static_cast<double>(n));
    for (auto& v : data) v *= norm;

    /* WHT是自逆变换 */
    fastWHT(data);

    /* 再归一化 */
    norm = 1.0 / static_cast<double>(n);
    for (auto& v : data) v *= norm;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += static_cast<quint64>(n);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(n);
    return data;
}

/**
 * @brief 生成Hadamard矩阵
 * @param order 阶数(2^order)
 * @return Hadamard矩阵
 */
QVector<QVector<double>> WalshHadamard::hadamardMatrix(int order) const
{
    int n = 1 << qMax(1, order);
    QVector<QVector<double>> matrix(n, QVector<double>(n, 1.0));

    /* Sylvester构造法: 递归分块 */
    for (int size = 2; size <= n; size *= 2) {
        int half = size / 2;
        for (int r = 0; r < n; r += size) {
            for (int c = 0; c < n; c += size) {
                /* 右下块取负 */
                for (int i = 0; i < half; ++i) {
                    for (int j = 0; j < half; ++j) {
                        matrix[r + half + i][c + half + j] = -1.0;
                    }
                }
            }
        }
    }
    return matrix;
}

/**
 * @brief 生成指定索引的Walsh函数
 * @param index Walsh函数索引(序列序)
 * @param length 函数长度
 * @return Walsh函数采样值
 */
QVector<double> WalshHadamard::walshFunction(int index, int length) const
{
    int n = 1;
    while (n < length) n *= 2;

    QVector<double> walsh(n, 1.0);

    /* 通过Rademacher函数的乘积构造Walsh函数 */
    /* 序列序索引的Gray码确定Rademacher组合 */
    int bits = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; ++bits; }

    int gray = grayCode(index);
    for (int k = 0; k < n; ++k) {
        double val = 1.0;
        for (int bit = 0; bit < bits; ++bit) {
            if (gray & (1 << bit)) {
                /* Rademacher函数 r_{bit+1}(k) */
                int period = 1 << (bit + 1);
                int phase = k % period;
                val *= (phase < period / 2) ? 1.0 : -1.0;
            }
        }
        walsh[k] = val;
    }

    walsh.resize(length);
    return walsh;
}

/**
 * @brief 互相关分析(基于WHT)
 * @param a 信号A
 * @param b 信号B
 * @return 相关结果列表
 */
QList<WalshHadamard::CorrelationResult> WalshHadamard::correlate(
    const QVector<double>& a, const QVector<double>& b) const
{
    QList<CorrelationResult> results;
    int n = qMin(a.size(), b.size());
    if (n < 2) return results;

    int size = 1;
    while (size < 2 * n) size *= 2;

    /* 两个信号的WHT */
    QVector<double> fa(size, 0.0), fb(size, 0.0);
    for (int i = 0; i < n; ++i) { fa[i] = a[i]; fb[i] = b[i]; }

    /* 简化互相关: 时域滑动窗口 */
    for (int lag = -n / 2; lag < n / 2; ++lag) {
        double sum = 0.0;
        double normA = 0.0, normB = 0.0;
        for (int i = 0; i < n; ++i) {
            int j = i + lag;
            if (j >= 0 && j < n) {
                sum += a[i] * b[j];
                normA += a[i] * a[i];
                normB += b[j] * b[j];
            }
        }
        double denom = qSqrt(normA * normB);
        CorrelationResult r;
        r.lag = lag;
        r.value = (denom > 1e-10) ? sum / denom : 0.0;
        results.append(r);
    }
    return results;
}

/**
 * @brief 自相关分析
 * @param signal 输入信号
 * @return 自相关结果
 */
QList<WalshHadamard::CorrelationResult> WalshHadamard::autoCorrelate(
    const QVector<double>& signal) const
{
    return correlate(signal, signal);
}

/** @brief 重置统计信息 */
void WalshHadamard::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 快速Walsh-Hadamard变换(自然序)
 * @param data [in/out] 输入数据，就地变换
 */
void WalshHadamard::fastWHT(QVector<double>& data) const
{
    int n = data.size();
    if (n < 2) return;

    /* 蝶形运算: 类似FFT但无复数旋转因子 */
    for (int stride = 1; stride < n; stride *= 2) {
        for (int i = 0; i < n; i += stride * 2) {
            for (int j = 0; j < stride; ++j) {
                double a = data[i + j];
                double b = data[i + j + stride];
                data[i + j] = a + b;
                data[i + j + stride] = a - b;
            }
        }
    }
}

/**
 * @brief 自然序转序列序(Hadamard序转Walsh序)
 * @param data [in/out] 数据
 */
void WalshHadamard::reorderToSequency(QVector<double>& data) const
{
    int n = data.size();
    int bits = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; ++bits; }

    QVector<double> temp(n);
    for (int i = 0; i < n; ++i) {
        int seqIdx = sequencyIndex(i, bits);
        temp[seqIdx] = data[i];
    }
    data = temp;
}

/**
 * @brief 自然序转二进制序(Paley序)
 * @param data [in/out] 数据
 */
void WalshHadamard::reorderToDyadic(QVector<double>& data) const
{
    int n = data.size();
    int bits = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; ++bits; }

    QVector<double> temp(n);
    for (int i = 0; i < n; ++i) {
        temp[bitReverse(i, bits)] = data[i];
    }
    data = temp;
}

/**
 * @brief 位反转
 * @param value 输入值
 * @param bits 位宽
 * @return 反转后的值
 */
int WalshHadamard::bitReverse(int value, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

/**
 * @brief 计算Gray码
 * @param value 输入值
 * @return Gray码值
 */
int WalshHadamard::grayCode(int value) const
{
    return value ^ (value >> 1);
}

/**
 * @brief 自然序索引转序列序索引
 * @param index 自然序索引
 * @param bits 位宽
 * @return 序列序索引
 */
int WalshHadamard::sequencyIndex(int index, int bits) const
{
    /* 序列序 = Gray码位反转 */
    return bitReverse(grayCode(index), bits);
}
