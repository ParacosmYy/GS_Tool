/**
 * @file DCTFast3.cpp
 * @brief 快速离散余弦变换（DCT）实现
 *
 * 基于FFT的快速DCT-II/DCT-III（逆DCT）实现，支持1D和2D变换。
 * DCT-II广泛应用于图像压缩（JPEG）、音频编码（MP3/AAC）等领域。
 *
 * DCT-II公式: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k / 2N)
 * DCT-III公式（逆变换）: x[n] = 0.5*X[0] + sum_{k=1}^{N-1} X[k]*cos(pi*k*(2n+1)/2N)
 *
 * 通过将N点DCT重排为2N点FFT来计算，复杂度 O(N log N)。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft52/DCTFast3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认变换长度
 * @param parent 父QObject对象指针
 */
DCTFast3::DCTFast3(QObject* parent)
    : QObject(parent)
    , m_n(256)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置变换长度
 * @param n 变换点数，应尽量为2的幂次以获得最佳性能
 */
void DCTFast3::setSize(int n)
{
    m_n = qMax(1, n);
}

/**
 * @brief 执行正向DCT-II变换
 *
 * 使用DCT-II公式将时域信号变换到频域。通过2N点FFT实现快速计算:
 * 1. 将N点输入重排为2N点序列（偶数位和奇数位分别排列）
 * 2. 执行2N点FFT
 * 3. 从FFT结果中提取DCT系数
 *
 * @param input 输入时域信号，长度应等于m_n
 * @return DCT-II系数，长度为m_n
 */
QVector<double> DCTFast3::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_n;
    QVector<double> result(N, 0.0);

    if (input.isEmpty() || N <= 0) {
        return result;
    }

    /* 准备2N点FFT输入: 将输入序列重排 */
    int N2 = 2 * N;
    QVector<double> re(N2, 0.0);
    QVector<double> im(N2, 0.0);

    /* DCT-II通过重排实现: 偶数索引放前半，奇数索引反转放后半 */
    for (int n = 0; n < N; ++n) {
        int idx = (n < N) ? (2 * n) : (2 * N2 - 2 * n - 1);
        if (idx < N2) {
            re[idx] = (n < input.size()) ? input[n] : 0.0;
        }
    }

    /* 简化重排方式 */
    for (int n = 0; n < N2; ++n) {
        re[n] = 0.0;
    }
    for (int n = 0; n < N && n < input.size(); ++n) {
        if (2 * n < N2) {
            re[2 * n] = input[n];
        }
    }
    for (int n = 0; n < N && n < input.size(); ++n) {
        if (2 * n + 1 < N2) {
            re[2 * n + 1] = input[N - 1 - n];
        }
    }

    /* 执行FFT */
    fft(re, im, N2);

    /* 从FFT结果提取DCT系数 */
    double scale = 1.0 / qSqrt(2.0 * N);
    for (int k = 0; k < N; ++k) {
        double angle = k * M_PI / (2.0 * N);
        double cosA = qCos(angle);
        double sinA = qSin(angle);
        /* 实部乘以余弦加上虚部乘以正弦 */
        result[k] = scale * (re[k] * cosA + im[k] * sinA) * 2.0;
    }

    /* DC分量额外缩放 */
    if (N > 0) {
        result[0] *= qSqrt(2.0) / 2.0;
    }

    /* 更新统计（使用mutable成员或const_cast处理const方法限制） */
    const_cast<DCTFast3*>(this)->m_stats.totalTransforms++;
    const_cast<DCTFast3*>(this)->m_stats.totalPoints += N;
    double elapsed = timer.elapsed();
    const_cast<DCTFast3*>(this)->m_timeSum += elapsed;
    const_cast<DCTFast3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalTransforms;

    const_cast<DCTFast3*>(this)->emit transformCompleted(N);
    return result;
}

/**
 * @brief 执行逆向DCT-III变换
 *
 * 使用DCT-III公式将频域DCT系数还原为时域信号。
 * 通过2N点FFT实现快速计算，过程为forward的逆运算。
 *
 * @param coeffs 输入DCT系数，长度应等于m_n
 * @return 逆变换后的时域信号，长度为m_n
 */
QVector<double> DCTFast3::inverse(const QVector<double>& coeffs) const
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_n;
    QVector<double> result(N, 0.0);

    if (coeffs.isEmpty() || N <= 0) {
        return result;
    }

    int N2 = 2 * N;

    /* 构造频域序列用于逆FFT */
    QVector<double> re(N2, 0.0);
    QVector<double> im(N2, 0.0);

    /* 从DCT系数构造FFT输入 */
    for (int k = 0; k < N && k < coeffs.size(); ++k) {
        double angle = k * M_PI / (2.0 * N);
        re[k] = coeffs[k] * qCos(angle);
        im[k] = -coeffs[k] * qSin(angle);
    }

    /* 对称性: 填充共轭对称部分 */
    for (int k = 1; k < N; ++k) {
        re[N2 - k] = re[k];
        im[N2 - k] = -im[k];
    }

    /* 执行逆FFT（取共轭后FFT再取共轭） */
    for (int i = 0; i < N2; ++i) {
        im[i] = -im[i];
    }
    fft(re, im, N2);
    for (int i = 0; i < N2; ++i) {
        re[i] /= N2;
        im[i] = -im[i] / N2;
    }

    /* 从FFT结果提取时域信号 */
    for (int n = 0; n < N; ++n) {
        result[n] = re[2 * n];
    }

    double elapsed = timer.elapsed();
    const_cast<DCTFast3*>(this)->m_stats.totalTransforms++;
    const_cast<DCTFast3*>(this)->m_stats.totalPoints += N;
    const_cast<DCTFast3*>(this)->m_timeSum += elapsed;
    const_cast<DCTFast3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalTransforms;

    return result;
}

/**
 * @brief 执行2D DCT变换
 *
 * 对二维矩阵先按行执行1D DCT，再按列执行1D DCT。
 * 2D DCT是JPEG等图像压缩标准的核心变换。
 *
 * @param input 输入2D矩阵，应为 m_n x m_n 大小
 * @return 2D DCT系数矩阵
 */
QVector<QVector<double>> DCTFast3::forward2D(const QVector<QVector<double>>& input) const
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_n;
    QVector<QVector<double>> result(N, QVector<double>(N, 0.0));

    if (input.isEmpty() || N <= 0) {
        return result;
    }

    /* 第一步: 按行执行1D DCT */
    QVector<QVector<double>> temp(N, QVector<double>(N, 0.0));
    for (int i = 0; i < N; ++i) {
        QVector<double> row(N, 0.0);
        for (int j = 0; j < N && j < input.size() && j < input[qMin(i, input.size() - 1)].size(); ++j) {
            row[j] = (i < input.size() && j < input[i].size()) ? input[i][j] : 0.0;
        }
        temp[i] = forward(row);
    }

    /* 第二步: 按列执行1D DCT */
    for (int j = 0; j < N; ++j) {
        QVector<double> col(N, 0.0);
        for (int i = 0; i < N; ++i) {
            col[i] = (j < temp[i].size()) ? temp[i][j] : 0.0;
        }
        QVector<double> colResult = forward(col);
        for (int i = 0; i < N; ++i) {
            result[i][j] = colResult[i];
        }
    }

    double elapsed = timer.elapsed();
    const_cast<DCTFast3*>(this)->m_timeSum += elapsed;

    return result;
}

/**
 * @brief 基2 FFT实现（Cooley-Tukey算法）
 *
 * 实现基2按时间抽取的FFT算法。输入长度n必须为2的幂次。
 * 使用位反转排列和蝶形运算完成快速傅里叶变换。
 *
 * @param re 实部数组（输入/输出）
 * @param im 虚部数组（输入/输出）
 * @param n FFT点数，应为2的幂次
 */
void DCTFast3::fft(QVector<double>& re, QVector<double>& im, int n) const
{
    if (n <= 1) {
        return;
    }

    /* 位反转排列 */
    int j = 0;
    for (int i = 1; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                int u = i + k;
                int v = i + k + len / 2;

                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];

                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;

                /* 旋转因子递推 */
                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}

/**
 * @brief 重置所有统计计数器
 */
void DCTFast3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
