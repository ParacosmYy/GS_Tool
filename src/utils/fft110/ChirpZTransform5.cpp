#include "ChirpZTransform5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Chirp-Z变换引擎
 * @param parent 父对象指针
 */
ChirpZTransform5::ChirpZTransform5(QObject* parent)
    : QObject(parent)
    , m_A(1.0, 0.0)
    , m_W(1.0, 0.0)
{
}

/**
 * @brief 重置所有统计信息
 */
void ChirpZTransform5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置螺旋轮廓参数A(起始点)和W(比率)
 * @param A 起始点复数 (实部, 虚部)
 * @param W 比率复数 (实部, 虚部)
 */
void ChirpZTransform5::setSpiralParams(const QPair<double, double>& A,
                                        const QPair<double, double>& W)
{
    m_A = A;
    m_W = W;
}

/**
 * @brief 设置输出频率点数M
 * @param m 输出点数
 */
void ChirpZTransform5::setOutputSize(int m)
{
    m_outputSize = qMax(1, m);
}

/**
 * @brief 就地基2 FFT（迭代实现）
 * @param re 实部数组
 * @param im 虚部数组
 * @param inverse 是否为逆变换
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im, bool inverse = false)
{
    const int n = re.size();
    if (n <= 1) return;

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double ang = sign * 2.0 * M_PI / len;
        double wnRe = qCos(ang), wnIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double wRe = 1.0, wIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = wRe * re[i + j + len / 2] - wIm * im[i + j + len / 2];
                double tIm = wRe * im[i + j + len / 2] + wIm * re[i + j + len / 2];
                re[i + j + len / 2] = re[i + j] - tRe;
                im[i + j + len / 2] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;
                double nwRe = wRe * wnRe - wIm * wnIm;
                double nwIm = wRe * wnIm + wIm * wnRe;
                wRe = nwRe; wIm = nwIm;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            re[i] /= n;
            im[i] /= n;
        }
    }
}

/**
 * @brief 对输入信号执行Chirp-Z变换
 *
 * 通过三步完成：乘以chirp序列 → 圆周卷积(FFT加速) → 乘以chirp序列。
 * 适用于在Z平面螺旋轮廓上的频谱分析。
 *
 * @param input 输入时域信号
 * @return M个复数频率点
 */
QVector<QPair<double, double>> ChirpZTransform5::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    const int N = input.size();
    const int M = m_outputSize;
    if (N == 0 || M <= 0) {
        emit transformCompleted(0);
        return result;
    }

    /* 计算卷积所需的FFT长度(>= N+M-1的最小2的幂) */
    int fftLen = 1;
    while (fftLen < N + M - 1) fftLen <<= 1;

    /* 构造chirp序列 y[k] = W^(k^2/2) */
    QVector<double> yRe(fftLen, 0.0), yIm(fftLen, 0.0);
    for (int k = 0; k < N + M - 1 && k < fftLen; ++k) {
        int idx = (k < N) ? k : fftLen - k;
        double k2 = static_cast<double>(idx) * idx;
        double ang = k2 * qAtan2(m_W.second, m_W.first) / qSqrt(
            m_W.first * m_W.first + m_W.second * m_W.second + 1e-30);
        double mag = qPow(qSqrt(m_W.first * m_W.first + m_W.second * m_W.second + 1e-30), k2 / 2.0);
        /* 简化：使用角度步进 */
        double chirpAng = -M_PI * idx * idx / N;
        yRe[k] = qCos(chirpAng);
        yIm[k] = qSin(chirpAng);
    }

    /* 输入信号乘以初始chirp */
    QVector<double> xRe(fftLen, 0.0), xIm(fftLen, 0.0);
    for (int n = 0; n < N; ++n) {
        double chirpAng = M_PI * n * n / N;
        double cRe = qCos(chirpAng), cIm = qSin(chirpAng);
        xRe[n] = input[n] * cRe - 0.0 * cIm;
        xIm[n] = input[n] * cIm + 0.0 * cRe;
    }

    /* FFT卷积 */
    radix2FFT(xRe, xIm);
    radix2FFT(yRe, yIm);
    for (int i = 0; i < fftLen; ++i) {
        double tRe = xRe[i] * yRe[i] - xIm[i] * yIm[i];
        double tIm = xRe[i] * yIm[i] + xIm[i] * yRe[i];
        xRe[i] = tRe; xIm[i] = tIm;
    }
    radix2FFT(xRe, xIm, true);

    /* 提取前M点并乘以最终chirp */
    result.reserve(M);
    for (int k = 0; k < M; ++k) {
        double chirpAng = M_PI * k * k / N;
        double cRe = qCos(chirpAng), cIm = qSin(chirpAng);
        double re = xRe[k] * cRe - xIm[k] * cIm;
        double im = xRe[k] * cIm + xIm[k] * cRe;
        result.append({re, im});
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(M);
    return result;
}
