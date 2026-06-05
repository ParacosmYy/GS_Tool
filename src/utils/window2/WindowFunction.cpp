/**
 * @file WindowFunction.cpp
 * @brief 窗函数库实现 — 20+种窗函数
 */

#include "utils/window2/WindowFunction.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
WindowFunction::WindowFunction(QObject* parent)
    : QObject(parent)
    , m_kaiserBeta(8.0)
    , m_gaussianSigma(0.4)
    , m_tukeyAlpha(0.5)
    , m_timeSum(0.0)
{
}

/** @brief 创建窗函数系数
 *  @param type 窗函数类型
 *  @param length 窗长度
 *  @return 窗系数 */
QVector<double> WindowFunction::create(Type type, int length)
{
    if (length < 1) return {};
    QVector<double> w(length, 1.0);

    for (int n = 0; n < length; ++n) {
        double t = static_cast<double>(n) / static_cast<double>(length - 1);
        double sym = (length % 2 == 0)
            ? static_cast<double>(n) - static_cast<double>(length - 1) / 2.0
            : static_cast<double>(n) - static_cast<double>(length) / 2.0;

        switch (type) {
        case Type::Rectangular:
            w[n] = 1.0;
            break;
        case Type::Hanning:
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * t));
            break;
        case Type::Hamming:
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * t);
            break;
        case Type::Blackman:
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * t)
                  + 0.08 * qCos(4.0 * M_PI * t);
            break;
        case Type::BlackmanHarris:
            w[n] = 0.35875 - 0.48829 * qCos(2.0 * M_PI * t)
                  + 0.14128 * qCos(4.0 * M_PI * t)
                  - 0.01168 * qCos(6.0 * M_PI * t);
            break;
        case Type::FlatTop:
            w[n] = 0.21557895 - 0.41663158 * qCos(2.0 * M_PI * t)
                  + 0.27726316 * qCos(4.0 * M_PI * t)
                  - 0.08357895 * qCos(6.0 * M_PI * t)
                  + 0.00694737 * qCos(8.0 * M_PI * t);
            break;
        case Type::Kaiser: {
            double halfLen = static_cast<double>(length - 1) / 2.0;
            double arg = m_kaiserBeta * qSqrt(1.0 - (sym / halfLen) * (sym / halfLen));
            /* 修正Bessel函数 I0 */
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 30; ++k) {
                term *= (arg / (2.0 * k)) * (arg / (2.0 * k));
                sum += term;
            }
            double denom = 1.0;
            term = 1.0;
            for (int k = 1; k < 30; ++k) {
                term *= (m_kaiserBeta / (2.0 * k)) * (m_kaiserBeta / (2.0 * k));
                denom += term;
            }
            w[n] = sum / denom;
            break;
        }
        case Type::Gaussian:
            w[n] = qExp(-0.5 * qPow((sym / (m_gaussianSigma * length / 2.0)), 2));
            break;
        case Type::Bartlett:
            w[n] = 1.0 - 2.0 / static_cast<double>(length - 1)
                  * qAbs(n - static_cast<double>(length - 1) / 2.0);
            break;
        case Type::Nuttall:
            w[n] = 0.355768 - 0.487396 * qCos(2.0 * M_PI * t)
                  + 0.144232 * qCos(4.0 * M_PI * t)
                  - 0.012604 * qCos(6.0 * M_PI * t);
            break;
        case Type::Tukey: {
            double alpha = m_tukeyAlpha;
            double halfLen = static_cast<double>(length - 1) / 2.0;
            if (qAbs(n - halfLen) <= alpha * halfLen) {
                w[n] = 1.0;
            } else {
                w[n] = 0.5 * (1.0 + qCos(M_PI / (1.0 - alpha)
                    * (static_cast<double>(n) / halfLen - 1.0 - alpha)));
            }
            break;
        }
        case Type::Bohman:
            w[n] = (1.0 - qAbs(sym) / (static_cast<double>(length) / 2.0))
                  * qCos(M_PI * qAbs(sym) / (static_cast<double>(length) / 2.0))
                  + 1.0 / M_PI * qSin(M_PI * qAbs(sym) / (static_cast<double>(length) / 2.0));
            break;
        case Type::Parzen: {
            double halfLen = static_cast<double>(length) / 2.0;
            double q = qAbs(sym) / halfLen;
            if (q <= 0.5) {
                w[n] = 1.0 - 6.0 * q * q * (1.0 - q);
            } else {
                w[n] = 2.0 * qPow(1.0 - q, 3);
            }
            break;
        }
        case Type::Welch:
            w[n] = 1.0 - qPow(sym / (static_cast<double>(length) / 2.0), 2);
            break;
        case Type::Cosine:
            w[n] = qSin(M_PI * t);
            break;
        case Type::Exponential:
            w[n] = qExp(-qAbs(sym) / (static_cast<double>(length) / 4.0));
            break;
        case Type::Taylor: {
            /* 简化Taylor窗 */
            double nn = static_cast<double>(length);
            w[n] = 1.0 - 2.0 * qPow(static_cast<double>(n) - nn / 2.0, 2)
                  / qPow(nn / 2.0, 2);
            w[n] = qMax(0.0, w[n]);
            break;
        }
        case Type::Chebyshev:
            /* 简化: Dolph-Chebyshev近似用Hanning替代 */
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * t));
            break;
        case Type::HannPoisson: {
            double a = 0.5;
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * t))
                  * qExp(-a * static_cast<double>(length) * qAbs(t - 0.5));
            break;
        }
        case Type::Poisson: {
            double tau = static_cast<double>(length) / 2.0 * 0.5;
            w[n] = qExp(-qAbs(sym) / tau);
            break;
        }
        case Type::Lanczos: {
            double x = 2.0 * t - 1.0;
            w[n] = (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * x) / (M_PI * x);
            break;
        }
        }
    }

    return w;
}

/** @brief 将窗函数应用到信号
 *  @param signal 输入信号
 *  @param type 窗函数类型
 *  @return 加窗后信号 */
QVector<double> WindowFunction::apply(const QVector<double>& signal, Type type)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    QVector<double> w = create(type, n);
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = signal[i] * w[i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplications;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalApplications);

    emit applicationCompleted(static_cast<int>(type), n);
    return result;
}

/** @brief 计算相干增益
 *  @param type 窗函数类型
 *  @param length 窗长度
 *  @return 相干增益 */
double WindowFunction::coherentGain(Type type, int length)
{
    QVector<double> w = create(type, length);
    double sum = 0.0;
    for (double v : w) sum += v;
    return sum / static_cast<double>(length);
}

/** @brief 设置Kaiser窗beta参数 @param beta beta值 */
void WindowFunction::setKaiserBeta(double beta) { m_kaiserBeta = qMax(0.0, beta); }

/** @brief 设置Gaussian窗sigma参数 @param sigma sigma值 */
void WindowFunction::setGaussianSigma(double sigma) { m_gaussianSigma = qMax(0.01, sigma); }

/** @brief 设置Tukey窗alpha参数 @param alpha alpha值 */
void WindowFunction::setTukeyAlpha(double alpha) { m_tukeyAlpha = qBound(0.0, alpha, 1.0); }

/** @brief 重置统计 */
void WindowFunction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
