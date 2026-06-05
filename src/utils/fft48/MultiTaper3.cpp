/**
 * @file MultiTaper3.cpp
 * @brief 多锥谱3 — 自适应DPSS+多窗加权 实现
 *
 * 使用离散扁球序列 (DPSS/Slepian) 锥窗对信号进行多窗谱估计，
 * 通过特征值加权合并各锥窗的周期图，获得低方差、高分辨率的功率谱。
 */

#include "utils/fft48/MultiTaper3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
MultiTaper3::MultiTaper3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置多锥谱估计参数
 * @param fftSize FFT 长度
 * @param numTapers 锥窗数量
 * @param nw 时间-带宽积参数 (NW)
 */
void MultiTaper3::setParameters(int fftSize, int numTapers, double nw)
{
    m_fftSize = qMax(16, fftSize);
    m_numTapers = qMax(1, numTapers);
    m_nw = qMax(0.5, nw);
    m_initialized = false;
    m_stats.numTapers = m_numTapers;
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率 (Hz)
 */
void MultiTaper3::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 估计信号的功率谱密度
 *
 * 1. 计算 DPSS 锥窗
 * 2. 对每个锥窗应用信号并计算 FFT
 * 3. 按特征值加权合并周期图
 *
 * @param signal 输入信号
 * @return 功率谱密度估计
 */
QVector<double> MultiTaper3::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) {
        computeDPSS();
        m_initialized = true;
    }

    int n = qMin(signal.size(), m_fftSize);
    int halfSize = m_fftSize / 2 + 1;
    QVector<double> psd(halfSize, 0.0);

    /* 对每个锥窗计算加权周期图 */
    for (int t = 0; t < m_numTapers; ++t) {
        /* 加窗信号 */
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        for (int i = 0; i < n; ++i) {
            real[i] = signal[i] * m_tapers[t][i];
        }

        /* FFT */
        fft(real, imag, m_fftSize);

        /* 周期图累加，按特征值加权 */
        double weight = (t < m_eigenvalues.size()) ? m_eigenvalues[t] : 1.0;
        for (int k = 0; k < halfSize; ++k) {
            double power = real[k] * real[k] + imag[k] * imag[k];
            psd[k] += power * weight;
        }
    }

    /* 归一化 */
    double norm = 1.0 / (m_numTapers * m_fftSize * m_sampleRate);
    for (int k = 0; k < halfSize; ++k) {
        psd[k] *= norm;
    }

    m_psd = psd;

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimates++;
    m_stats.totalSamplesProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(m_numTapers, bandwidth());
    return psd;
}

/**
 * @brief 获取频率轴向量
 * @return 频率向量 (Hz)
 */
QVector<double> MultiTaper3::frequencies() const
{
    int halfSize = m_fftSize / 2 + 1;
    QVector<double> freq(halfSize);
    double df = m_sampleRate / m_fftSize;
    for (int k = 0; k < halfSize; ++k) {
        freq[k] = k * df;
    }
    return freq;
}

/**
 * @brief 计算谱估计的有效带宽
 * @return 带宽 (Hz)
 */
double MultiTaper3::bandwidth() const
{
    return 2.0 * m_nw * m_sampleRate / m_fftSize;
}

/**
 * @brief 重置统计信息
 */
void MultiTaper3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.numTapers = m_numTapers;
    m_timeSum = 0.0;
}

/**
 * @brief 计算离散扁球序列 (DPSS) 锥窗
 *
 * 通过三对角矩阵的特征值分解近似计算 DPSS 序列。
 * 三对角矩阵的对角元素和次对角元素由 NW 参数决定。
 */
void MultiTaper3::computeDPSS()
{
    int n = m_fftSize;
    m_tapers.resize(m_numTapers, QVector<double>(n, 0.0));
    m_eigenvalues.resize(m_numTapers, 0.0);

    double a = qCos(2.0 * M_PI * m_nw / n);

    /* 使用三对角矩阵特征值近似 DPSS */
    /* 构建 tridiag 矩阵参数并求解 */
    QVector<double> diagEigs = tridiagEigen(n, a, 1.0 - a * a);

    /* 使用逆迭代法从特征值生成 DPSS 向量 */
    for (int t = 0; t < m_numTapers; ++t) {
        double lambda = (t < diagEigs.size()) ? diagEigs[t] : 1.0;
        m_eigenvalues[t] = lambda;

        /* 初始化锥窗为正弦序列的近似 */
        for (int i = 0; i < n; ++i) {
            double phase = M_PI * (t + 1) * (i + 1.0) / (n + 1.0);
            m_tapers[t][i] = qSin(phase);
        }

        /* 幂迭代使其收敛到特征向量 */
        for (int iter = 0; iter < 50; ++iter) {
            /* 三对角矩阵乘法: y = T * x */
            QVector<double> y(n, 0.0);
            for (int i = 0; i < n; ++i) {
                double diag = n * 0.5 * a - (n * 0.5 - i) * (n * 0.5 - i - 1) * 0.0;
                double d = 0.5 * n * qCos(2.0 * M_PI * m_nw / n);
                y[i] = d * m_tapers[t][i];
                if (i > 0) {
                    double off = 0.5 * i * (n - i);
                    y[i] += off * m_tapers[t][i - 1];
                }
                if (i < n - 1) {
                    double off = 0.5 * (i + 1.0) * (n - i - 1.0);
                    y[i] += off * m_tapers[t][i + 1];
                }
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += y[i] * y[i];
            }
            norm = qSqrt(qMax(norm, 1e-30));
            for (int i = 0; i < n; ++i) {
                m_tapers[t][i] = y[i] / norm;
            }
        }
    }
}

/**
 * @brief 三对角矩阵特征值近似
 * @param n 矩阵维度
 * @param a 对角线缩放参数
 * @param b 次对角线参数
 * @return 前 m_numTapers 个最小特征值
 */
QVector<double> MultiTaper3::tridiagEigen(int n, double a, double b) const
{
    QVector<double> eigs(m_numTapers);
    for (int t = 0; t < m_numTapers; ++t) {
        /* 使用近似公式: lambda_k ≈ a*n/2 + b*... */
        eigs[t] = a * n * 0.5 + b * qCos(M_PI * (t + 1) / (n + 1)) * n * 0.5;
    }
    std::sort(eigs.begin(), eigs.end(), std::greater<double>());
    return eigs;
}

/**
 * @brief 基数-2 FFT 实现
 * @param real 实部数组（输入/输出）
 * @param imag 虚部数组（输入/输出）
 * @param n FFT 长度（必须为2的幂）
 */
void MultiTaper3::fft(QVector<double>& real, QVector<double>& imag, int n) const
{
    /* 位逆序置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            qSwap(real[i], real[j]);
            qSwap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wReal = qCos(ang);
        double wImag = qSin(ang);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }
}
