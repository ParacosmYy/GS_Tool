/**
 * @file SpectralEstimator.cpp
 * @brief 参数化谱估计器实现
 *
 * 实现AR(Yule-Walker/Burg)、ARMA参数化功率谱估计。
 * AR模型通过Levinson-Durbin递归或Burg反射系数高效求解;
 * ARMA通过创新算法估计; AIC准则自动选择最优模型阶数。
 */

#include "utils/signal27/SpectralEstimator.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SpectralEstimator::SpectralEstimator(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief AR模型估计 — Yule-Walker法
 *
 * 使用Levinson-Durbin递归从自相关序列快速求解AR系数。
 * 步骤:
 * 1. 计算信号的(p+1)阶自相关函数
 * 2. Levinson-Durbin递归逐步增加模型阶数
 * 3. 每步更新反射系数和预测误差功率
 *
 * @param signal 输入信号
 * @param order AR阶数p
 */
void SpectralEstimator::estimateAR(const QVector<double>& signal, int order)
{
    QElapsedTimer timer;
    timer.start();

    m_modelType = AR;
    m_maCoeffs.clear();

    if (signal.size() < order + 1 || order <= 0) {
        m_arCoeffs.clear();
        m_noiseVar = 0.0;
        m_aic = 0.0;
        return;
    }

    /* 计算自相关序列(0~order阶) */
    QVector<double> acf = autocorrelation(signal, order);

    /* Levinson-Durbin递归求解AR系数 */
    m_arCoeffs = levinsonDurbin(acf, order, m_noiseVar);

    /* 计算AIC = N*ln(noiseVar) + 2*(p+1) */
    int N = signal.size();
    m_aic = N * qLn(m_noiseVar + 1e-30) + 2.0 * (order + 1);

    /* 更新统计 */
    m_stats.totalEstimates++;
    m_stats.totalSamplesProcessed += signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(AR, order);
}

/**
 * @brief AR模型估计 — Burg法
 *
 * Burg法直接从数据估计反射系数, 不需要自相关函数。
 * 特点: 保证模型稳定性(所有反射系数绝对值<1), 频谱分辨率高。
 *
 * 算法:
 * 1. 初始化前向/后向预测误差
 * 2. 逐阶估计反射系数 k = -2*sum(f*b) / sum(f^2+b^2)
 * 3. 更新AR系数和预测误差
 *
 * @param signal 输入信号
 * @param order AR阶数
 */
void SpectralEstimator::estimateARBurg(const QVector<double>& signal, int order)
{
    QElapsedTimer timer;
    timer.start();

    m_modelType = AR;
    m_maCoeffs.clear();

    const int N = signal.size();
    if (N < order + 1 || order <= 0) {
        m_arCoeffs.clear();
        m_noiseVar = 0.0;
        m_aic = 0.0;
        return;
    }

    /* 前向和后向预测误差初始化 */
    QVector<double> fwd(signal);
    QVector<double> bwd(signal);

    /* AR系数和反射系数 */
    QVector<double> a(order + 1, 0.0);
    a[0] = 1.0;

    /* 初始误差功率 = 信号方差 */
    double errPow = 0.0;
    for (int i = 0; i < N; ++i) {
        errPow += signal[i] * signal[i];
    }
    errPow /= N;

    for (int m = 1; m <= order; ++m) {
        /* 计算反射系数 */
        double num = 0.0;
        double den = 0.0;
        for (int i = m; i < N; ++i) {
            num += fwd[i] * bwd[i - 1];
            den += fwd[i] * fwd[i] + bwd[i - 1] * bwd[i - 1];
        }
        double km = (qFabs(den) > 1e-30) ? -2.0 * num / den : 0.0;
        /* 保证稳定性: 反射系数绝对值不超过1 */
        km = qBound(-1.0, km, 1.0);

        /* 更新AR系数 */
        QVector<double> aNew = a;
        for (int i = 1; i <= m; ++i) {
            aNew[i] = a[i] + km * a[m - i];
        }
        a = aNew;

        /* 更新预测误差 */
        QVector<double> newFwd(N, 0.0);
        QVector<double> newBwd(N, 0.0);
        for (int i = m; i < N; ++i) {
            newFwd[i] = fwd[i] + km * bwd[i - 1];
            newBwd[i] = bwd[i - 1] + km * fwd[i];
        }
        fwd = newFwd;
        bwd = newBwd;

        /* 更新误差功率 */
        errPow *= (1.0 - km * km);
        if (errPow < 1e-30) errPow = 1e-30;
    }

    /* 提取AR系数 a[1..order] */
    m_arCoeffs.resize(order);
    for (int i = 0; i < order; ++i) {
        m_arCoeffs[i] = a[i + 1];
    }
    m_noiseVar = errPow;

    /* AIC */
    m_aic = N * qLn(m_noiseVar + 1e-30) + 2.0 * (order + 1);

    m_stats.totalEstimates++;
    m_stats.totalSamplesProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(AR, order);
}

/**
 * @brief ARMA模型估计 — 创新算法(Innovations)
 *
 * 先用高阶AR近似估计ARMA参数:
 * 1. 用Yule-Walker法估计高阶AR系数
 * 2. 通过长AR法推导MA系数
 * 3. 重新估计AR系数
 *
 * @param signal 输入信号
 * @param arOrder AR部分阶数
 * @param maOrder MA部分阶数
 */
void SpectralEstimator::estimateARMA(const QVector<double>& signal,
                                      int arOrder, int maOrder)
{
    QElapsedTimer timer;
    timer.start();

    m_modelType = ARMA;

    const int N = signal.size();
    if (N < arOrder + maOrder + 1 || arOrder < 0 || maOrder < 0) {
        m_arCoeffs.clear();
        m_maCoeffs.clear();
        m_noiseVar = 0.0;
        m_aic = 0.0;
        return;
    }

    if (maOrder == 0) {
        /* 退化为纯AR模型 */
        estimateAR(signal, arOrder);
        m_modelType = ARMA;
        return;
    }

    /* 长AR法: 先估计高阶AR模型 */
    int longOrder = qMax(arOrder + maOrder, 2 * (arOrder + maOrder));
    longOrder = qMin(longOrder, N / 3);

    QVector<double> acf = autocorrelation(signal, longOrder);
    double dummyVar = 1.0;
    QVector<double> longAr = levinsonDurbin(acf, longOrder, dummyVar);

    /* 从长AR推导MA系数 */
    /* 使用残差序列的自相关估计MA部分 */
    QVector<double> residuals(N, 0.0);
    for (int i = longOrder; i < N; ++i) {
        residuals[i] = signal[i];
        for (int j = 0; j < longOrder; ++j) {
            residuals[i] -= longAr[j] * signal[i - 1 - j];
        }
    }

    /* MA系数通过残差的自相关估计 */
    QVector<double> maAcf = autocorrelation(residuals, maOrder);
    m_maCoeffs.resize(maOrder + 1, 0.0);
    m_maCoeffs[0] = 1.0;
    if (maOrder > 0 && qFabs(maAcf[0]) > 1e-30) {
        for (int i = 1; i <= maOrder; ++i) {
            m_maCoeffs[i] = maAcf[i] / maAcf[0];
        }
    }

    /* AR系数取长AR的前arOrder项 */
    m_arCoeffs.resize(arOrder);
    for (int i = 0; i < arOrder; ++i) {
        m_arCoeffs[i] = (i < longAr.size()) ? longAr[i] : 0.0;
    }

    /* 噪声方差 */
    m_noiseVar = dummyVar;

    /* AIC = N*ln(noiseVar) + 2*(p+q+1) */
    m_aic = N * qLn(m_noiseVar + 1e-30) + 2.0 * (arOrder + maOrder + 1);

    m_stats.totalEstimates++;
    m_stats.totalSamplesProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(ARMA, arOrder);
}

/**
 * @brief 计算功率谱密度(PSD)
 *
 * 从估计的模型参数计算功率谱:
 * - AR:  PSD(f) = noiseVar / |1 + sum(a_k * e^{-j2πfk})|^2
 * - ARMA: PSD(f) = noiseVar * |1 + sum(b_k * e^{-j2πfk})|^2
 *                                        / |1 + sum(a_k * e^{-j2πfk})|^2
 * 输出以dB/Hz为单位: 10*log10(PSD)
 *
 * @param freqs 频率点列表(Hz)
 * @param sampleRate 采样率(Hz)
 * @return PSD值列表(dB/Hz)
 */
QVector<double> SpectralEstimator::powerSpectralDensity(const QVector<double>& freqs,
                                                         double sampleRate) const
{
    QVector<double> psd(freqs.size(), 0.0);

    for (int i = 0; i < freqs.size(); ++i) {
        double omega = 2.0 * M_PI * freqs[i] / sampleRate;

        /* 计算AR部分分母 |1 + a*z^{-1} + ...|^2 */
        double arReal = 1.0, arImag = 0.0;
        for (int k = 0; k < m_arCoeffs.size(); ++k) {
            double angle = -omega * (k + 1);
            arReal += m_arCoeffs[k] * qCos(angle);
            arImag += m_arCoeffs[k] * qSin(angle);
        }
        double arMag2 = arReal * arReal + arImag * arImag;

        double mag2 = 1.0;
        if (m_modelType == AR) {
            mag2 = m_noiseVar / (arMag2 + 1e-30);
        } else {
            /* ARMA: 乘以MA分子 */
            double maReal = 1.0, maImag = 0.0;
            for (int k = 0; k < m_maCoeffs.size(); ++k) {
                double angle = -omega * k;
                maReal += m_maCoeffs[k] * qCos(angle);
                maImag += m_maCoeffs[k] * qSin(angle);
            }
            double maMag2 = maReal * maReal + maImag * maImag;
            mag2 = m_noiseVar * maMag2 / (arMag2 + 1e-30);
        }

        psd[i] = 10.0 * qLn(mag2 + 1e-30) / qLn(10.0);
    }

    return psd;
}

/**
 * @brief 获取模型阶数
 *
 * AR模型返回AR系数数, ARMA返回AR阶数(因模型阶数由AR+MA共同定义)。
 *
 * @return 模型阶数
 */
int SpectralEstimator::order() const
{
    switch (m_modelType) {
    case AR:  return m_arCoeffs.size();
    case MA:  return m_maCoeffs.size();
    case ARMA: return m_arCoeffs.size() + m_maCoeffs.size();
    }
    return 0;
}

/**
 * @brief 重置统计计数器
 * 将估计次数、采样处理数和平均时间归零
 */
void SpectralEstimator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Levinson-Durbin递归 — 从自相关序列求解AR系数
 *
 * 递归求解Yule-Walker方程:
 * R(0)   R(1)   ... R(p-1)   a(1)       R(1)
 * R(1)   R(0)   ... R(p-2)   a(2)       R(2)
 *  ...                           =   ...
 * R(p-1) R(p-2) ... R(0)     a(p)       R(p)
 *
 * 每步递归:
 * 1. 计算反射系数 k = -(R(m+1) + sum(a_i*R(m+1-i))) / errPow
 * 2. 更新AR系数: a_i += k * a_{m-i}
 * 3. 更新误差功率: errPow *= (1 - k^2)
 *
 * @param autocorr 自相关序列(0~order阶, 长度order+1)
 * @param order AR阶数
 * @param noiseVar 输出: 预测误差功率(噪声方差)
 * @return AR系数向量(长度order, a[0]~a[order-1])
 */
QVector<double> SpectralEstimator::levinsonDurbin(const QVector<double>& autocorr,
                                                   int order, double& noiseVar) const
{
    if (autocorr.size() < order + 1 || order <= 0) {
        noiseVar = 0.0;
        return {};
    }

    QVector<double> a(order + 1, 0.0);
    a[0] = 1.0;
    double errPow = autocorr[0];

    if (qFabs(errPow) < 1e-30) {
        noiseVar = 0.0;
        QVector<double> result(order, 0.0);
        return result;
    }

    for (int m = 1; m <= order; ++m) {
        /* 计算反射系数 */
        double k = autocorr[m];
        for (int i = 1; i < m; ++i) {
            k += a[i] * autocorr[m - i];
        }
        k /= errPow;
        k = -k;

        /* 更新系数 */
        QVector<double> aNew = a;
        for (int i = 1; i < m; ++i) {
            aNew[i] = a[i] + k * a[m - i];
        }
        aNew[m] = k;
        a = aNew;

        /* 更新误差功率 */
        errPow *= (1.0 - k * k);
        if (errPow < 1e-30) errPow = 1e-30;
    }

    noiseVar = errPow;
    QVector<double> result(order);
    for (int i = 0; i < order; ++i) {
        result[i] = a[i + 1];
    }
    return result;
}

/**
 * @brief 计算自相关函数
 *
 * 使用无偏估计: R(k) = (1/(N-k)) * sum(x[i]*x[i+k])
 * 自相关函数是功率谱的傅里叶变换, 用于AR参数估计。
 *
 * @param signal 输入信号
 * @param maxLag 最大滞后阶数
 * @return 自相关序列(0~maxLag)
 */
QVector<double> SpectralEstimator::autocorrelation(const QVector<double>& signal,
                                                    int maxLag) const
{
    int N = signal.size();
    if (maxLag >= N) maxLag = N - 1;
    QVector<double> acf(maxLag + 1, 0.0);

    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            sum += signal[i] * signal[i + lag];
        }
        acf[lag] = sum / (N - lag);
    }

    return acf;
}
