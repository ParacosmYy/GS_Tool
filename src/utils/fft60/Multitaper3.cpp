/**
 * @file Multitaper3.cpp
 * @brief 多锥化谱估计实现
 *
 * 实现基于DPSS (离散扁长椭球序列) 锥化函数的多锥化功率谱估计。
 * 通过使用多个正交锥化函数减少谱估计方差，同时保持频率分辨率的偏置特性。
 * 支持可配置的时间带宽积(NW)和锥化函数数量。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft60/Multitaper3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化多锥化谱估计器
 * @param parent 父QObject指针
 */
Multitaper3::Multitaper3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置信号长度
 * @param n 信号采样点数 (默认 1024)
 */
void Multitaper3::setSize(int n)
{
    m_n = qMax(4, n);
}

/**
 * @brief 设置锥化函数数量
 * @param nw 锥化函数数量 (默认 5)
 *
 * 通常选择 2*NW-1 个锥化函数以达到最优方差减小
 */
void Multitaper3::setNumTapers(int nw)
{
    m_numTapers = qMax(1, nw);
}

/**
 * @brief 设置时间带宽积
 * @param nw 时间带宽积 (默认 4.0)
 *
 * NW越大，频率分辨率越低但方差越小
 */
void Multitaper3::setBandwidth(double nw)
{
    m_nw = qMax(1.0, nw);
}

/**
 * @brief 计算多锥化功率谱估计
 *
 * 步骤:
 * 1. 计算DPSS锥化函数及其特征值
 * 2. 对每个锥化函数，计算加窗信号的FFT
 * 3. 计算每个锥化函数的功率谱
 * 4. 按特征值加权平均所有锥化函数的功率谱
 *
 * @param signal 输入信号
 * @return 多锥化功率谱估计 (长度为 m_n / 2 + 1)
 */
QVector<double> Multitaper3::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(m_n, signal.size());
    QVector<double> spectrum(n / 2 + 1, 0.0);

    if (n < 4) {
        m_stats.totalEstimates++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalEstimates > 0) ? m_timeSum / m_stats.totalEstimates : 0.0;
        emit estimateCompleted(0, 0);
        return spectrum;
    }

    /* 步骤1: 计算DPSS锥化函数 */
    computeDPSS(n, m_numTapers, m_nw);

    /* 步骤2: 对每个锥化函数计算功率谱并加权平均 */
    double eigenSum = 0.0;
    for (int k = 0; k < m_numTapers; ++k) {
        double eigenval = (k < m_eigenvalues.size()) ? m_eigenvalues[k] : 1.0;
        if (eigenval < 0.01) continue; /* 跳过特征值太小的锥化函数 */
        eigenSum += eigenval;

        /* 加窗信号 = 锥化函数 * 输入信号 */
        QVector<double> windowed(n);
        for (int i = 0; i < n; ++i) {
            windowed[i] = m_tapers[k][i] * signal[i];
        }

        /* 计算DFT (仅正频率部分) */
        for (int f = 0; f <= n / 2; ++f) {
            double re = 0.0;
            double im = 0.0;
            for (int i = 0; i < n; ++i) {
                double angle = -2.0 * M_PI * f * i / n;
                re += windowed[i] * qCos(angle);
                im += windowed[i] * qSin(angle);
            }
            double power = (re * re + im * im) / (n * n);
            spectrum[f] += eigenval * power;
        }
    }

    /* 归一化 */
    if (eigenSum > 0.0) {
        for (int f = 0; f <= n / 2; ++f) {
            spectrum[f] /= eigenSum;
        }
    }

    /* 更新统计 */
    m_stats.totalEstimates++;
    m_stats.totalTapers += m_numTapers;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(m_numTapers, n);
    return spectrum;
}

/**
 * @brief 重置所有统计数据
 */
void Multitaper3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_tapers.clear();
    m_eigenvalues.clear();
}

/**
 * @brief 计算DPSS (离散扁长椭球序列) 锥化函数
 *
 * 使用三对角矩阵特征值方法计算DPSS:
 * 1. 构建三对角矩阵，主对角线元素为 cos(2*pi*NW/N * i)
 * 2. 副对角线元素为 i*(n-i) / 2.0
 * 3. 计算最大的k个特征向量作为DPSS
 *
 * @param n 序列长度
 * @param k 锥化函数数量
 * @param nw 时间带宽积
 */
void Multitaper3::computeDPSS(int n, int k, double nw)
{
    m_tapers.resize(k);
    m_eigenvalues.resize(k);

    for (int t = 0; t < k; ++t) {
        m_tapers[t].resize(n);
        m_eigenvalues[t] = 0.0;
    }

    /* 构建三对角矩阵的对角线和副对角线 */
    QVector<double> mainDiag(n);
    QVector<double> offDiag(n - 1);

    double w = M_PI * nw / n;

    for (int i = 0; i < n; ++i) {
        mainDiag[i] = qCos(2.0 * w * (i - (n - 1) / 2.0));
    }
    for (int i = 0; i < n - 1; ++i) {
        offDiag[i] = 0.5 * (i + 1) * (n - 1 - i);
    }

    /* 使用简化的幂迭代法计算前k个特征向量 */
    for (int t = 0; t < k; ++t) {
        /* 初始化为正弦函数近似 */
        for (int i = 0; i < n; ++i) {
            m_tapers[t][i] = qSin(M_PI * (t + 1) * (i + 1) / (n + 1));
        }

        /* 幂迭代 */
        for (int iter = 0; iter < 50; ++iter) {
            /* 三对角矩阵乘向量 */
            QVector<double> v(n, 0.0);
            v[0] = mainDiag[0] * m_tapers[t][0] + offDiag[0] * m_tapers[t][1];
            for (int i = 1; i < n - 1; ++i) {
                v[i] = offDiag[i - 1] * m_tapers[t][i - 1]
                     + mainDiag[i] * m_tapers[t][i]
                     + offDiag[i] * m_tapers[t][i + 1];
            }
            v[n - 1] = offDiag[n - 2] * m_tapers[t][n - 2] + mainDiag[n - 1] * m_tapers[t][n - 1];

            /* 正交化: 减去之前特征向量的投影 */
            for (int j = 0; j < t; ++j) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += v[i] * m_tapers[j][i];
                for (int i = 0; i < n; ++i) v[i] -= dot * m_tapers[j][i];
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += v[i] * v[i];
            norm = qSqrt(norm);
            if (norm < 1e-15) break;

            for (int i = 0; i < n; ++i) {
                m_tapers[t][i] = v[i] / norm;
            }
        }

        /* 计算特征值 (Rayleigh商) */
        double rayleigh = 0.0;
        for (int i = 0; i < n; ++i) {
            double Av_i = mainDiag[i] * m_tapers[t][i];
            if (i > 0) Av_i += offDiag[i - 1] * m_tapers[t][i - 1];
            if (i < n - 1) Av_i += offDiag[i] * m_tapers[t][i + 1];
            rayleigh += m_tapers[t][i] * Av_i;
        }
        m_eigenvalues[t] = qBound(0.0, (rayleigh + 1.0) / 2.0, 1.0);
    }
}
