#include "PrunedFFT6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <QSet>

/**
 * @brief 构造函数，初始化剪枝FFT引擎
 * @param parent 父对象指针
 */
PrunedFFT6::PrunedFFT6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void PrunedFFT6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置FFT大小
 * @param fftSize FFT点数（必须为2的幂）
 */
void PrunedFFT6::setFFTSize(int fftSize)
{
    m_fftSize = qMax(4, fftSize);
}

/**
 * @brief 估算剪枝后的计算节省率
 * @param totalBins 总频点数
 * @param outputBins 需要输出的频点数
 * @return 节省百分比 [0, 1]
 */
double PrunedFFT6::estimateSavings(int totalBins, int outputBins) const
{
    if (totalBins <= 0) return 0.0;
    return 1.0 - static_cast<double>(outputBins) / totalBins;
}

/**
 * @brief 获取bit反转索引
 */
static int bitReverse(int x, int log2n)
{
    int result = 0;
    for (int i = 0; i < log2n; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/**
 * @brief 执行剪枝FFT变换
 *
 * 仅计算指定输出频点的蝶形路径，跳过零输入的分支。
 * 当输出频点远少于总频点时，显著降低计算量。
 *
 * @param inputSignal 输入时域信号
 * @param outputBins 需要输出的频点索引集合
 * @return 指定频点的复数结果
 */
QVector<QPair<double, double>> PrunedFFT6::compute(
    const QVector<double>& inputSignal, const QVector<int>& outputBins)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    if (inputSignal.isEmpty() || outputBins.isEmpty()) {
        emit prunedCompleted(0);
        return result;
    }

    int N = inputSignal.size();
    /* 使用设定的FFT大小或输入长度 */
    int fftN = qMax(N, m_fftSize);

    /* 确保fftN为2的幂 */
    int fftLen = 1;
    while (fftLen < fftN) fftLen <<= 1;

    /* 对于少量输出频点，直接用DFT公式更高效 */
    if (outputBins.size() * 2 < fftLen) {
        for (int bin : outputBins) {
            if (bin < 0 || bin >= fftLen) {
                result.append({0.0, 0.0});
                continue;
            }
            double re = 0.0, im = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = -2.0 * M_PI * bin * n / fftLen;
                re += inputSignal[n] * qCos(angle);
                im += inputSignal[n] * qSin(angle);
            }
            result.append({re, im});
        }
    } else {
        /* 输出频点较多时使用完整FFT然后提取 */
        QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
        for (int i = 0; i < N; ++i) re[i] = inputSignal[i];

        /* 位反转 */
        int log2n = 0;
        { int tmp = fftLen; while (tmp > 1) { tmp >>= 1; log2n++; } }

        for (int i = 0; i < fftLen; ++i) {
            int j = bitReverse(i, log2n);
            if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
        }

        /* 蝶形运算 */
        for (int len = 2; len <= fftLen; len <<= 1) {
            double ang = -2.0 * M_PI / len;
            double wnRe = qCos(ang), wnIm = qSin(ang);
            for (int i = 0; i < fftLen; i += len) {
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

        /* 提取指定频点 */
        QSet<int> binSet(outputBins.begin(), outputBins.end());
        for (int bin : outputBins) {
            if (bin >= 0 && bin < fftLen)
                result.append({re[bin], im[bin]});
            else
                result.append({0.0, 0.0});
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit prunedCompleted(result.size());
    return result;
}
