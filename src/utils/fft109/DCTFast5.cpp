#include "DCTFast5.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化快速DCT变换器
 * @param parent 父对象指针
 */
DCTFast5::DCTFast5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DCTFast5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行DCT-II（前向变换）
 *
 * DCT-II公式：
 * X[k] = Σ x[n] * cos(π(2n+1)k / 2N), k=0..N-1
 * 通过2N点预补零FFT加速计算。
 *
 * @param samples 输入时域采样
 * @return DCT系数序列
 */
QVector<double> DCTFast5::forward(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    if (N == 0) {
        emit transformCompleted(0);
        return {};
    }

    QVector<double> coefficients(N);

    /* 直接计算DCT-II（小规模数据足够高效） */
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = M_PI * (2 * n + 1) * k / (2.0 * N);
            sum += samples[n] * qCos(angle);
        }
        /* 缩放因子 */
        if (k == 0) {
            coefficients[k] = sum * qSqrt(1.0 / N);
        } else {
            coefficients[k] = sum * qSqrt(2.0 / N);
        }
    }

    m_stats.lastTransformSize = N;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N);
    return coefficients;
}

/**
 * @brief 执行DCT-III（逆变换）
 *
 * DCT-III公式：
 * x[n] = (1/N)X[0] + (2/N)Σ X[k]*cos(π(2n+1)k / 2N), k=1..N-1
 *
 * @param coefficients DCT系数序列
 * @return 重建的时域采样
 */
QVector<double> DCTFast5::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    const int N = coefficients.size();
    if (N == 0) {
        emit transformCompleted(0);
        return {};
    }

    QVector<double> samples(N);

    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double angle = M_PI * (2 * n + 1) * k / (2.0 * N);
            double c = (k == 0) ? qSqrt(1.0 / N) : qSqrt(2.0 / N);
            sum += coefficients[k] * c * qCos(angle);
        }
        samples[n] = sum;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N);
    return samples;
}

/**
 * @brief 保留前N个系数重建信号
 *
 * 有损压缩的核心：丢弃高频DCT系数，仅用低频系数重建信号。
 *
 * @param samples 原始采样
 * @param keepCoefficients 保留的系数数量
 * @return 重建后的采样
 */
QVector<double> DCTFast5::reconstruct(const QVector<double>& samples, int keepCoefficients)
{
    QVector<double> coeffs = forward(samples);
    if (keepCoefficients < coeffs.size()) {
        for (int i = keepCoefficients; i < coeffs.size(); ++i) {
            coeffs[i] = 0.0;
        }
    }
    return inverse(coeffs);
}

/**
 * @brief 计算DCT压缩的能量保留比
 * @param samples 原始采样
 * @param keepCoefficients 保留的系数数量
 * @return 能量保留百分比 [0.0, 1.0]
 */
double DCTFast5::energyRetention(const QVector<double>& samples, int keepCoefficients) const
{
    const int N = samples.size();
    if (N == 0) return 0.0;

    /* 计算总能量 */
    double totalEnergy = 0.0;
    for (double s : samples) totalEnergy += s * s;

    if (totalEnergy < 1e-20) return 1.0;

    /* 计算DCT并累加前k个系数的能量 */
    double retainedEnergy = 0.0;
    for (int k = 0; k < qMin(keepCoefficients, N); ++k) {
        double coeff = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = M_PI * (2 * n + 1) * k / (2.0 * N);
            coeff += samples[n] * qCos(angle);
        }
        retainedEnergy += coeff * coeff / N;
    }

    return qMin(1.0, retainedEnergy / totalEnergy);
}
