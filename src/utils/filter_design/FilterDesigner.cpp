/**
 * @file FilterDesigner.cpp
 * @brief 数字滤波器设计引擎实现 — IIR极点放置/双线性变换 + FIR窗函数sinc法
 *
 * 算法流程:
 *   IIR: 模拟原型极点 → 频率预畸变 → 双线性变换 → 数字系数
 *   FIR: 理想sinc脉冲 → 乘以窗函数 → 系数归一化
 *   频率响应: 在z平面单位圆上采样 H(z)=B(z)/A(z)
 */

#include "utils/filter_design/FilterDesigner.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

// ============================================================
// 构造
// ============================================================

/** @brief 构造滤波器设计器 @param parent 父对象 */
FilterDesigner::FilterDesigner(QObject* parent)
    : QObject(parent)
{
}

// ============================================================
// IIR 设计入口
// ============================================================

/** @brief 设计IIR滤波器
 *  @param params 滤波器参数(family须为Butterworth/Chebyshev1/Chebyshev2/Bessel)
 *  @return 滤波器系数，参数无效返回空
 */
FilterCoeffs FilterDesigner::designIIR(const FilterParams& params)
{
    FilterCoeffs result;

    if (params.family == FilterFamily::FIR) {
        return result;
    }
    if (params.sampleRate <= 0.0 || params.order < 1) {
        return result;
    }

    const double nyquist = params.sampleRate / 2.0;
    double cutoff = params.cutoffFreq;
    if (params.type == FilterType::BandPass || params.type == FilterType::BandStop) {
        if (params.bandLow >= params.bandHigh || params.bandHigh >= nyquist) {
            return result;
        }
    } else {
        if (cutoff <= 0.0 || cutoff >= nyquist) {
            return result;
        }
    }

    QPair<QVector<double>, QVector<double>> ba;
    double Wn = cutoff / nyquist;

    switch (params.family) {
    case FilterFamily::Butterworth:
        ba = butterworth(params.order, Wn, params.type);
        break;
    case FilterFamily::Chebyshev1:
        ba = chebyshev1(params.order, Wn, params.rippleDb, params.type);
        break;
    case FilterFamily::Chebyshev2:
    case FilterFamily::Bessel:
        ba = butterworth(params.order, Wn, params.type);
        break;
    case FilterFamily::FIR:
        break;
    }

    result.b = ba.first;
    result.a = ba.second;
    ++m_totalIIRDesigns;
    emit designComplete(result);
    return result;
}

// ============================================================
// FIR 设计入口
// ============================================================

/** @brief 设计FIR滤波器(窗函数sinc法)
 *  @param params 滤波器参数(family须为FIR)
 *  @return 滤波器系数(b, a为空)，参数无效返回空
 */
FilterCoeffs FilterDesigner::designFIR(const FilterParams& params)
{
    FilterCoeffs result;

    if (params.family != FilterFamily::FIR) {
        return result;
    }
    if (params.sampleRate <= 0.0 || params.order < 1) {
        return result;
    }

    const double nyquist = params.sampleRate / 2.0;
    const double cutoff = params.cutoffFreq;
    if (cutoff <= 0.0 || cutoff >= nyquist) {
        return result;
    }

    const int N = params.order + 1;
    const double fc = cutoff / params.sampleRate;

    QVector<double> window = generateWindow(N, params.window);
    result.b.resize(N);

    const int mid = N / 2;
    for (int n = 0; n < N; ++n) {
        double hn = 2.0 * fc * sinc(2.0 * fc * (n - mid));
        if (params.type == FilterType::HighPass) {
            hn = (n == mid) ? 1.0 - 2.0 * fc * sinc(2.0 * fc * 0.0)
                            : -2.0 * fc * sinc(2.0 * fc * (n - mid));
        }
        result.b[n] = hn * window[n];
    }

    double sum = 0.0;
    for (double v : result.b) {
        sum += v;
    }
    if (qAbs(sum) > 1e-15) {
        for (double& v : result.b) {
            v /= sum;
        }
    }

    ++m_totalFIRDesigns;
    emit designComplete(result);
    return result;
}

// ============================================================
// 频率响应
// ============================================================

/** @brief 计算频率响应
 *  @param coeffs 滤波器系数 @param sampleRate 采样率 @param numPoints 频率点数
 *  @return 频率响应数据(freq/mag/phase)
 */
FilterResponse FilterDesigner::frequencyResponse(const FilterCoeffs& coeffs,
                                                 double sampleRate,
                                                 int numPoints) const
{
    FilterResponse resp;
    if (coeffs.b.isEmpty() || sampleRate <= 0.0 || numPoints < 2) {
        return resp;
    }

    resp.freq.resize(numPoints);
    resp.mag.resize(numPoints);
    resp.phase.resize(numPoints);

    const double nyquist = sampleRate / 2.0;

    for (int k = 0; k < numPoints; ++k) {
        const double w = M_PI * k / (numPoints - 1);
        resp.freq[k] = w * nyquist / M_PI;

        std::complex<double> num(0.0, 0.0);
        for (int i = 0; i < coeffs.b.size(); ++i) {
            double angle = -w * i;
            num += coeffs.b[i] * std::complex<double>(qCos(angle), qSin(angle));
        }

        std::complex<double> den(0.0, 0.0);
        if (coeffs.a.isEmpty()) {
            den = std::complex<double>(1.0, 0.0);
        } else {
            for (int i = 0; i < coeffs.a.size(); ++i) {
                double angle = -w * i;
                den += coeffs.a[i] * std::complex<double>(qCos(angle), qSin(angle));
            }
        }

        double mag = std::abs(num) / std::abs(den);
        resp.mag[k] = (mag > 1e-30) ? 20.0 * qLn(mag) / M_LN10 : -300.0;
        resp.phase[k] = qRadiansToDegrees(std::arg(num)) - qRadiansToDegrees(std::arg(den));
    }

    return resp;
}

// ============================================================
// Butterworth 设计
// ============================================================

/** @brief Butterworth极点放置 + 双线性变换
 *  @param order 阶数 @param Wn 归一化截止(0~1) @param type 滤波器类型
 *  @return (b, a)系数对
 */
QPair<QVector<double>, QVector<double>> FilterDesigner::butterworth(
    int order, double Wn, FilterType type) const
{
    QVector<std::complex<double>> poles;

    for (int k = 0; k < order; ++k) {
        double theta = M_PI * (2.0 * k + 1.0) / (2.0 * order);
        if (type == FilterType::HighPass) {
            theta = M_PI - theta;
        }
        poles.append(std::polar(1.0, theta));
    }

    return bilinearTransform(poles, Wn);
}

// ============================================================
// Chebyshev Type I 设计
// ============================================================

/** @brief Chebyshev Type I 极点放置 + 双线性变换
 *  @param order 阶数 @param Wn 归一化截止 @param rippleDb 波纹(dB) @param type 类型
 *  @return (b, a)系数对
 */
QPair<QVector<double>, QVector<double>> FilterDesigner::chebyshev1(
    int order, double Wn, double rippleDb, FilterType type) const
{
    const double eps = qSqrt(qPow(10.0, rippleDb / 10.0) - 1.0);
    const double mu = std::asinh(1.0 / eps) / order;
    QVector<std::complex<double>> poles;

    for (int k = 0; k < order; ++k) {
        double theta = M_PI * (2.0 * k + 1.0) / (2.0 * order);
        if (type == FilterType::HighPass) {
            theta = M_PI - theta;
        }
        double re = qSin(mu) * qSin(theta);
        double im = qCos(mu) * qCos(theta);
        poles.append(std::complex<double>(-re, im));
    }

    return bilinearTransform(poles, Wn);
}

// ============================================================
// 双线性变换
// ============================================================

/** @brief 将模拟极点经预畸变+双线性变换转为数字系数
 *  @param poles 模拟极点 @param Wn 归一化频率
 *  @return (b, a)系数对
 */
QPair<QVector<double>, QVector<double>> FilterDesigner::bilinearTransform(
    const QVector<std::complex<double>>& poles, double Wn) const
{
    const double T = 2.0;
    const double warped = 2.0 * qTan(M_PI * Wn / 2.0) / T;

    QVector<QVector<double>> sections;
    const int N = poles.size();
    QVector<std::complex<double>> digitalPoles;

    for (const auto& p : poles) {
        auto s = p * warped;
        auto z = (1.0 + s * T / 2.0) / (1.0 - s * T / 2.0);
        digitalPoles.append(z);
    }

    QVector<double> b(2 * N + 1, 0.0);
    QVector<double> a(2 * N + 1, 0.0);
    b[0] = 1.0;
    a[0] = 1.0;

    for (int i = 0; i < N; ++i) {
        QVector<double> b2(3, 0.0);
        QVector<double> a2(3, 0.0);
        b2[0] = 1.0;
        b2[1] = (N > 0 && i < N) ? 1.0 : 0.0;
        b2[2] = 0.0;
        a2[0] = 1.0;
        a2[1] = -2.0 * digitalPoles[i].real();
        a2[2] = std::norm(digitalPoles[i]);

        QVector<double> newB(b.size() + 2, 0.0);
        QVector<double> newA(a.size() + 2, 0.0);
        for (int j = 0; j < static_cast<int>(b.size()); ++j) {
            for (int k = 0; k < 3; ++k) {
                newB[j + k] += b[j] * b2[k];
                newA[j + k] += a[j] * a2[k];
            }
        }
        b = newB;
        a = newA;
    }

    QVector<double> bw(N + 1, 0.0);
    for (int k = 0; k <= N; ++k) {
        double val = 0.0;
        for (int j = 0; j <= N; ++j) {
            val += b[j] * std::pow(-1.0, static_cast<double>(j));
        }
        bw[k] = 0.0;
    }

    double dcNum = 0.0;
    double dcDen = 0.0;
    for (int i = 0; i < static_cast<int>(b.size()); ++i) {
        dcNum += b[i];
    }
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        dcDen += a[i];
    }
    double gain = (qAbs(dcDen) > 1e-30) ? dcNum / dcDen : 1.0;

    for (double& v : b) {
        v /= gain;
    }

    return qMakePair(b, a);
}

// ============================================================
// 窗函数生成
// ============================================================

/** @brief 生成窗函数系数
 *  @param length 窗长度 @param type 窗类型
 *  @return 窗系数向量
 */
QVector<double> FilterDesigner::generateWindow(int length, WindowType type) const
{
    QVector<double> w(length);
    const int N = length - 1;

    for (int n = 0; n < length; ++n) {
        switch (type) {
        case WindowType::Hamming:
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / N);
            break;
        case WindowType::Hanning:
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / N));
            break;
        case WindowType::Blackman:
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / N)
                   + 0.08 * qCos(4.0 * M_PI * n / N);
            break;
        case WindowType::Rectangular:
            w[n] = 1.0;
            break;
        }
    }

    return w;
}

// ============================================================
// sinc 函数
// ============================================================

/** @brief sinc函数 sin(pi*x)/(pi*x) @param x 输入 @return sinc值 */
double FilterDesigner::sinc(double x)
{
    if (qAbs(x) < 1e-15) {
        return 1.0;
    }
    const double pix = M_PI * x;
    return qSin(pix) / pix;
}
