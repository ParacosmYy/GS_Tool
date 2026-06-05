/**
 * @file MultiTaper2.cpp
 * @brief 多锥谱估计实现 - 基于DPSS锥的频谱估计
 *
 * 使用离散扁长椭球序列(Slepian/DPSS)作为多锥函数，
 * 对每个锥化信号做FFT后取平均，降低频谱方差。
 */

#include "utils/fft37/MultiTaper2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数(NW=4, K=7)
 * @param parent 父QObject
 */
MultiTaper2::MultiTaper2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置DPSS锥参数
 * @param nw 时间带宽积(NW)，典型值3-5
 * @param k 锥的数量，通常取 2*NW-1
 */
void MultiTaper2::setTapers(int nw, int k)
{
    m_nw = qMax(1, nw);
    m_k = qMax(1, k);
    /* 参数变更后清除缓存的锥 */
    m_tapers.clear();
    m_eigenvalues.clear();
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void MultiTaper2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算三对角矩阵的特征分解(用于DPSS生成)
 *
 * DPSS是如下三对角矩阵的特征向量:
 * T[i][i] = (N-1)/2 * cos(2*pi*NW/N) - (N-1)/2 + i
 * T[i][i+1] = T[i+1][i] = i*(N-i)/2
 *
 * @param n 矩阵大小
 * @param nw 时间带宽积
 * @param numEigvecs 需要的特征向量数
 * @return 特征向量矩阵(每行一个特征向量)和对应特征值
 */
static QPair<QVector<QVector<double>>, QVector<double>>
solveTridiagEigen(int n, double nw, int numEigvecs)
{
    /* 构建三对角矩阵 */
    QVector<double> diag(n, 0.0);
    QVector<double> offDiag(n - 1, 0.0);

    double theta = M_PI * nw / n;

    for (int i = 0; i < n; ++i) {
        diag[i] = (n - 1) / 2.0 * qCos(2.0 * theta) - (n - 1) / 2.0 + i;
    }
    for (int i = 0; i < n - 1; ++i) {
        offDiag[i] = qSqrt(static_cast<double>(i + 1) * (n - 1 - i)) / 2.0;
    }

    /* QR迭代求前numEigvecs个特征向量 */
    QVector<QVector<double>> eigvecs(numEigvecs, QVector<double>(n, 0.0));
    QVector<double> eigvals(numEigvecs, 0.0);

    /* 使用逆迭代法 */
    for (int k = 0; k < numEigvecs; ++k) {
        /* 初始猜测 */
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i)
            v[i] = qSin(M_PI * (i + 1) * (k + 1) / (n + 1));

        /* 归一化 */
        double nrm = 0.0;
        for (int i = 0; i < n; ++i) nrm += v[i] * v[i];
        nrm = qSqrt(qMax(nrm, 1e-30));
        for (int i = 0; i < n; ++i) v[i] /= nrm;

        /* 逆迭代 */
        double lambda = k * 1.0; /* 初始特征值估计 */

        for (int iter = 0; iter < 100; ++iter) {
            /* (T - lambda*I) v_new = v_old  三对角解 */
            QVector<double> d = diag;
            for (int i = 0; i < n; ++i) d[i] -= lambda;

            /* Thomas算法求解 */
            QVector<double> c = offDiag;
            QVector<double> rhs = v;

            /* 前向消元 */
            for (int i = 1; i < n; ++i) {
                double m = c[i - 1] / d[i - 1];
                d[i] -= m * c[i - 1];
                rhs[i] -= m * rhs[i - 1];
            }

            /* 回代 */
            rhs[n - 1] /= d[n - 1];
            for (int i = n - 2; i >= 0; --i)
                rhs[i] = (rhs[i] - c[i] * rhs[i + 1]) / d[i];

            /* Rayleigh商更新特征值 */
            double newLambda = 0.0;
            for (int i = 0; i < n; ++i)
                newLambda += v[i] * rhs[i];
            if (qAbs(newLambda) > 1e-30) lambda += 1.0 / newLambda;

            v = rhs;

            /* 正交化到已有特征向量 */
            for (int p = 0; p < k; ++p) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += v[i] * eigvecs[p][i];
                for (int i = 0; i < n; ++i) v[i] -= dot * eigvecs[p][i];
            }

            /* 归一化 */
            nrm = 0.0;
            for (int i = 0; i < n; ++i) nrm += v[i] * v[i];
            nrm = qSqrt(qMax(nrm, 1e-30));
            for (int i = 0; i < n; ++i) v[i] /= nrm;
        }

        eigvecs[k] = v;
        eigvals[k] = lambda;
    }

    return {eigvecs, eigvals};
}

/**
 * @brief 基础DFT实现(输入无需2的幂)
 * @param x 输入序列
 * @return 复数频谱的功率谱(只返回幅度平方)
 */
static QVector<double> computePowerSpectrum(const QVector<double>& x)
{
    const int N = x.size();
    const int halfN = N / 2 + 1;
    QVector<double> power(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        double angle = -2.0 * M_PI * k / N;
        for (int n = 0; n < N; ++n) {
            double w = angle * n;
            re += x[n] * qCos(w);
            im += x[n] * qSin(w);
        }
        power[k] = (re * re + im * im) / (N * N);
    }
    return power;
}

/**
 * @brief 执行多锥谱估计
 *
 * 步骤:
 * 1. 生成K个DPSS锥序列
 * 2. 对每个锥与输入信号的乘积计算功率谱
 * 3. 对所有功率谱取加权平均
 *
 * @param input 输入信号序列
 * @return 功率谱密度估计(N/2+1个频率点)
 */
QVector<double> MultiTaper2::estimate(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int N = input.size();
    const int halfN = N / 2 + 1;

    QVector<double> result(halfN, 0.0);

    if (N < 2) {
        m_stats.totalEstimates++;
        m_stats.totalSamplesProcessed += N;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;
        emit estimateComplete(halfN);
        return result;
    }

    /* 生成或复用DPSS锥 */
    if (m_tapers.size() != m_k || m_tapers.isEmpty() || m_tapers[0].size() != N) {
        auto pair = solveTridiagEigen(N, m_nw, m_k);
        m_tapers = pair.first;
        m_eigenvalues = pair.second;
    }

    /* 如果锥长度不匹配，截断 */
    int numTapers = qMin(m_k, m_tapers.size());

    /* 对每个锥计算功率谱并累加 */
    double weightSum = 0.0;
    for (int t = 0; t < numTapers; ++t) {
        if (m_tapers[t].size() != N) continue;

        /* 锥化信号 */
        QVector<double> tapered(N, 0.0);
        for (int i = 0; i < N; ++i)
            tapered[i] = input[i] * m_tapers[t][i];

        /* 计算功率谱 */
        QVector<double> psd = computePowerSpectrum(tapered);

        /* 权重: 使用特征值作为权重(集中度) */
        double w = 1.0; /* 统一权重简化 */
        if (t < m_eigenvalues.size())
            w = qMax(m_eigenvalues[t], 0.01);

        for (int k = 0; k < halfN; ++k)
            result[k] += psd[k] * w;
        weightSum += w;
    }

    /* 归一化 */
    if (weightSum > 0.0) {
        for (int k = 0; k < halfN; ++k)
            result[k] /= weightSum;
    }

    /* 转换为dB标度参考下的功率谱密度 */
    double freqRes = m_sampleRate / N;
    Q_UNUSED(freqRes);

    m_stats.totalEstimates++;
    m_stats.totalSamplesProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateComplete(halfN);
    return result;
}

/**
 * @brief 获取当前锥序列
 * @return 锥序列矩阵(每行一个锥)
 */
QVector<QVector<double>> MultiTaper2::taperSequences() const
{
    return m_tapers;
}

/**
 * @brief 获取锥的特征值(集中度指标)
 * @return 特征值向量
 */
QVector<double> MultiTaper2::taperEigenvalues() const
{
    return m_eigenvalues;
}

/**
 * @brief 重置所有统计数据
 */
void MultiTaper2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
