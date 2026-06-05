#include "ChirpZTransform4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ChirpZTransform4.cpp
 * @brief Chirp Z变换(CZT)实现
 *
 * CZT在z平面上的螺旋等间隔采样:
 * X(k) = sum_{n=0}^{N-1} x(n) * A^{-n} * W^{nk}
 * 其中A=A0*exp(j*theta0), W=W0*exp(j*phi0)
 * 可通过FFT高效计算。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
ChirpZTransform4::ChirpZTransform4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置螺旋比参数
 * @param ratio W0参数，控制螺旋的收缩/扩展
 */
void ChirpZTransform4::setRatio(double ratio)
{
    m_ratio = qMax(0.01, ratio);
}

/**
 * @brief 设置变换阶数
 * @param order 输出频点数M
 */
void ChirpZTransform4::setOrder(int order)
{
    m_order = qMax(2, order);
}

/**
 * @brief 计算Chirp Z变换
 *
 * CZT算法步骤:
 * 1. 构建y[n] = x[n] * A^{-n}
 * 2. 构建v[n] = W^{-n^2/2} 的线性卷积核
 * 3. 通过FFT计算g = y * v 的循环卷积
 * 4. 乘以W^{k^2/2}得到最终结果
 *
 * @param input 输入时域信号
 * @return 变换结果(幅度,相位)对
 */
QVector<QPair<double, double>> ChirpZTransform4::compute(const QVector<double>& input)
{
    if (input.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = input.size();
    const int M = m_order;

    QVector<QPair<double, double>> result(M);

    // 参数设置
    const double A0 = 1.0;
    const double theta0 = 0.0;
    const double W0 = m_ratio;
    const double phi0 = 2.0 * M_PI / M;

    // 直接计算CZT(简化实现)
    for (int k = 0; k < M; ++k) {
        double re = 0.0, im = 0.0;

        for (int n = 0; n < N; ++n) {
            // A^{-n} = A0^{-n} * exp(-j*n*theta0)
            const double aRe = std::pow(A0, -n) * std::cos(-n * theta0);
            const double aIm = std::pow(A0, -n) * std::sin(-n * theta0);

            // W^{nk} = W0^{nk} * exp(j*nk*phi0)
            const double wAngle = n * k * phi0;
            const double wRe = std::pow(W0, n * k) * std::cos(wAngle);
            const double wIm = std::pow(W0, n * k) * std::sin(wAngle);

            // x(n) * A^{-n} * W^{nk}
            // 先计算 x(n) * A^{-n}
            const double xaRe = input[n] * aRe;
            const double xaIm = input[n] * aIm;

            // 再乘以 W^{nk}
            re += xaRe * wRe - xaIm * wIm;
            im += xaRe * wIm + xaIm * wRe;
        }

        const double mag = std::sqrt(re * re + im * im);
        const double phase = std::atan2(im, re);
        result[k] = qMakePair(mag, phase);
    }

    // 更新统计信息
    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(M);
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void ChirpZTransform4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
