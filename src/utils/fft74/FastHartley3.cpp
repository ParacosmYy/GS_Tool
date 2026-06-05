/**
 * @file FastHartley3.cpp
 * @brief 快速Hartley变换(FHT)实现
 *
 * 实现离散Hartley变换(DHT)的快速算法，使用蝶形运算
 * 递归分解。DHT与DFT类似但使用cas函数(cos+sin)代替
 * 复数指数。支持卷积计算和自然序变换。
 */

#include "utils/fft74/FastHartley3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认变换大小为256点。
 */
FastHartley3::FastHartley3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换大小
 * @param n 变换大小，自动向上取整到2的幂
 */
void FastHartley3::setSize(int n)
{
    int p = 1;
    while (p < n && p < 16384) p <<= 1;
    m_n = p;
}

/**
 * @brief 前向Hartley变换
 * @param data 输入数据，长度自动补齐到2的幂
 * @return 变换结果(Hartley域系数)
 *
 * DHT定义: H[k] = sum_{n=0}^{N-1} x[n] * cas(2*pi*n*k/N)
 * 其中 cas(t) = cos(t) + sin(t)
 */
QVector<double> FastHartley3::forward(const QVector<double>& data)
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

    /* 执行快速Hartley变换 */
    fht(result);

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
 * @brief 逆Hartley变换
 * @param data 输入Hartley域系数
 * @return 时域信号
 *
 * 逆变换与前向变换结构相同，仅除以N归一化。
 * 这是因为DHT具有自逆性: IDHT(x) = DHT(x) / N
 */
QVector<double> FastHartley3::inverse(const QVector<double>& data)
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

    /* 执行FHT(自逆性) */
    fht(result);

    /* 归一化 */
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
 * @brief 使用FHT计算循环卷积
 * @param a 第一个序列
 * @param b 第二个序列
 * @return 卷积结果
 *
 * 利用DHT的卷积定理:
 * conv(a,b)[n] = IDHT(H_a[k]*H_b[k] + H_a[N-k]*H_b[k] + H_a[k]*H_b[N-k] - H_a[N-k]*H_b[N-k]) / 2
 * 简化为直接在Hartley域的计算。
 */
QVector<double> FastHartley3::convolve(const QVector<double>& a, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMax(a.size(), b.size());
    int p = 1;
    while (p < 2 * n) p <<= 1;

    /* 补零到相同长度 */
    QVector<double> fa(p, 0.0), fb(p, 0.0);
    for (int i = 0; i < a.size(); ++i) fa[i] = a[i];
    for (int i = 0; i < b.size(); ++i) fb[i] = b[i];

    /* 前向FHT */
    fht(fa);
    fht(fb);

    /* Hartley域卷积 */
    QVector<double> result(p, 0.0);
    for (int k = 0; k < p; ++k) {
        int kRev = (k == 0) ? 0 : p - k;
        /* 卷积公式: (Ha[k]*Hb[k] + Ha[kRev]*Hb[k] + Ha[k]*Hb[kRev] - Ha[kRev]*Hb[kRev]) / 2 */
        result[k] = (fa[k] * fb[k] + fa[kRev] * fb[k] + fa[k] * fb[kRev] - fa[kRev] * fb[kRev]) / 2.0;
    }

    /* 逆FHT */
    fht(result);
    double norm = 1.0 / static_cast<double>(p);
    for (double& v : result) v *= norm;

    /* 截取有效长度 */
    result.resize(n > 0 ? 2 * n - 1 : 0);

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += p;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return result;
}

/**
 * @brief 重置统计信息
 */
void FastHartley3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 快速Hartley变换(原地计算)
 * @param data 输入输出数据
 *
 * 使用蝶形运算的FHT，时间复杂度O(N*logN)。
 * 基本蝶形运算:
 *   x'[i]   = x[i] + x[i+h]
 *   x'[i+h] = x[i] * cas(2*pi*j/h) + x[i+h] * cas(-2*pi*j/h)
 * 其中 cas(t) = cos(t) + sin(t)
 *
 * 逐层分解，每层步长加倍。使用旋转因子cas(2*pi*j/h)。
 */
void FastHartley3::fht(QVector<double>& data)
{
    int n = data.size();

    for (int h = 1; h < n; h <<= 1) {
        double angle = M_PI / h;
        double cosA = qCos(angle);
        double sinA = qSin(angle);

        for (int i = 0; i < n; i += (h << 1)) {
            for (int j = 0; j < h; ++j) {
                double x = data[i + j];
                double y = data[i + j + h];

                /* 蝶形运算 */
                double casVal = cosA * (2 * j + 1);
                double casNeg = -sinA * (2 * j + 1);

                data[i + j] = x + y;
                data[i + j + h] = x - y;

                /* 应用旋转因子(仅内层需要) */
                if (j > 0 && h > 1) {
                    double rotAngle = 2.0 * M_PI * j / (h << 1);
                    double casRot = qCos(rotAngle) + qSin(rotAngle);
                    double casRotNeg = qCos(rotAngle) - qSin(rotAngle);
                    data[i + j + h] = x * casRot + y * casRotNeg;
                }
            }
        }
    }
}
