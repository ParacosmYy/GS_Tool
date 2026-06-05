/**
 * @file Multitaper4.cpp
 * @brief 多锥谱估计实现 — DPSS锥窗+加权平均功率谱
 *
 * 多锥谱估计(Multitaper Spectral Estimation)使用多个正交的离散扁长椭球
 * 序列(DPSS/Slepian序列)作为锥窗函数，对信号进行多次加窗FFT分析后
 * 加权平均功率谱。相比单一锥窗方法，显著降低频谱估计方差，
 * 同时保持频率分辨率的偏置特性。
 *
 * 本实现使用三对角矩阵特征值方法生成DPSS近似，支持可配置的
 * 时间带宽积NW和锥窗数量。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/fft79/Multitaper4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 *
 * 默认NW=4.0(时间带宽积)，numTapers=7(锥窗数量)。
 * 通常选择 2*NW-1 个锥窗达到最优方差-偏置平衡。
 */
Multitaper4::Multitaper4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置时间带宽积和锥窗数量
 * @param NW 时间带宽积(典型值3~5，越大分辨率越低但方差越小)
 * @param numTapers 锥窗数量(建议 <= 2*NW-1)
 * @return 参数是否合法
 *
 * NW决定了主瓣宽度(频率分辨率)和锥窗的能量集中度。
 * numTapers越多，方差越小，但过多会引入偏置。
 */
bool Multitaper4::setParameters(double NW, int numTapers)
{
    if (NW < 1.0 || numTapers < 1) {
        return false;
    }
    m_NW = NW;
    m_numTapers = numTapers;
    m_tapers.clear();
    return true;
}

/**
 * @brief 估计功率谱密度
 * @param signal 输入实数信号
 * @return 功率谱密度估计，长度为 N/2+1
 *
 * 算法步骤:
 * 1. 生成DPSS锥窗序列(如果尚未生成或信号长度变化)
 * 2. 对每个锥窗: 加窗信号 → DFT → 计算功率谱
 * 3. 按特征值加权平均所有锥窗的功率谱
 */
QVector<double> Multitaper4::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    const int halfN = N / 2 + 1;
    QVector<double> spectrum(halfN, 0.0);

    if (N < 4) {
        m_stats.totalEstimates++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalEstimates > 0)
            ? m_timeSum / m_stats.totalEstimates : 0.0;
        emit estimationCompleted(0, 0);
        return spectrum;
    }

    /* 生成DPSS锥窗 */
    generateDPSS(N);

    /* 对每个锥窗计算加窗功率谱并加权平均 */
    double eigenSum = 0.0;
    for (int k = 0; k < m_numTapers && k < m_tapers.size(); ++k) {
        /* 特征值作为权重(跳过能量集中度太低的锥窗) */
        double eigenvalue = m_eigenvalues.value(k, 1.0);
        if (eigenvalue < 0.01) continue;
        eigenSum += eigenvalue;

        /* 加窗信号 */
        QVector<double> windowed(N);
        for (int i = 0; i < N; ++i) {
            windowed[i] = signal[i] * m_tapers[k][i];
        }

        /* 计算DFT并累加功率谱 */
        for (int f = 0; f < halfN; ++f) {
            double re = 0.0;
            double im = 0.0;
            for (int i = 0; i < N; ++i) {
                double angle = -2.0 * M_PI * f * i / N;
                re += windowed[i] * qCos(angle);
                im += windowed[i] * qSin(angle);
            }
            double power = (re * re + im * im) / static_cast<double>(N);
            spectrum[f] += eigenvalue * power;
        }
    }

    /* 归一化 */
    if (eigenSum > 1e-15) {
        for (int f = 0; f < halfN; ++f) {
            spectrum[f] /= eigenSum;
        }
    }

    /* 更新统计信息 */
    m_stats.totalEstimates++;
    m_stats.totalTapers += m_numTapers;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimationCompleted(m_numTapers, N);
    return spectrum;
}

/**
 * @brief 获取DPSS锥函数矩阵
 * @return 锥窗矩阵 [numTapers][N]，每行为一个锥窗序列
 */
QVector<QVector<double>> Multitaper4::tapers() const
{
    return m_tapers;
}

/**
 * @brief 获取各锥的独立功率谱(未平均)
 * @param signal 输入信号
 * @return 功率谱矩阵 [numTapers][N/2+1]
 *
 * 返回每个锥窗的独立功率谱，可用于分析各锥的贡献
 * 或实现自定义的加权/组合策略。
 */
QVector<QVector<double>> Multitaper4::individualSpectra(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    const int halfN = N / 2 + 1;
    QVector<QVector<double>> spectra;

    if (N < 4) {
        m_stats.totalEstimates++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalEstimates > 0)
            ? m_timeSum / m_stats.totalEstimates : 0.0;
        emit estimationCompleted(0, 0);
        return spectra;
    }

    generateDPSS(N);

    spectra.resize(m_numTapers);
    for (int k = 0; k < m_numTapers && k < m_tapers.size(); ++k) {
        spectra[k].resize(halfN, 0.0);

        /* 加窗信号 */
        QVector<double> windowed(N);
        for (int i = 0; i < N; ++i) {
            windowed[i] = signal[i] * m_tapers[k][i];
        }

        /* 计算DFT功率谱 */
        for (int f = 0; f < halfN; ++f) {
            double re = 0.0;
            double im = 0.0;
            for (int i = 0; i < N; ++i) {
                double angle = -2.0 * M_PI * f * i / N;
                re += windowed[i] * qCos(angle);
                im += windowed[i] * qSin(angle);
            }
            spectra[k][f] = (re * re + im * im) / static_cast<double>(N);
        }
    }

    /* 更新统计 */
    m_stats.totalEstimates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimationCompleted(m_numTapers, N);
    return spectra;
}

/**
 * @brief 重置所有累计统计信息
 */
void Multitaper4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_tapers.clear();
    m_eigenvalues.clear();
}

/**
 * @brief 生成DPSS(离散扁长椭球序列)锥窗
 * @param N 序列长度
 *
 * 使用三对角矩阵特征值方法近似计算DPSS:
 * 1. 构建三对角Toeplitz矩阵，主对角线为 cos(2*pi*NW/N * (i-N/2))
 * 2. 副对角线为 i*(N-1-i)/2
 * 3. 使用幂迭代法计算前numTapers个最大特征向量
 * 4. Rayleigh商估计特征值作为锥窗权重
 *
 * 该方法避免了完整的特征值分解，适合实时计算。
 */
void Multitaper4::generateDPSS(int N)
{
    /* 检查是否需要重新计算 */
    if (!m_tapers.isEmpty() && m_tapers[0].size() == N) {
        return;
    }

    m_tapers.resize(m_numTapers);
    m_eigenvalues.resize(m_numTapers);

    /* 构建三对角矩阵参数 */
    QVector<double> mainDiag(N);
    QVector<double> offDiag(N - 1);
    double w = M_PI * m_NW / N;

    for (int i = 0; i < N; ++i) {
        mainDiag[i] = qCos(2.0 * w * (i - (N - 1) / 2.0));
    }
    for (int i = 0; i < N - 1; ++i) {
        offDiag[i] = 0.5 * static_cast<double>((i + 1) * (N - 1 - i));
    }

    /* 对每个锥窗: 幂迭代 + 正交化 */
    for (int t = 0; t < m_numTapers; ++t) {
        m_tapers[t].resize(N);
        m_eigenvalues[t] = 0.0;

        /* 初始化: 正弦函数近似(DPSS的零阶近似) */
        for (int i = 0; i < N; ++i) {
            m_tapers[t][i] = qSin(M_PI * (t + 1) * (i + 1) / (N + 1));
        }

        /* 幂迭代收敛到特征向量 */
        for (int iter = 0; iter < 60; ++iter) {
            /* 三对角矩阵乘向量 */
            QVector<double> v(N, 0.0);
            v[0] = mainDiag[0] * m_tapers[t][0] + offDiag[0] * m_tapers[t][1];
            for (int i = 1; i < N - 1; ++i) {
                v[i] = offDiag[i - 1] * m_tapers[t][i - 1]
                     + mainDiag[i] * m_tapers[t][i]
                     + offDiag[i] * m_tapers[t][i + 1];
            }
            v[N - 1] = offDiag[N - 2] * m_tapers[t][N - 2]
                      + mainDiag[N - 1] * m_tapers[t][N - 1];

            /* Gram-Schmidt正交化: 减去已计算特征向量的投影 */
            for (int j = 0; j < t; ++j) {
                double dot = 0.0;
                for (int i = 0; i < N; ++i) {
                    dot += v[i] * m_tapers[j][i];
                }
                for (int i = 0; i < N; ++i) {
                    v[i] -= dot * m_tapers[j][i];
                }
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < N; ++i) {
                norm += v[i] * v[i];
            }
            norm = qSqrt(norm);
            if (norm < 1e-15) break;

            for (int i = 0; i < N; ++i) {
                m_tapers[t][i] = v[i] / norm;
            }
        }

        /* Rayleigh商计算特征值 */
        double rayleigh = 0.0;
        for (int i = 0; i < N; ++i) {
            double Av = mainDiag[i] * m_tapers[t][i];
            if (i > 0) Av += offDiag[i - 1] * m_tapers[t][i - 1];
            if (i < N - 1) Av += offDiag[i] * m_tapers[t][i + 1];
            rayleigh += m_tapers[t][i] * Av;
        }
        m_eigenvalues[t] = qBound(0.0, (rayleigh + 1.0) / 2.0, 1.0);
    }
}
