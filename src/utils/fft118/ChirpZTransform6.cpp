#include "ChirpZTransform6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化CZT v6引擎
 * @param parent 父对象指针
 */
ChirpZTransform6::ChirpZTransform6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ChirpZTransform6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置是否使用Bluestein FFT加速
 * @param enabled 是否启用加速
 */
void ChirpZTransform6::setBluesteinAcceleration(bool enabled)
{
    m_useBluestein = enabled;
}

/**
 * @brief 就地基2 FFT
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im, bool inverse = false)
{
    const int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double ang = sign * 2.0 * M_PI / len;
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
    if (inverse) {
        for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
    }
}

/**
 * @brief 执行Chirp-Z变换
 *
 * 在Z平面螺旋轮廓上采样，通过FFT加速的卷积实现。
 * X(z_k) = sum_{n=0}^{N-1} x[n] * A^(-n) * W^(nk)
 *
 * @param inputSignal 输入时域信号
 * @param startAngle 起始角度(rad)
 * @param angleStep 角度步进(rad)
 * @param radiusStep 径向步进
 * @param numPoints 输出点数
 * @return 变换结果（复数序列）
 */
QVector<QPair<double, double>> ChirpZTransform6::compute(
    const QVector<double>& inputSignal, double startAngle, double angleStep,
    double radiusStep, int numPoints)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int N = inputSignal.size();
    if (N == 0 || numPoints <= 0) {
        emit transformCompleted(0);
        return result;
    }

    if (m_useBluestein && numPoints == N && qFuzzyCompare(radiusStep, 1.0)) {
        /* 标准FFT路径：单位圆上均匀采样 */
        int fftLen = 1;
        while (fftLen < N) fftLen <<= 1;

        QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
        for (int i = 0; i < N; ++i) re[i] = inputSignal[i];
        radix2FFT(re, im);

        result.reserve(numPoints);
        for (int i = 0; i < numPoints; ++i)
            result.append({re[i], im[i]});
    } else {
        /* 卷积实现CZT */
        int L = 1;
        while (L < N + numPoints - 1) L <<= 1;

        /* 构造输入序列 a[n] = x[n] * A^(-n) * W^(n^2/2) */
        QVector<double> aRe(L, 0.0), aIm(L, 0.0);
        for (int n = 0; n < N; ++n) {
            double angA = -startAngle * n;
            double angW = angleStep * static_cast<long long>(n) * n / 2.0;
            double radFactor = qPow(radiusStep, -n * (1.0 + n / 2.0));
            double totalAng = angA + angW;
            double env = qExp(-n * 0.01); /* 防止发散 */
            aRe[n] = inputSignal[n] * qCos(totalAng) * env;
            aIm[n] = inputSignal[n] * qSin(totalAng) * env;
        }

        /* 构造chirp序列 b[n] = W^(-n^2/2) */
        QVector<double> bRe(L, 0.0), bIm(L, 0.0);
        for (int n = 0; n < N + numPoints - 1 && n < L; ++n) {
            int idx = (n <= N - 1) ? n : L - (n - (N - 1));
            double angW = -angleStep * static_cast<long long>(idx) * idx / 2.0;
            bRe[n] = qCos(angW);
            bIm[n] = -qSin(angW);
        }

        /* FFT卷积 */
        radix2FFT(aRe, aIm);
        radix2FFT(bRe, bIm);
        for (int i = 0; i < L; ++i) {
            double tRe = aRe[i] * bRe[i] - aIm[i] * bIm[i];
            double tIm = aRe[i] * bIm[i] + aIm[i] * bRe[i];
            aRe[i] = tRe; aIm[i] = tIm;
        }
        radix2FFT(aRe, aIm, true);

        /* 提取结果并乘以最终chirp */
        result.reserve(numPoints);
        for (int k = 0; k < numPoints; ++k) {
            double angW = angleStep * static_cast<long long>(k) * k / 2.0;
            double cRe = qCos(angW), cIm = qSin(angW);
            double re = aRe[k] * cRe - aIm[k] * cIm;
            double im = aRe[k] * cIm + aIm[k] * cRe;
            result.append({re, im});
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(result.size());
    return result;
}

/**
 * @brief 在指定频率范围内进行细分辨分析
 * @param inputSignal 输入信号
 * @param startFreqHz 起始频率(Hz)
 * @param endFreqHz 终止频率(Hz)
 * @param numPoints 输出点数
 * @param sampleRate 采样率(Hz)
 * @return 频率-复数对序列
 */
QVector<QPair<double, QPair<double, double>>> ChirpZTransform6::frequencyRangeAnalysis(
    const QVector<double>& inputSignal, double startFreqHz, double endFreqHz,
    int numPoints, double sampleRate)
{
    QVector<QPair<double, QPair<double, double>>> result;

    double startAngle = 2.0 * M_PI * startFreqHz / sampleRate;
    double endAngle = 2.0 * M_PI * endFreqHz / sampleRate;
    double angleStep = (endAngle - startAngle) / qMax(1, numPoints - 1);

    auto complex = compute(inputSignal, startAngle, angleStep, 1.0, numPoints);

    result.reserve(complex.size());
    double freqStep = (endFreqHz - startFreqHz) / qMax(1, numPoints - 1);
    for (int i = 0; i < complex.size(); ++i) {
        double freq = startFreqHz + i * freqStep;
        result.append({freq, complex[i]});
    }
    return result;
}
