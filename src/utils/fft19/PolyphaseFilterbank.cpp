/**
 * @file PolyphaseFilterbank.cpp
 * @brief 多相滤波器组实现 — 原型设计/多相分解/分析合成
 */

#include "utils/fft19/PolyphaseFilterbank.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PolyphaseFilterbank::PolyphaseFilterbank(QObject* parent)
    : QObject(parent)
    , m_M(8)
    , m_L(4)
    , m_protoType(PrototypeType::KaiserWindow)
    , m_sampleRate(44100.0)
    , m_timeSum(0.0)
{
    designPrototype();
}

/** @brief 设置通道数 @param M 通道数 */
void PolyphaseFilterbank::setChannelCount(int M)
{
    m_M = qMax(2, M);
    designPrototype();
}

/** @brief 设置每相抽头数 @param L 抽头数 */
void PolyphaseFilterbank::setTapsPerPhase(int L)
{
    m_L = qMax(1, L);
    designPrototype();
}

/** @brief 设置原型滤波器类型 @param type 类型 */
void PolyphaseFilterbank::setPrototypeType(PrototypeType type)
{
    m_protoType = type;
    designPrototype();
}

/** @brief 设计原型滤波器 @return 原型滤波器系数 */
QVector<double> PolyphaseFilterbank::designPrototype()
{
    int N = m_M * m_L; /* 原型滤波器总长度 */

    m_prototype.resize(N);

    switch (m_protoType) {
    case PrototypeType::KaiserWindow: {
        double beta = 5.0; /* Kaiser窗参数 */
        QVector<double> sincFilter(N);
        QVector<double> kaiserWin(N);

        generateSincFilter(N, 1.0 / m_M, sincFilter);
        generateKaiserWindow(N, beta, kaiserWin);

        for (int i = 0; i < N; ++i) {
            m_prototype[i] = sincFilter[i] * kaiserWin[i];
        }
        break;
    }
    case PrototypeType::SincWindow: {
        generateSincFilter(N, 1.0 / m_M, m_prototype);
        break;
    }
    case PrototypeType::CosineModulated: {
        /* 基于余弦调制的原型滤波器 */
        generateSincFilter(N, 1.0 / (2.0 * m_M), m_prototype);
        for (int n = 0; n < N; ++n) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * (n + 0.5) / N));
            m_prototype[n] *= w;
        }
        break;
    }
    }

    /* 归一化 */
    double energy = 0.0;
    for (double h : m_prototype) energy += h * h;
    if (energy > 0) {
        double norm = 1.0 / qSqrt(energy);
        for (auto& h : m_prototype) h *= norm * qSqrt(static_cast<double>(m_M));
    }

    /* 执行多相分解 */
    m_polyMatrix = polyphaseDecompose(m_prototype);

    /* 初始化状态缓冲区 */
    m_stateMatrix.resize(m_M);
    for (auto& state : m_stateMatrix) {
        state.resize(m_L, 0.0);
    }

    return m_prototype;
}

/** @brief 多相分解 @param prototype 原型滤波器 @return 多相矩阵 */
QVector<QVector<double>> PolyphaseFilterbank::polyphaseDecompose(
    const QVector<double>& prototype)
{
    QVector<QVector<double>> polyMatrix(m_M);

    for (int m = 0; m < m_M; ++m) {
        polyMatrix[m].resize(m_L);
        for (int l = 0; l < m_L; ++l) {
            int idx = l * m_M + m;
            /* 反转系数顺序(因果实现) */
            polyMatrix[m][m_L - 1 - l] = (idx < prototype.size())
                ? prototype[idx] : 0.0;
        }
    }

    return polyMatrix;
}

/** @brief 分析滤波器组 @param input 输入样本块 @return 子带信号 */
QVector<QVector<double>> PolyphaseFilterbank::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    int blocks = N / m_M;

    if (blocks <= 0) {
        return QVector<QVector<double>>(m_M);
    }

    QVector<QVector<double>> subbands(m_M);

    for (int b = 0; b < blocks; ++b) {
        /* 步骤1: 串并转换 — 移入新数据块 */
        for (int m = 0; m < m_M; ++m) {
            /* 移位状态缓冲区 */
            for (int l = m_L - 1; l > 0; --l) {
                m_stateMatrix[m][l] = m_stateMatrix[m][l - 1];
            }
            /* 新样本进入 */
            int idx = b * m_M + m;
            m_stateMatrix[m][0] = (idx < N) ? input[idx] : 0.0;
        }

        /* 步骤2: 多相滤波 — 每相卷积 */
        QVector<double> filtered(m_M, 0.0);
        for (int m = 0; m < m_M; ++m) {
            double sum = 0.0;
            for (int l = 0; l < m_L; ++l) {
                sum += m_polyMatrix[m][l] * m_stateMatrix[m][l];
            }
            filtered[m] = sum;
        }

        /* 步骤3: M点DFT */
        QVector<double> real(m_M), imag(m_M, 0.0);
        for (int m = 0; m < m_M; ++m) real[m] = filtered[m];
        computeDFT(real, imag);

        /* 输出子带样本 */
        for (int m = 0; m < m_M; ++m) {
            subbands[m].append(qSqrt(real[m] * real[m] + imag[m] * imag[m]));
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalAnalyses;
    m_stats.totalSamplesProcessed += static_cast<quint64>(N);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAnalyses + m_stats.totalSyntheses);

    emit analysisComplete(m_M, blocks);
    return subbands;
}

/** @brief 合成滤波器组 @param subbands 子带信号 @return 重构输出 */
QVector<double> PolyphaseFilterbank::synthesize(
    const QVector<QVector<double>>& subbands)
{
    QElapsedTimer timer;
    timer.start();

    if (subbands.size() < m_M) return {};

    int blocks = subbands[0].size();
    QVector<double> output(blocks * m_M, 0.0);

    for (int b = 0; b < blocks; ++b) {
        /* 步骤1: IDFT */
        QVector<double> real(m_M), imag(m_M, 0.0);
        for (int m = 0; m < m_M; ++m) {
            real[m] = (b < subbands[m].size()) ? subbands[m][b] : 0.0;
        }
        computeIDFT(real, imag);

        /* 步骤2: 多相合成滤波 */
        QVector<double> filtered(m_M, 0.0);
        for (int m = 0; m < m_M; ++m) {
            double sum = 0.0;
            for (int l = 0; l < m_L; ++l) {
                sum += m_polyMatrix[m][l] * real[m]; /* 简化: 用原型滤波器 */
            }
            filtered[m] = sum;
        }

        /* 步骤3: 并串转换 */
        for (int m = 0; m < m_M; ++m) {
            int idx = b * m_M + m;
            if (idx < output.size()) {
                output[idx] = filtered[m];
            }
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSyntheses;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAnalyses + m_stats.totalSyntheses);

    emit synthesisComplete(output.size(), 0.0);
    return output;
}

/** @brief 完整分析-合成处理 @param input 输入 @return 重构输出 */
QVector<double> PolyphaseFilterbank::processAnalysisSynthesis(
    const QVector<double>& input)
{
    auto subbands = analyze(input);

    /* 子带处理: 这里直接传递(无处理)，可用于子带间操作 */
    QVector<double> reconstructed = synthesize(subbands);

    /* 计算重构误差 */
    int minLen = qMin(input.size(), reconstructed.size());
    if (minLen > 0) {
        double mse = reconstructionError(input.mid(0, minLen),
                                         reconstructed.mid(0, minLen));
        m_stats.reconstructionError = mse;
    }

    return reconstructed;
}

/** @brief 获取子带中心频率 @return 频率数组 */
QVector<double> PolyphaseFilterbank::subbandCenterFrequencies() const
{
    QVector<double> freqs(m_M);
    for (int m = 0; m < m_M; ++m) {
        freqs[m] = static_cast<double>(m) * m_sampleRate / (2.0 * m_M);
    }
    return freqs;
}

/** @brief 计算重构误差 @param original 原始 @param reconstructed 重构 @return MSE */
double PolyphaseFilterbank::reconstructionError(
    const QVector<double>& original,
    const QVector<double>& reconstructed) const
{
    int n = qMin(original.size(), reconstructed.size());
    if (n == 0) return 0.0;

    double mse = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = original[i] - reconstructed[i];
        mse += diff * diff;
    }
    return mse / n;
}

/** @brief 重置统计 */
void PolyphaseFilterbank::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 DFT @param real 实部 @param imag 虚部 */
void PolyphaseFilterbank::computeDFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    /* Bit-reversal置换 */
    int j = 0;
    for (int i = 1; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                double tRe = curRe * real[i + k + len/2] - curIm * imag[i + k + len/2];
                double tIm = curRe * imag[i + k + len/2] + curIm * real[i + k + len/2];
                real[i + k + len/2] = real[i + k] - tRe;
                imag[i + k + len/2] = imag[i + k] - tIm;
                real[i + k] += tRe;
                imag[i + k] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

/** @brief 基2 IDFT @param real 实部 @param imag 虚部 */
void PolyphaseFilterbank::computeIDFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();

    /* 共轭 */
    for (int i = 0; i < N; ++i) imag[i] = -imag[i];

    computeDFT(real, imag);

    /* 归一化并共轭 */
    for (int i = 0; i < N; ++i) {
        real[i] /= N;
        imag[i] = -imag[i] / N;
    }
}

/** @brief 修正Bessel函数I0 @param x 参数 @return I0(x) */
double PolyphaseFilterbank::besselI0(double x) const
{
    double sum = 1.0, term = 1.0;
    double halfX = x / 2.0;
    for (int k = 1; k < 25; ++k) {
        term *= (halfX / k) * (halfX / k);
        sum += term;
        if (term < 1e-12) break;
    }
    return sum;
}

/** @brief 生成Kaiser窗 @param N 长度 @param beta 参数 @param window 输出窗 */
void PolyphaseFilterbank::generateKaiserWindow(int N, double beta,
                                                QVector<double>& window)
{
    window.resize(N);
    double denom = besselI0(beta);
    for (int n = 0; n < N; ++n) {
        double arg = beta * qSqrt(1.0 - qPow((2.0 * n - N + 1) / (N - 1), 2));
        window[n] = besselI0(arg) / denom;
    }
}

/** @brief 生成sinc滤波器 @param N 长度 @param cutoff 归一化截止 @param filter 输出 */
void PolyphaseFilterbank::generateSincFilter(int N, double cutoff,
                                              QVector<double>& filter)
{
    filter.resize(N);
    double center = (N - 1) / 2.0;
    for (int n = 0; n < N; ++n) {
        double x = static_cast<double>(n) - center;
        if (qAbs(x) < 1e-10) {
            filter[n] = 2.0 * cutoff;
        } else {
            filter[n] = qSin(2.0 * M_PI * cutoff * x) / (M_PI * x);
        }
    }
}
