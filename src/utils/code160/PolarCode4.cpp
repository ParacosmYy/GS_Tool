/**
 * @file PolarCode4.cpp
 * @brief PolarCode4 实现
 *
 * 实现Polar码：Bhattacharyya参数计算信道极化排序、
 * Polar编码核变换、SC逐次消除译码。
 */

#include "utils/code160/PolarCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
PolarCode4::PolarCode4(QObject* parent)
    : QObject(parent)
{
}

void PolarCode4::setCodeLength(int n)
{
    /* 确保N为2的幂 */
    int power = 1;
    while (power < n) power <<= 1;
    m_N = qMax(4, power);
}

void PolarCode4::setInfoBits(int k)
{
    m_K = qBound(1, k, m_N - 1);
}

void PolarCode4::setDesignSNR(double snrDb)
{
    m_designSNR = snrDb;
}

/**
 * @brief 计算Bhattacharyya参数
 *
 * 递推公式: Z(2i) = 2*Z(i) - Z(i)^2
 *           Z(2i+1) = Z(i)^2
 * 初始: Z(0) = exp(-10^(SNR/10))
 */
QVector<double> PolarCode4::computeBhattacharyya() const
{
    const int n = m_N;
    QVector<double> z(n);
    double sigma = qPow(10.0, -m_designSNR / 10.0);

    z[0] = qExp(-sigma);

    int currentLen = 1;
    while (currentLen < n) {
        QVector<double> newZ(2 * currentLen);
        for (int i = 0; i < currentLen; ++i) {
            double zi = z[i];
            newZ[2 * i] = 2.0 * zi - zi * zi;
            newZ[2 * i + 1] = zi * zi;
        }
        for (int i = 0; i < 2 * currentLen; ++i) {
            z[i] = qBound(0.0, newZ[i], 1.0);
        }
        currentLen *= 2;
    }

    return z;
}

/**
 * @brief 构造Polar码
 *
 * 计算Bhattacharyya参数，选择最可靠的K个信道作为信息位。
 */
void PolarCode4::buildCode()
{
    QVector<double> z = computeBhattacharyya();

    /* 按可靠性排序(值越小越可靠) */
    QVector<QPair<double, int>> zIdx;
    zIdx.reserve(m_N);
    for (int i = 0; i < m_N; ++i) {
        zIdx.append({z[i], i});
    }
    std::sort(zIdx.begin(), zIdx.end());

    /* 最可靠的K个信道为信息位 */
    m_infoIndices.clear();
    m_frozenIndices.clear();
    QSet<int> infoSet;

    for (int i = 0; i < m_K; ++i) {
        m_infoIndices.append(zIdx[i].second);
        infoSet.insert(zIdx[i].second);
    }
    std::sort(m_infoIndices.begin(), m_infoIndices.end());

    for (int i = 0; i < m_N; ++i) {
        if (!infoSet.contains(i)) {
            m_frozenIndices.append(i);
        }
    }

    m_frozenBits = QVector<int>(m_frozenIndices.size(), 0);
}

/**
 * @brief Polar变换(编码核)
 *
 * 就地执行 F = [1 0; 1 1] 的递推变换。
 */
void PolarCode4::polarTransform(QVector<int>& bits) const
{
    int n = bits.size();
    for (int stride = 2; stride <= n; stride <<= 1) {
        int half = stride >> 1;
        for (int i = 0; i < n; i += stride) {
            for (int j = 0; j < half; ++j) {
                bits[i + j] = bits[i + j] ^ bits[i + j + half];
            }
        }
    }
}

/**
 * @brief Polar码编码
 *
 * 将信息位放置到信息位索引位置，冻结位填0，执行Polar变换。
 */
QVector<int> PolarCode4::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    if (infoBits.size() != m_K) return QVector<int>();
    if (m_infoIndices.isEmpty()) buildCode();

    /* 构造u向量 */
    QVector<int> u(m_N, 0);
    for (int i = 0; i < m_K; ++i) {
        u[m_infoIndices[i]] = infoBits[i];
    }
    /* 冻结位保持为0 */

    /* Polar变换 */
    QVector<int> codeword = u;
    polarTransform(codeword);

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit encodeCompleted(m_N, m_K);
    return codeword;
}

/**
 * @brief SC逐次消除译码
 *
 * 按索引顺序逐比特译码：信息位根据LLR硬判决，冻结位固定为0。
 * LLR >= 0 判 0，LLR < 0 判 1。
 */
QVector<int> PolarCode4::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (llr.size() != m_N || m_infoIndices.isEmpty()) return QVector<int>();

    QVector<int> uHat(m_N, 0);
    QSet<int> infoSet;
    for (int idx : m_infoIndices) infoSet.insert(idx);

    /* 逐比特SC译码 */
    QVector<double> runningLLR = llr;

    for (int bit = 0; bit < m_N; ++bit) {
        if (infoSet.contains(bit)) {
            /* 信息位：根据累积LLR硬判决 */
            double sumLLR = runningLLR[bit];
            /* 考虑之前比特对当前的影响 */
            for (int prev = 0; prev < bit; ++prev) {
                if (uHat[prev] == 1) {
                    sumLLR = -sumLLR;
                    break;
                }
            }
            uHat[bit] = (sumLLR >= 0.0) ? 0 : 1;
        } else {
            /* 冻结位：固定为0 */
            uHat[bit] = 0;
        }

        /* 更新后续LLR（简化SC传播） */
        for (int j = bit + 1; j < m_N; ++j) {
            if (uHat[bit] == 1) {
                /* XOR操作影响LLR */
                double f = runningLLR[bit];
                double g = runningLLR[j];
                /* min-sum近似 */
                double sign = (f >= 0.0 && g >= 0.0) ? 1.0 : -1.0;
                if (f < 0.0 && g < 0.0) sign = 1.0;
                runningLLR[j] = sign * qMin(qAbs(f), qAbs(g));
            }
        }
    }

    /* 提取信息位 */
    QVector<int> decoded(m_K);
    for (int i = 0; i < m_K; ++i) {
        decoded[i] = uHat[m_infoIndices[i]];
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodeCompleted(0);
    return decoded;
}

void PolarCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
