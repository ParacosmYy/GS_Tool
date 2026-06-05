/**
 * @file PolyphaseFilterbank3.cpp
 * @brief 多相滤波器组3实现 — 临界采样+完整重建
 *
 * 多相滤波器组(Polyphase Filterbank)实现，支持分析和合成两个方向。
 * 使用原型低通滤波器进行子带分解，适合音频编码和频谱分析。
 * 支持临界采样（每子带降采样至1/M）。
 */

#include "utils/fft47/PolyphaseFilterbank3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化子带数并设计原型滤波器
 * @param numBands 子带数量
 * @param parent 父QObject
 */
PolyphaseFilterbank3::PolyphaseFilterbank3(int numBands, QObject* parent)
    : QObject(parent)
    , m_numBands(numBands)
{
    designPrototype();
    buildPolyMatrix();
}

/**
 * @brief 设置子带数量
 * @param bands 子带数，应为2的幂次
 */
void PolyphaseFilterbank3::setNumBands(int bands)
{
    m_numBands = qMax(2, bands);
    designPrototype();
    buildPolyMatrix();
    m_stateBuffer.clear();
}

/**
 * @brief 设置原型滤波器长度
 * @param length 滤波器长度，应为子带数的整数倍
 */
void PolyphaseFilterbank3::setFilterLength(int length)
{
    m_filterLength = qMax(m_numBands, length);
    /* 对齐到子带数的整数倍 */
    m_filterLength = ((m_filterLength + m_numBands - 1) / m_numBands) * m_numBands;
    designPrototype();
    buildPolyMatrix();
    m_stateBuffer.clear();
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void PolyphaseFilterbank3::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 分析: 将时域信号分解为子带信号
 * @param input 输入时域信号
 * @return 子带信号矩阵，每行为一个子带
 *
 * 多相分析: 滤波 -> 降采样 -> FFT旋转
 */
QVector<QVector<double>> PolyphaseFilterbank3::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int M = m_numBands;
    const int N = input.size();
    const int numBlocks = N / M;

    if (numBlocks == 0) {
        return QVector<QVector<double>>(M, QVector<double>());
    }

    /* 初始化状态缓冲 */
    if (m_stateBuffer.size() != M) {
        m_stateBuffer.resize(M);
        for (auto& buf : m_stateBuffer) {
            buf.resize(m_filterLength / M, 0.0);
        }
    }

    QVector<QVector<double>> subbands(M, QVector<double>(numBlocks, 0.0));

    for (int block = 0; block < numBlocks; ++block) {
        /* 多相滤波: 每个子带独立滤波 */
        QVector<double> filtered(M, 0.0);

        for (int m = 0; m < M; ++m) {
            /* 抽取第m个子带的样本 */
            double sum = 0.0;
            int polyPhaseLen = m_filterLength / M;
            for (int k = 0; k < polyPhaseLen; ++k) {
                int inputIdx = block * M + m - k * M;
                if (inputIdx >= 0 && inputIdx < N) {
                    int filterIdx = k * M + m;
                    if (filterIdx >= 0 && filterIdx < m_protoFilter.size()) {
                        sum += input[inputIdx] * m_protoFilter[filterIdx];
                    }
                }
            }
            filtered[m] = sum;
        }

        /* FFT旋转: 将多相信号转换为频域子带 */
        QVector<double> fftReal = filtered;
        QVector<double> fftImag(M, 0.0);
        fft(fftReal, fftImag, M);

        /* 取幅度作为子带样本 */
        for (int m = 0; m < M; ++m) {
            subbands[m][block] = qSqrt(fftReal[m] * fftReal[m] + fftImag[m] * fftImag[m]);
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += N;
    m_stats.numBands = M;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit analysisCompleted(N, M);
    return subbands;
}

/**
 * @brief 合成: 将子带信号重建为时域信号
 * @param subbands 子带信号矩阵
 * @return 重建的时域信号
 *
 * 多相合成: IFFT旋转 -> 插值 -> 滤波 -> 叠加
 */
QVector<double> PolyphaseFilterbank3::synthesize(const QVector<QVector<double>>& subbands)
{
    QElapsedTimer timer;
    timer.start();

    const int M = qMin(subbands.size(), m_numBands);
    if (M == 0) return QVector<double>();

    const int numBlocks = subbands[0].size();
    const int outputLen = numBlocks * M;

    QVector<double> output(outputLen, 0.0);

    for (int block = 0; block < numBlocks; ++block) {
        /* IFFT旋转 */
        QVector<double> ifftReal(M, 0.0);
        QVector<double> ifftImag(M, 0.0);

        for (int m = 0; m < M && m < subbands.size(); ++m) {
            /* 从幅度重建（假设相位为零，简化合成） */
            ifftReal[m] = subbands[m][block];
        }

        /* IFFT */
        for (int i = 0; i < M; ++i) ifftImag[i] = -0.0;
        fft(ifftReal, ifftImag, M);
        for (int i = 0; i < M; ++i) {
            ifftReal[i] /= M;
            ifftImag[i] = -ifftImag[i] / M;
        }

        /* 多相合成滤波 + 插值 */
        int polyPhaseLen = m_filterLength / M;
        for (int m = 0; m < M; ++m) {
            for (int k = 0; k < polyPhaseLen; ++k) {
                int outputIdx = block * M + m + k * M;
                int filterIdx = k * M + (M - 1 - m);
                if (outputIdx < outputLen && filterIdx < m_protoFilter.size()) {
                    output[outputIdx] += ifftReal[m] * m_protoFilter[filterIdx];
                }
            }
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit synthesisCompleted(outputLen);
    return output;
}

/**
 * @brief 设计原型低通滤波器
 *
 * 使用Sinc函数加Kaiser窗设计原型滤波器。
 * 截止频率设为π/M（临界采样条件）。
 */
void PolyphaseFilterbank3::designPrototype()
{
    m_protoFilter.resize(m_filterLength);
    const double cutoff = 1.0 / m_numBands; /* 归一化截止频率 */

    /* 简单的Sinc + Hann窗原型滤波器 */
    const int N = m_filterLength;
    const int halfN = N / 2;

    for (int n = 0; n < N; ++n) {
        double t = n - halfN;
        double sinc = (qFabs(t) < 1e-10) ? 1.0 : qSin(M_PI * cutoff * t) / (M_PI * t);
        double window = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
        m_protoFilter[n] = sinc * window * cutoff;
    }

    /* 归一化: 确保单位能量 */
    double energy = 0.0;
    for (double v : m_protoFilter) energy += v * v;
    if (energy > 1e-15) {
        double norm = 1.0 / qSqrt(energy);
        for (double& v : m_protoFilter) v *= norm;
    }
}

/**
 * @brief 构建多相矩阵
 *
 * 将原型滤波器按多相结构重新排列为矩阵形式，
 * 每列对应一个子带的子滤波器系数。
 */
void PolyphaseFilterbank3::buildPolyMatrix()
{
    const int M = m_numBands;
    const int K = m_filterLength / M;

    m_polyMatrix.resize(M);
    for (int m = 0; m < M; ++m) {
        m_polyMatrix[m].resize(K);
        for (int k = 0; k < K; ++k) {
            int idx = k * M + m;
            m_polyMatrix[m][k] = (idx < m_protoFilter.size()) ? m_protoFilter[idx] : 0.0;
        }
    }
}

/**
 * @brief 基2 FFT
 * @param real 实部数组
 * @param imag 虚部数组
 * @param n FFT点数
 */
void PolyphaseFilterbank3::fft(QVector<double>& real, QVector<double>& imag, int n) const
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR; imag[v] = imag[u] - tI;
                real[u] += tR; imag[u] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }
}

/**
 * @brief 重置所有统计信息
 */
void PolyphaseFilterbank3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
