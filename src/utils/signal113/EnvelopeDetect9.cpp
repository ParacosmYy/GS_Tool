#include "EnvelopeDetect9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化包络检测器
 * @param parent 父对象指针
 */
EnvelopeDetect9::EnvelopeDetect9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置检测模式
 * @param hilbert true=Hilbert变换模式，false=峰值检波模式
 */
void EnvelopeDetect9::setHilbertMode(bool hilbert)
{
    m_hilbertMode = hilbert;
}

/**
 * @brief 设置平滑滤波器截止频率
 * @param freqHz 截止频率(Hz)
 */
void EnvelopeDetect9::setSmoothingCutoff(double freqHz)
{
    m_smoothingCutoff = qMax(1.0, freqHz);
}

/**
 * @brief 执行包络检测
 *
 * Hilbert模式：通过FFT构造解析信号的幅度包络
 * 1. 计算输入信号的FFT
 * 2. 将负频率分量置零，正频率分量加倍
 * 3. 逆FFT得到解析信号
 * 4. 取解析信号的模作为包络
 *
 * 峰值检波模式：使用攻击-释放包络跟踪器
 * 1. 逐采样检测信号绝对值
 * 2. 上升沿使用快攻击时间常数
 * 3. 下降沿使用慢释放时间常数
 *
 * @param signal 输入时域信号
 * @return 幅值包络序列
 */
QVector<double> EnvelopeDetect9::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> envelope;
    const int n = signal.size();
    if (n == 0) {
        emit detectionCompleted(0);
        return envelope;
    }

    if (m_hilbertMode) {
        /* Hilbert变换法提取包络 */
        /* 计算DFT */
        QVector<double> re(n), im(n, 0.0);
        for (int i = 0; i < n; ++i) re[i] = signal[i];

        /* 正变换 */
        for (int k = 0; k < n; ++k) {
            double rSum = 0.0, iSum = 0.0;
            for (int i = 0; i < n; ++i) {
                double angle = -2.0 * M_PI * k * i / n;
                rSum += re[i] * qCos(angle) - im[i] * qSin(angle);
                iSum += re[i] * qSin(angle) + im[i] * qCos(angle);
            }
            re[k] = rSum;
            im[k] = iSum;
        }

        /* 构造解析信号：正频率×2，DC和Nyquist保持不变 */
        for (int k = 1; k < n / 2; ++k) {
            re[k] *= 2.0;
            im[k] *= 2.0;
        }
        for (int k = n / 2 + 1; k < n; ++k) {
            re[k] = 0.0;
            im[k] = 0.0;
        }

        /* 逆变换 */
        QVector<double> analyticRe(n, 0.0), analyticIm(n, 0.0);
        for (int i = 0; i < n; ++i) {
            double rSum = 0.0, iSum = 0.0;
            for (int k = 0; k < n; ++k) {
                double angle = 2.0 * M_PI * k * i / n;
                rSum += re[k] * qCos(angle) - im[k] * qSin(angle);
                iSum += re[k] * qSin(angle) + im[k] * qCos(angle);
            }
            analyticRe[i] = rSum / n;
            analyticIm[i] = iSum / n;
        }

        /* 取解析信号模值 */
        envelope.resize(n);
        for (int i = 0; i < n; ++i) {
            envelope[i] = qSqrt(analyticRe[i] * analyticRe[i]
                                + analyticIm[i] * analyticIm[i]);
        }
    } else {
        /* 峰值检波法 */
        envelope.resize(n);
        const double attackAlpha = 0.01;
        const double releaseAlpha = 0.9999;
        double env = 0.0;

        for (int i = 0; i < n; ++i) {
            double absVal = qAbs(signal[i]);
            if (absVal > env) {
                env = attackAlpha * absVal + (1.0 - attackAlpha) * env;
            } else {
                env = releaseAlpha * env + (1.0 - releaseAlpha) * absVal;
            }
            envelope[i] = env;
        }
    }

    /* 平滑后处理 */
    const double smoothAlpha = 0.95;
    for (int i = 1; i < n; ++i) {
        envelope[i] = smoothAlpha * envelope[i - 1]
                      + (1.0 - smoothAlpha) * envelope[i];
    }

    m_stats.totalDetected++;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;

    emit detectionCompleted(envelope.size());
    return envelope;
}

/**
 * @brief 重置所有统计信息
 */
void EnvelopeDetect9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
