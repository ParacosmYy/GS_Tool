/**
 * @file PolarCode.cpp
 * @brief Polar码编码器/解码器实现
 */

#include "PolarCode.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <numeric>

PolarCode::PolarCode(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_n(0)
    , m_N(0)
    , m_k(0)
    , m_channelType(AWGN)
    , m_channelParam(0.5)
{
}

bool PolarCode::initialize(int n, int k, ChannelType channelType,
                           double channelParam)
{
    if (n < 1 || n > 20) return false;  /* N最大约1M */
    if (k < 1 || k > (1 << n)) return false;

    m_n = n;
    m_N = 1 << n;
    m_k = k;
    m_channelType = channelType;
    m_channelParam = channelParam;

    /* 计算可靠度序列并确定冻结集 */
    computeReliabilitySequence();

    /* 构建生成矩阵 */
    buildGeneratorMatrix();

    return true;
}

QVector<int> PolarCode::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_N, 0);

    if (infoBits.size() != m_k || m_N == 0) {
        m_timeSum += timer.elapsed();
        return codeword;
    }

    /* 将信息比特放置到信息位,冻结位填0 */
    QVector<int> u(m_N, 0);
    for (int i = 0; i < m_k; ++i)
        u[m_infoSet[i]] = infoBits[i];

    /* Polar编码: x = u * G_N, G_N = B_N * F^{⊗n} */
    /* 使用递归结构: F^{⊗n} 的蝶形运算 */
    QVector<int> x = u;
    for (int stage = 0; stage < m_n; ++stage) {
        int step = 1 << (stage + 1);
        int halfStep = 1 << stage;
        for (int i = 0; i < m_N; i += step) {
            for (int j = 0; j < halfStep; ++j) {
                int a = x[i + j];
                int b = x[i + j + halfStep];
                x[i + j] = (a + b) % 2;
                x[i + j + halfStep] = b;
            }
        }
    }

    codeword = x;

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encodeCompleted(m_N);
    return codeword;
}

QVector<int> PolarCode::decodeSC(const QVector<double>& received, bool isLLR)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded(m_k, 0);

    if (received.size() != m_N || m_N == 0) {
        m_timeSum += timer.elapsed();
        return decoded;
    }

    /* 转换为LLR */
    QVector<double> llr(m_N, 0.0);
    if (isLLR) {
        llr = received;
    } else {
        switch (m_channelType) {
        case BEC:
            llr = computeLLR_BEC(received);
            break;
        case AWGN:
        default:
            llr = computeLLR_AWGN(received);
            break;
        }
    }

    /* SC译码: 逐比特判决 */
    QVector<int> u(m_N, 0);
    QVector<double> beliefs = llr;

    /* 使用递归SC译码 */
    for (int bit = 0; bit < m_N; ++bit) {
        /* 检查是否为冻结比特 */
        bool isFrozen = !m_infoSet.contains(bit);

        if (isFrozen) {
            u[bit] = 0;  /* 冻结比特固定为0 */
        } else {
            /* 信息比特: 根据LLR硬判决 */
            u[bit] = (beliefs[bit] < 0.0) ? 1 : 0;
        }

        /* 更新后续比特的LLR(简化的传播) */
        if (bit < m_N - 1) {
            for (int next = bit + 1; next < m_N; ++next) {
                /* 使用部分和更新 */
                int stage = 0;
                int pos = bit;
                while ((pos & 1) == 0 && stage < m_n) {
                    pos >>= 1;
                    stage++;
                }
                /* 简化的LLR更新: f和g函数 */
                int blockSize = 1 << stage;
                int blockStart = (next / blockSize) * blockSize;
                if (blockStart + blockSize <= m_N) {
                    int idx = next - blockStart;
                    if (idx < blockSize / 2 && blockStart + idx + blockSize / 2 < m_N) {
                        double la = beliefs[blockStart + idx];
                        double lb = beliefs[blockStart + idx + blockSize / 2];
                        beliefs[blockStart + idx] = 2.0 * std::tanh(
                            std::atanh(std::tanh(la / 2.0) *
                                       std::tanh(lb / 2.0)));
                    }
                }
            }
        }
    }

    /* 提取信息比特 */
    for (int i = 0; i < m_k; ++i)
        decoded[i] = u[m_infoSet[i]];

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodeCompleted(m_k);
    return decoded;
}

QVector<double> PolarCode::bhattacharyyaParams(int n,
                                               ChannelType channelType,
                                               double channelParam) const
{
    int N = 1 << n;
    QVector<double> z(N, 0.0);
    z[0] = channelParam;

    /* 递归计算Bhattacharyya参数 */
    for (int level = 1; level <= n; ++level) {
        int currSize = 1 << level;
        QVector<double> newZ(currSize);
        for (int i = 0; i < currSize / 2; ++i) {
            switch (channelType) {
            case BEC:
                newZ[i] = bhattacharyyaBEC(z[i], level);
                newZ[i + currSize / 2] = z[i] * z[i];
                break;
            case AWGN:
                newZ[i] = bhattacharyyaAWGN(z[i], level);
                newZ[i + currSize / 2] = z[i] * z[i];
                break;
            case BSC:
            default:
                newZ[i] = 2.0 * z[i] - z[i] * z[i];
                newZ[i + currSize / 2] = z[i] * z[i];
                break;
            }
        }
        z = newZ;
    }

    return z;
}

QPair<int, int> PolarCode::codeParameters() const
{
    return {m_N, m_k};
}

QVector<int> PolarCode::infoBitIndices() const
{
    return m_infoSet;
}

void PolarCode::computeReliabilitySequence()
{
    /* 计算Bhattacharyya参数 */
    QVector<double> z = bhattacharyyaParams(m_n, m_channelType, m_channelParam);

    /* 按Bhattacharyya参数排序: 参数小的位置更可靠 */
    QVector<int> indices(m_N);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&z](int a, int b) { return z[a] < z[b]; });

    /* 前k个最可靠位置作为信息位 */
    m_infoSet = indices.mid(0, m_k);
    std::sort(m_infoSet.begin(), m_infoSet.end());

    /* 剩余位置作为冻结位 */
    m_frozenSet = indices.mid(m_k);
    std::sort(m_frozenSet.begin(), m_frozenSet.end());
}

void PolarCode::buildGeneratorMatrix()
{
    /* F^{⊗n} 的生成矩阵 (扁平化,行优先) */
    m_generator.resize(m_N * m_N, 0);

    /* F = [[1,0],[1,1]], F^{⊗n} 通过递归Kronecker积构建 */
    m_generator[0] = 1;
    int currSize = 1;

    for (int stage = 0; stage < m_n; ++stage) {
        int newSize = currSize * 2;
        QVector<int> newGen(newSize * newSize, 0);

        for (int i = 0; i < currSize; ++i) {
            for (int j = 0; j < currSize; ++j) {
                int val = m_generator[i * currSize + j];
                /* 左上: G */
                newGen[i * newSize + j] = val;
                /* 右上: 0 */
                newGen[i * newSize + j + currSize] = 0;
                /* 左下: G */
                newGen[(i + currSize) * newSize + j] = val;
                /* 右下: G */
                newGen[(i + currSize) * newSize + j + currSize] = val;
            }
        }

        m_generator = newGen;
        currSize = newSize;
    }
}

QVector<double> PolarCode::computeLLR_BEC(const QVector<double>& received) const
{
    QVector<double> llr(received.size(), 0.0);
    for (int i = 0; i < received.size(); ++i) {
        if (received[i] < -0.5)       llr[i] = 20.0;   /* 接收为0 */
        else if (received[i] > 0.5)   llr[i] = -20.0;  /* 接收为1 */
        else                          llr[i] = 0.0;     /* 擦除 */
    }
    return llr;
}

QVector<double> PolarCode::computeLLR_AWGN(const QVector<double>& received) const
{
    /* AWGN信道LLR: 2*y/sigma^2 */
    double sigma2 = 1.0 / (2.0 * std::pow(10.0, m_channelParam / 10.0) + 1e-15);
    QVector<double> llr(received.size());
    for (int i = 0; i < received.size(); ++i)
        llr[i] = 2.0 * received[i] / sigma2;
    return llr;
}

double PolarCode::bhattacharyyaBEC(double erasureProb, int level) const
{
    /* BEC: Z(W^-) = 2Z - Z^2 */
    Q_UNUSED(level);
    return 2.0 * erasureProb - erasureProb * erasureProb;
}

double PolarCode::bhattacharyyaAWGN(double snr, int level) const
{
    /* AWGN近似的Bhattacharyya上界 */
    Q_UNUSED(level);
    return std::exp(-snr);
}

PolarCode::Stats PolarCode::stats() const { return m_stats; }

void PolarCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
