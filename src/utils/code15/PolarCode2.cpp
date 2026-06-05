/**
 * @file PolarCode2.cpp
 * @brief 极化码编解码器实现
 */

#include "PolarCode2.h"
#include <QElapsedTimer>
#include <QSet>
#include <cmath>
#include <algorithm>
#include <limits>

PolarCode2::PolarCode2(int n, int k, int listSize, QObject* parent)
    : QObject(parent)
    , m_n(n)
    , m_k(k)
    , m_listSize(listSize)
    , m_timeSum(0.0)
{
    /* 计算级数 */
    m_stages = 0;
    int tmp = n;
    while (tmp > 1) { m_stages++; tmp >>= 1; }

    /* 计算信道可靠度并选择索引 */
    computeReliability();
    selectIndices();
}

QVector<int> PolarCode2::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_n, 0);

    /* 将信息位放入对应位置 */
    for (int i = 0; i < m_k && i < infoBits.size(); ++i)
        codeword[m_infoIndices[i]] = infoBits[i];

    /* 极化编码: x = u * G_n, G_n = B_n * F^{⊗n}, F = [[1,0],[1,1]] */
    QVector<int> result = codeword;
    for (int stage = 0; stage < m_stages; ++stage) {
        int stride = 1 << stage;
        int blockSize = stride << 1;
        QVector<int> temp = result;
        for (int base = 0; base < m_n; base += blockSize) {
            for (int j = 0; j < stride; ++j) {
                int idx1 = base + j;
                int idx2 = base + j + stride;
                temp[idx1] = (result[idx1] + result[idx2]) % 2;
                /* idx2保持不变 */
            }
        }
        result = temp;
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodingCompleted(m_n, m_k);
    return result;
}

QVector<int> PolarCode2::decode(const QVector<double>& llr, DecodeMethod method)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (method == SC) {
        scDecode(llr, decoded);
    } else {
        decoded = sclDecode(llr);
    }

    /* 提取信息位 */
    QVector<int> infoBits(m_k);
    for (int i = 0; i < m_k; ++i)
        infoBits[i] = decoded[m_infoIndices[i]];

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit decodingCompleted(0);
    return infoBits;
}

double PolarCode2::codeRate() const
{
    return static_cast<double>(m_k) / m_n;
}

QVector<double> PolarCode2::channelPolarization(double initialError) const
{
    QVector<double> current(1, initialError);
    for (int stage = 0; stage < m_stages; ++stage) {
        QVector<double> next;
        for (double z : current) {
            /* W- 信道: z = 2z^2 - 2z^3 ≈ 2z - z^2 (近似) */
            next.append(2.0 * z - z * z);
            /* W+ 信道: z = z^2 */
            next.append(z * z);
        }
        current = next;
    }
    std::sort(current.begin(), current.end());
    return current;
}

void PolarCode2::computeReliability()
{
    /* 简化Bhattacharyya参数递归 */
    m_reliability.resize(m_n);
    QVector<double> bhata(1, 0.5);
    for (int stage = 0; stage < m_stages; ++stage) {
        QVector<double> next;
        for (double z : bhata) {
            next.append(2.0 * z - z * z);
            next.append(z * z);
        }
        bhata = next;
    }
    /* 可靠度 = 1 - Bhattacharyya */
    for (int i = 0; i < m_n; ++i)
        m_reliability[i] = 1.0 - bhata[i];
}

void PolarCode2::selectIndices()
{
    /* 按可靠度排序，选最高的k个作为信息位 */
    QVector<QPair<double, int>> indexed;
    for (int i = 0; i < m_n; ++i)
        indexed.append({m_reliability[i], i});

    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    m_infoIndices.resize(m_k);
    for (int i = 0; i < m_k; ++i)
        m_infoIndices[i] = indexed[i].second;

    /* 冻结位 = 剩余的 */
    QSet<int> infoSet(m_infoIndices.begin(), m_infoIndices.end());
    m_frozenIndices.clear();
    for (int i = 0; i < m_n; ++i) {
        if (!infoSet.contains(i))
            m_frozenIndices.append(i);
    }
    std::sort(m_frozenIndices.begin(), m_frozenIndices.end());
}

void PolarCode2::scDecode(const QVector<double>& llr, QVector<int>& decoded)
{
    decoded.resize(m_n);
    QSet<int> frozenSet(m_frozenIndices.begin(), m_frozenIndices.end());

    /* 简化SC: 逐位决定 */
    QVector<double> curLLR = llr;
    for (int i = 0; i < m_n; ++i) {
        if (frozenSet.contains(i)) {
            decoded[i] = 0;
        } else {
            decoded[i] = (curLLR[i] < 0) ? 1 : 0;
        }

        /* 更新后续LLR (简化: 直接用原始LLR) */
        if (i + 1 < m_n) {
            double absLLR = std::abs(curLLR[i]);
            if (decoded[i] == 1)
                curLLR[i + 1] = -curLLR[i + 1];
        }
    }
}

QVector<int> PolarCode2::sclDecode(const QVector<double>& llr)
{
    QSet<int> frozenSet(m_frozenIndices.begin(), m_frozenIndices.end());

    /* 初始化路径列表 */
    QList<Path> paths;
    Path initPath;
    initPath.bits.resize(m_n, 0);
    initPath.llr = llr;
    initPath.metric = 0.0;
    initPath.active = true;
    paths.append(initPath);

    for (int i = 0; i < m_n; ++i) {
        QList<Path> candidates;

        for (auto& path : paths) {
            if (!path.active) continue;

            if (frozenSet.contains(i)) {
                /* 冻结位: 只选0 */
                path.bits[i] = 0;
                double absLLR = std::max(std::abs(path.llr[i]), 1e-10);
                if (path.llr[i] < 0)
                    path.metric += std::log(1.0 + std::exp(-absLLR));
                candidates.append(path);
            } else {
                /* 信息位: 分裂为两个候选 */
                for (int bit = 0; bit <= 1; ++bit) {
                    Path p = path;
                    p.bits[i] = bit;
                    double absLLR = std::max(std::abs(p.llr[i]), 1e-10);
                    if ((bit == 0 && p.llr[i] < 0) ||
                        (bit == 1 && p.llr[i] >= 0))
                        p.metric += std::log(1.0 + std::exp(-absLLR));
                    candidates.append(p);
                }
            }
        }

        /* 保留top-L条路径 */
        std::sort(candidates.begin(), candidates.end(),
                  [](const Path& a, const Path& b) {
                      return a.metric < b.metric;
                  });

        if (candidates.size() > m_listSize)
            candidates = candidates.mid(0, m_listSize);

        paths = candidates;
    }

    /* 返回最佳路径 */
    if (paths.isEmpty()) return QVector<int>(m_n, 0);
    return paths.first().bits;
}

void PolarCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
