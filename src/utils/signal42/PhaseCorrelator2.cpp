/**
 * @file PhaseCorrelator2.cpp
 * @brief 相位相关器2 — 亚像素配准+频域位移估计实现
 *
 * 实现基于相位相关的图像/信号配准：
 * - 2D FFT/IFFT变换
 * - 互功率谱计算
 * - 亚像素峰值定位（抛物线插值）
 * - 置信度评估
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/signal42/PhaseCorrelator2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PhaseCorrelator2::PhaseCorrelator2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 2D FFT变换（行-列分解法）
 * @param real 实部矩阵（w*h），就地变换
 * @param imag 虚部矩阵（w*h），就地变换
 * @param w 宽度
 * @param h 高度
 *
 * 先对每行做1D FFT，再对每列做1D FFT。
 */
void PhaseCorrelator2::fft2D(QVector<double>& real, QVector<double>& imag, int w, int h)
{
    /* 行变换 */
    for (int y = 0; y < h; ++y) {
        int rowSize = w;
        /* 1D FFT (Cooley-Tukey) */
        for (int s = 1; s < qLn(w) / qLn(2.0) + 1; ++s) {
            int m = 1 << s;
            int halfM = m >> 1;
            for (int k = 0; k < w; k += m) {
                for (int j = 0; j < halfM; ++j) {
                    double angle = -2.0 * M_PI * j / m;
                    double wr = qCos(angle);
                    double wi = qSin(angle);
                    int idx1 = y * w + k + j;
                    int idx2 = y * w + k + j + halfM;
                    double tRe = wr * real[idx2] - wi * imag[idx2];
                    double tIm = wr * imag[idx2] + wi * real[idx2];
                    real[idx2] = real[idx1] - tRe;
                    imag[idx2] = imag[idx1] - tIm;
                    real[idx1] = real[idx1] + tRe;
                    imag[idx1] = imag[idx1] + tIm;
                }
            }
        }
    }

    /* 列变换 */
    for (int x = 0; x < w; ++x) {
        for (int s = 1; s < qLn(h) / qLn(2.0) + 1; ++s) {
            int m = 1 << s;
            int halfM = m >> 1;
            for (int k = 0; k < h; k += m) {
                for (int j = 0; j < halfM; ++j) {
                    double angle = -2.0 * M_PI * j / m;
                    double wr = qCos(angle);
                    double wi = qSin(angle);
                    int idx1 = (k + j) * w + x;
                    int idx2 = (k + j + halfM) * w + x;
                    double tRe = wr * real[idx2] - wi * imag[idx2];
                    double tIm = wr * imag[idx2] + wi * real[idx2];
                    real[idx2] = real[idx1] - tRe;
                    imag[idx2] = imag[idx1] - tIm;
                    real[idx1] = real[idx1] + tRe;
                    imag[idx1] = imag[idx1] + tIm;
                }
            }
        }
    }
}

/**
 * @brief 2D IFFT变换
 * @param real 实部矩阵
 * @param imag 虚部矩阵
 * @param w 宽度
 * @param h 高度
 */
void PhaseCorrelator2::ifft2D(QVector<double>& real, QVector<double>& imag, int w, int h)
{
    /* 取共轭 */
    for (int i = 0; i < w * h; ++i)
        imag[i] = -imag[i];

    /* 正变换 */
    fft2D(real, imag, w, h);

    /* 除以N并取共轭 */
    double N = w * h;
    for (int i = 0; i < w * h; ++i) {
        real[i] /= N;
        imag[i] = -imag[i] / N;
    }
}

/**
 * @brief 亚像素峰值定位（抛物线插值）
 * @param surface 相关面
 * @param w 宽度
 * @param h 高度
 * @param px 整数峰值x
 * @param py 整数峰值y
 * @return (亚像素x, 亚像素y) 位移
 */
QPair<double,double> PhaseCorrelator2::subpixelPeak(const QVector<double>& surface,
                                                     int w, int h, int px, int py) const
{
    /* x方向抛物线插值 */
    double xLeft = surface[py * w + ((px - 1 + w) % w)];
    double xCenter = surface[py * w + px];
    double xRight = surface[py * w + ((px + 1) % w)];
    double dx = 0.0;
    double denom = xLeft - 2.0 * xCenter + xRight;
    if (qAbs(denom) > 1e-30)
        dx = (xLeft - xRight) / (2.0 * denom);

    /* y方向抛物线插值 */
    double yTop = surface[((py - 1 + h) % h) * w + px];
    double yCenter = surface[py * w + px];
    double yBottom = surface[((py + 1) % h) * w + px];
    double dy = 0.0;
    denom = yTop - 2.0 * yCenter + yBottom;
    if (qAbs(denom) > 1e-30)
        dy = (yTop - yBottom) / (2.0 * denom);

    /* 处理环绕（如果峰值在边缘，位移可能是负值） */
    double subX = px + dx;
    double subY = py + dy;
    if (subX > w / 2.0) subX -= w;
    if (subY > h / 2.0) subY -= h;

    return {subX, subY};
}

/**
 * @brief 执行相位相关，估计两幅图像间的位移
 * @param ref 参考图像（w*h，行优先）
 * @param target 目标图像（w*h，行优先）
 * @param width 图像宽度
 * @param height 图像高度
 * @return (dx, dy) 估计位移（亚像素精度）
 *
 * 步骤：
 * 1. FFT(ref) 和 FFT(target)
 * 2. 计算互功率谱 R = F_ref * conj(F_target) / |F_ref * conj(F_target)|
 * 3. IFFT(R) 得到相关面
 * 4. 找峰值并做亚像素插值
 */
QPair<double,double> PhaseCorrelator2::correlate(const QVector<double>& ref,
                                                  const QVector<double>& target,
                                                  int width, int height)
{
    QElapsedTimer timer;
    timer.start();

    int N = width * height;
    QVector<double> refReal = ref;
    QVector<double> refImag(N, 0.0);
    QVector<double> tgtReal = target;
    QVector<double> tgtImag(N, 0.0);

    /* FFT变换 */
    fft2D(refReal, refImag, width, height);
    fft2D(tgtReal, tgtImag, width, height);

    /* 计算互功率谱 */
    m_cps.resize(N);
    QVector<double> cpsReal(N), cpsImag(N);
    for (int i = 0; i < N; ++i) {
        /* R * conj(T) */
        double rr = refReal[i] * tgtReal[i] + refImag[i] * tgtImag[i];
        double ri = refImag[i] * tgtReal[i] - refReal[i] * tgtImag[i];
        double mag = qSqrt(rr * rr + ri * ri);
        if (mag > 1e-30) {
            cpsReal[i] = rr / mag;
            cpsImag[i] = ri / mag;
        } else {
            cpsReal[i] = 0.0;
            cpsImag[i] = 0.0;
        }
    }

    /* IFFT得到相关面 */
    ifft2D(cpsReal, cpsImag, width, height);

    /* 找峰值 */
    int bestX = 0, bestY = 0;
    m_peakValue = cpsReal[0];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double val = cpsReal[y * width + x];
            m_cps[y * width + x] = val;
            if (val > m_peakValue) {
                m_peakValue = val;
                bestX = x;
                bestY = y;
            }
        }
    }

    /* 计算置信度 */
    double totalEnergy = 0.0;
    for (int i = 0; i < N; ++i)
        totalEnergy += cpsReal[i] * cpsReal[i];
    m_confidence = (totalEnergy > 0) ? m_peakValue / qSqrt(totalEnergy) : 0.0;

    /* 亚像素插值 */
    QPair<double,double> shift = subpixelPeak(m_cps, width, height, bestX, bestY);

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalCorrelations++;
    m_stats.totalPixelsProcessed += N;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationCompleted(shift.first, shift.second, m_confidence);
    return shift;
}

/**
 * @brief 重置所有统计计数器
 */
void PhaseCorrelator2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
