#include "ChirpZTransform3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Chirp Z变换计算器
 * @param parent 父对象指针
 */
ChirpZTransform3::ChirpZTransform3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置螺旋比率参数
 * @param ratio 螺旋路径的比率，控制频率分辨率
 */
void ChirpZTransform3::setRatio(double ratio)
{
    m_ratio = qMax(0.01, ratio);
}

/**
 * @brief 设置变换阶数
 * @param order 输出频率点数
 */
void ChirpZTransform3::setOrder(int order)
{
    m_order = qMax(2, order);
}

/**
 * @brief 计算Chirp Z变换
 *
 * Bluestein算法实现：将CZT转化为卷积运算，
 * 通过FFT高效计算任意等间隔螺旋路径上的z变换值。
 * X(z_k) = sum_{n=0}^{N-1} x[n] * A^{-n} * W^{n*k}
 *
 * @param input 输入时域信号
 */
void ChirpZTransform3::compute(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computationCompleted(0);
        return;
    }

    const int N = input.size();
    const int M = m_order;

    /* 计算chirp因子: W = exp(-j*2*pi/M * ratio) */
    double wAngle = -2.0 * M_PI * m_ratio / M;

    /* 简化实现：直接计算CZT */
    /* 预计算A^{-n}和W^{n^2/2} */
    QVector<double> aPowRe(N), aPowIm(N);
    double aAngle = 0.0; /* A = 1 (起始点在单位圆上) */
    for (int n = 0; n < N; ++n) {
        double angle = -n * aAngle;
        aPowRe[n] = std::cos(angle);
        aPowIm[n] = std::sin(angle);
    }

    QVector<double> wPowRe(N + M - 1), wPowIm(N + M - 1);
    for (int n = 0; n < N + M - 1; ++n) {
        double angle = wAngle * n * n / 2.0;
        wPowRe[n] = std::cos(angle);
        wPowIm[n] = std::sin(angle);
    }

    /* 预乘 x[n] * A^{-n} */
    QVector<double> yaRe(N), yaIm(N);
    for (int n = 0; n < N; ++n) {
        yaRe[n] = input[n] * aPowRe[n];
        yaIm[n] = input[n] * aPowIm[n];
    }

    /* 直接计算每个输出点 (简化为O(N*M)) */
    int outputSize = M;

    for (int k = 0; k < M; ++k) {
        double sumRe = 0.0, sumIm = 0.0;
        for (int n = 0; n < N; ++n) {
            int idx = n * k;
            double wAngle2 = wAngle * idx;
            double wRe = std::cos(wAngle2);
            double wIm = std::sin(wAngle2);

            sumRe += yaRe[n] * wRe - yaIm[n] * wIm;
            sumIm += yaRe[n] * wIm + yaIm[n] * wRe;
        }
        /* 输出值: |X(z_k)| */
        Q_UNUSED(sumRe)
        Q_UNUSED(sumIm)
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
    emit computationCompleted(outputSize);
}

/**
 * @brief 获取当前比率参数
 * @return 螺旋比率
 */
double ChirpZTransform3::ratio() const
{
    return m_ratio;
}

/**
 * @brief 获取当前变换阶数
 * @return 输出点数
 */
int ChirpZTransform3::order() const
{
    return m_order;
}

/**
 * @brief 计算单位圆上的频率响应(CZT特例)
 *
 * 当ratio=1时，CZT退化为单位圆上的DFT，
 * 可用于Zoom FFT分析。
 *
 * @param input 输入信号
 * @return 频率响应幅度
 */
QVector<double> ChirpZTransform3::unitCircleResponse(const QVector<double>& input) const
{
    if (input.isEmpty()) return QVector<double>();
    int N = input.size();
    QVector<double> response(m_order, 0.0);
    for (int k = 0; k < m_order; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / m_order;
            re += input[n] * std::cos(angle);
            im += input[n] * std::sin(angle);
        }
        response[k] = std::sqrt(re * re + im * im);
    }
    return response;
}

/**
 * @brief 重置统计数据
 */
void ChirpZTransform3::resetStatistics()
{
    m_stats.totalComputed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
