/**
 * @file PrunedFFT3.cpp
 * @brief 剪枝FFT3 — 输入/输出稀疏自适应FFT实现
 *
 * 实现输入和输出端均支持稀疏掩码的FFT算法：
 * - 自动跳过零输入/不需要的输出蝶形运算
 * - 旋转因子预计算
 * - 前向/逆向变换
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "fft40/PrunedFFT3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认FFT大小
 * @param parent 父对象
 */
PrunedFFT3::PrunedFFT3(QObject* parent)
    : QObject(parent)
{
    computeTwiddles();
}

/**
 * @brief 设置FFT大小（必须为2的幂）
 * @param n FFT点数
 */
void PrunedFFT3::setSize(int n)
{
    /* 确保为2的幂 */
    int p = 1;
    while (p < n) p <<= 1;
    m_n = p;
    m_logN = 0;
    int tmp = m_n;
    while (tmp > 1) { m_logN++; tmp >>= 1; }
    computeTwiddles();
}

/**
 * @brief 设置输入稀疏掩码
 * @param activeInputs 活跃输入索引列表
 *
 * 只处理指定索引处的非零输入，其余跳过以节省计算。
 */
void PrunedFFT3::setInputMask(const QVector<int>& activeInputs)
{
    m_inputMask = QVector<int>(m_n, 0);
    for (int idx : activeInputs) {
        if (idx >= 0 && idx < m_n)
            m_inputMask[idx] = 1;
    }
}

/**
 * @brief 设置输出稀疏掩码
 * @param activeOutputs 活跃输出索引列表
 *
 * 只计算指定索引处的输出，其余跳过以节省计算。
 */
void PrunedFFT3::setOutputMask(const QVector<int>& activeOutputs)
{
    m_outputMask = QVector<int>(m_n, 0);
    for (int idx : activeOutputs) {
        if (idx >= 0 && idx < m_n)
            m_outputMask[idx] = 1;
    }
}

/**
 * @brief 预计算旋转因子（twiddle factors）
 *
 * 为每个蝶形层预计算 W_N^k = exp(-2*pi*j*k/N)
 */
void PrunedFFT3::computeTwiddles()
{
    int halfN = m_n / 2;
    m_twiddleReal.resize(halfN);
    m_twiddleImag.resize(halfN);
    for (int k = 0; k < halfN; ++k) {
        double angle = -2.0 * M_PI * k / m_n;
        m_twiddleReal[k] = qCos(angle);
        m_twiddleImag[k] = qSin(angle);
    }
}

/**
 * @brief 反转比特序
 * @param x 输入值
 * @param bits 有效位数
 * @return 比特反转后的值
 */
int PrunedFFT3::reverseBits(int x, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/**
 * @brief 前向剪枝FFT变换
 * @param real 实部输入（长度n）
 * @param imag 虚部输入（长度n）
 * @return 变换结果实部（长度n）
 *
 * 执行剪枝蝶形运算：对于每层，根据输入掩码跳过零输入，
 * 根据输出掩码跳过不需要的输出计算。
 */
QVector<double> PrunedFFT3::forward(const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0);
    QVector<double> im(n, 0.0);

    /* 应用输入掩码并做比特反转 */
    bool hasInputMask = !m_inputMask.isEmpty();
    for (int i = 0; i < n; ++i) {
        bool active = hasInputMask ? (m_inputMask[i] == 1) : true;
        if (active && i < real.size()) {
            int j = reverseBits(i, m_logN);
            re[j] = real[i];
            im[j] = imag[i];
        }
    }

    /* 剪枝蝶形运算 */
    int prunedNodes = 0;
    for (int s = 1; s <= m_logN; ++s) {
        int m = 1 << s;
        int halfM = m >> 1;
        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                int idx = k + j;
                int idx2 = idx + halfM;
                /* 检查是否需要计算此节点 */
                bool needCompute = (re[idx] != 0.0 || im[idx] != 0.0 ||
                                    re[idx2] != 0.0 || im[idx2] != 0.0);
                if (!needCompute) {
                    prunedNodes++;
                    continue;
                }
                double twIdx = j * (n / m);
                double wr = m_twiddleReal[qBound(0, (int)twIdx, n/2 - 1)];
                double wi = m_twiddleImag[qBound(0, (int)twIdx, n/2 - 1)];
                double tRe = wr * re[idx2] - wi * im[idx2];
                double tIm = wr * im[idx2] + wi * re[idx2];
                re[idx2] = re[idx] - tRe;
                im[idx2] = im[idx] - tIm;
                re[idx] = re[idx] + tRe;
                im[idx] = im[idx] + tIm;
            }
        }
    }

    /* 应用输出掩码 */
    bool hasOutputMask = !m_outputMask.isEmpty();
    if (hasOutputMask) {
        for (int i = 0; i < n; ++i) {
            if (m_outputMask[i] == 0) {
                re[i] = 0.0;
                prunedNodes++;
            }
        }
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalPointsProcessed += n;
    m_stats.totalPrunedNodes += prunedNodes;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, prunedNodes);
    return re;
}

/**
 * @brief 逆向剪枝FFT变换
 * @param real 实部输入
 * @param imag 虚部输入
 * @return 逆变换结果实部
 *
 * 与前向变换相同，但旋转因子取共轭，最后除以N。
 */
QVector<double> PrunedFFT3::inverse(const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0);
    QVector<double> im(n, 0.0);

    /* 比特反转 */
    for (int i = 0; i < n && i < real.size(); ++i) {
        int j = reverseBits(i, m_logN);
        re[j] = real[i];
        im[j] = -imag[i];  /* 取共轭 */
    }

    /* 蝶形运算（与前向相同结构） */
    int prunedNodes = 0;
    for (int s = 1; s <= m_logN; ++s) {
        int m = 1 << s;
        int halfM = m >> 1;
        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                int idx = k + j;
                int idx2 = idx + halfM;
                if (re[idx] == 0.0 && im[idx] == 0.0 &&
                    re[idx2] == 0.0 && im[idx2] == 0.0) {
                    prunedNodes++;
                    continue;
                }
                double twIdx = j * (n / m);
                double wr = m_twiddleReal[qBound(0, (int)twIdx, n/2 - 1)];
                double wi = m_twiddleImag[qBound(0, (int)twIdx, n/2 - 1)];
                double tRe = wr * re[idx2] - (-wi) * im[idx2];
                double tIm = wr * im[idx2] + (-wi) * re[idx2];
                re[idx2] = re[idx] - tRe;
                im[idx2] = im[idx] - tIm;
                re[idx] = re[idx] + tRe;
                im[idx] = im[idx] + tIm;
            }
        }
    }

    /* 除以N并取共轭 */
    for (int i = 0; i < n; ++i)
        re[i] /= n;

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalPointsProcessed += n;
    m_stats.totalPrunedNodes += prunedNodes;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, prunedNodes);
    return re;
}

/**
 * @brief 计算当前剪枝效率
 * @return 效率值（0~1，1表示无剪枝）
 */
double PrunedFFT3::efficiency() const
{
    if (m_stats.totalPointsProcessed == 0) return 1.0;
    double totalPossible = m_stats.totalTransforms * m_n * m_logN / 2.0;
    if (totalPossible == 0) return 1.0;
    return 1.0 - (double)m_stats.totalPrunedNodes / totalPossible;
}

/**
 * @brief 重置所有统计计数器
 */
void PrunedFFT3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
