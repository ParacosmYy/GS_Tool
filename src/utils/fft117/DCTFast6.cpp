#include "DCTFast6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化快速DCT引擎
 * @param parent 父对象指针
 */
DCTFast6::DCTFast6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DCTFast6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置DCT类型
 * @param type DCT类型 (1/2/3/4)
 */
void DCTFast6::setDCTType(int type)
{
    m_dctType = qBound(1, type, 4);
}

/**
 * @brief 启用正交归一化
 * @param enabled 是否启用归一化
 */
void DCTFast6::setNormalization(bool enabled)
{
    m_normalize = enabled;
}

/**
 * @brief 就地基2 FFT
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im)
{
    const int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wnRe = qCos(ang), wnIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double wRe = 1.0, wIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = wRe * re[i+j+len/2] - wIm * im[i+j+len/2];
                double tIm = wRe * im[i+j+len/2] + wIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double nw = wRe * wnRe - wIm * wnIm;
                wIm = wRe * wnIm + wIm * wnRe;
                wRe = nw;
            }
        }
    }
}

/**
 * @brief 执行DCT-II变换（最常用形式，JPEG使用）
 *
 * 通过将输入信号重排后嵌入2N点FFT来实现快速计算。
 * 公式: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k / 2N)
 *
 * @param inputSignal 输入时域信号
 * @return DCT系数序列
 */
QVector<double> DCTFast6::forward(const QVector<double>& inputSignal)
{
    QElapsedTimer timer;
    timer.start();

    const int N = inputSignal.size();
    QVector<double> result;
    if (N == 0) {
        emit transformCompleted(0);
        return result;
    }

    result.resize(N);

    if (m_dctType == 2) {
        /* DCT-II via 2N点FFT: 重排输入 */
        int fftLen = 1;
        while (fftLen < 2 * N) fftLen <<= 1;

        QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
        /* 将x[n]映射到偶数位置：re[2n] = x[n], 奇数位置为0 */
        for (int n = 0; n < N; ++n) {
            re[n] = inputSignal[n];
        }
        /* 反转后半部分 */
        for (int n = 0; n < N / 2; ++n) {
            double tmp = re[N - 1 - n];
            re[N - 1 - n] = re[n];
            re[n] = tmp;
        }

        radix2FFT(re, im);

        /* 提取DCT系数 */
        for (int k = 0; k < N; ++k) {
            double angle = M_PI * k / (2 * N);
            result[k] = 2.0 * (re[k] * qCos(angle) + im[k] * qSin(angle));
        }

        if (m_normalize) {
            result[0] *= qSqrt(1.0 / (4.0 * N));
            for (int k = 1; k < N; ++k)
                result[k] *= qSqrt(1.0 / (2.0 * N));
        }
    } else if (m_dctType == 1) {
        /* DCT-I: 直接计算 */
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                sum += inputSignal[n] * qCos(M_PI * k * n / (N - 1));
            }
            result[k] = sum;
        }
    } else {
        /* DCT-III/IV: 直接计算 */
        for (int k = 0; k < N; ++k) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = M_PI * (2 * k + 1) * (2 * n + 1) / (4.0 * N);
                if (m_dctType == 3) angle = M_PI * k * (2 * n + 1) / (2.0 * N);
                sum += inputSignal[n] * qCos(angle);
            }
            result[k] = sum;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N);
    return result;
}

/**
 * @brief 执行逆DCT-II变换
 *
 * DCT-II的逆变换为DCT-III（带归一化系数）。
 *
 * @param dctCoefficients DCT系数序列
 * @return 重建的时域信号
 */
QVector<double> DCTFast6::inverse(const QVector<double>& dctCoefficients)
{
    QElapsedTimer timer;
    timer.start();

    const int N = dctCoefficients.size();
    QVector<double> result;
    if (N == 0) {
        emit transformCompleted(0);
        return result;
    }

    result.resize(N);

    /* IDCT-II = DCT-III: x[n] = X[0]/2 + sum_{k=1}^{N-1} X[k]*cos(pi*k*(2n+1)/(2N)) */
    for (int n = 0; n < N; ++n) {
        double sum = dctCoefficients[0] / 2.0;
        for (int k = 1; k < N; ++k) {
            sum += dctCoefficients[k] * qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        result[n] = sum;
    }

    if (m_normalize) {
        for (int n = 0; n < N; ++n)
            result[n] *= qSqrt(2.0 / N);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N);
    return result;
}
