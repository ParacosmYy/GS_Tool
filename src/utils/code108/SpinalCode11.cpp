#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include "SpinalCode11.h"

/**
 * @brief 构造函数，初始化Spinal编解码器
 * @param parent 父对象指针
 */
SpinalCode11::SpinalCode11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpinalCode11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置脊码的分段长度k
 *
 * k决定每次哈希输入的比特数，典型值4~8。
 * 较大的k提供更高频谱效率但解码复杂度指数增长。
 *
 * @param k 分段长度 (1~16)
 */
void SpinalCode11::setSpineLength(int k)
{
    m_spineK = qBound(1, k, 16);
}

/**
 * @brief 设置解码搜索树宽度
 *
 * beam width影响解码精度和复杂度的平衡。
 * 较大的搜索宽度提高解码成功率但增加计算开销。
 *
 * @param width 搜索宽度 (1~256)
 */
void SpinalCode11::setSearchWidth(int width)
{
    m_searchWidth = qBound(1, width, 256);
}

/**
 * @brief 哈希函数（Jenkins one-at-a-time变体）
 *
 * 将脊值与输入段混合，产生新的脊值。
 * 用于编码和解码过程中的脊值更新。
 *
 * @param spineValue 当前脊值
 * @param segment 输入比特段
 * @return 新的脊值
 */
static quint32 spinalHash(quint32 spineValue, quint32 segment)
{
    quint32 hash = spineValue + segment;
    hash = (hash + 0x7ed55d16) + (hash << 12);
    hash = (hash ^ 0xc761c23c) ^ (hash >> 19);
    hash = (hash + 0x165667b1) + (hash << 5);
    hash = (hash + 0xd3a2646c) ^ (hash << 9);
    hash = (hash + 0xfd7046c5) + (hash << 3);
    hash = (hash ^ 0xb55a4f09) ^ (hash >> 16);
    return hash;
}

/**
 * @brief 从哈希值生成符号
 *
 * 将哈希值映射到BPSK符号空间 {-1, +1}。
 *
 * @param hashValue 哈希值
 * @param symbolCount 输出符号数
 * @return 符号序列
 */
static QVector<int> hashToSymbols(quint32 hashValue, int symbolCount)
{
    QVector<int> symbols;
    symbols.reserve(symbolCount);
    quint32 h = hashValue;
    for (int i = 0; i < symbolCount; ++i) {
        symbols.append((h & 1) * 2 - 1); /* 0->-1, 1->+1 */
        h >>= 1;
        if (h == 0) h = hashValue ^ (quint32)i; /* 重新混入 */
    }
    return symbols;
}

/**
 * @brief Spinal编码
 *
 * 将输入比特流按k位分段，每段通过哈希函数更新脊值，
 * 每个脊值产生多个输出符号。
 *
 * @param bits 输入比特序列
 * @return 编码输出符号序列 (值域{-1, +1})
 */
QVector<int> SpinalCode11::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> symbols;
    if (bits.isEmpty()) {
        emit encodingCompleted(0);
        return symbols;
    }

    int numSegments = qCeil((double)bits.size() / m_spineK);
    quint32 spineValue = 0;
    int symbolsPerSegment = 4;

    for (int seg = 0; seg < numSegments; ++seg) {
        /* 提取k位段 */
        quint32 segment = 0;
        for (int b = 0; b < m_spineK; ++b) {
            int idx = seg * m_spineK + b;
            int bit = (idx < bits.size()) ? (bits[idx] & 1) : 0;
            segment |= (bit << b);
        }

        /* 更新脊值 */
        spineValue = spinalHash(spineValue, segment);

        /* 生成符号 */
        QVector<int> segSymbols = hashToSymbols(spineValue, symbolsPerSegment);
        symbols.append(segSymbols);
    }

    m_stats.totalEncoded++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;

    emit encodingCompleted(symbols.size());
    return symbols;
}

/**
 * @brief 计算欧氏距离
 *
 * 比较候选符号与接收符号之间的欧氏距离平方和。
 *
 * @param candidate 候选符号序列
 * @param received 接收符号序列
 * @param offset 接收序列中的起始偏移
 * @return 距离度量
 */
static double computeDistance(const QVector<int>& candidate,
                              const QVector<double>& received, int offset)
{
    double dist = 0.0;
    int len = qMin(candidate.size(), received.size() - offset);
    for (int i = 0; i < len; ++i) {
        double diff = candidate[i] - received[offset + i];
        dist += diff * diff;
    }
    return dist;
}

/**
 * @brief Spinal解码 (束搜索)
 *
 * 在k-ary树中执行beam search，每个层级枚举2^k种可能段值，
 * 保留距离最小的beamWidth条路径，最终选择最优路径还原比特流。
 *
 * @param symbols 接收符号序列
 * @param messageLength 原始消息比特长度
 * @return 解码后的比特序列
 */
QVector<int> SpinalCode11::decode(const QVector<double>& symbols, int messageLength)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (symbols.isEmpty() || messageLength <= 0) return decoded;

    int numSegments = qCeil((double)messageLength / m_spineK);
    int symbolsPerSegment = 4;
    int numCandidates = 1 << m_spineK;

    /* 束搜索候选: (脊值, 距离, 路径) */
    struct Candidate {
        quint32 spine;
        double distance;
        QVector<int> path;
    };

    QVector<Candidate> beam;
    beam.append({0, 0.0, {}});

    for (int seg = 0; seg < numSegments; ++seg) {
        QVector<Candidate> newBeam;
        int symOffset = seg * symbolsPerSegment;

        for (const auto& cand : beam) {
            for (int bits = 0; bits < numCandidates; ++bits) {
                quint32 newSpine = spinalHash(cand.spine, bits);
                QVector<int> candSymbols = hashToSymbols(newSpine, symbolsPerSegment);

                double dist = cand.distance;
                int len = qMin(symbolsPerSegment,
                               qMax(0, symbols.size() - symOffset));
                for (int i = 0; i < len; ++i) {
                    double diff = candSymbols[i] - symbols[symOffset + i];
                    dist += diff * diff;
                }

                Candidate nc;
                nc.spine = newSpine;
                nc.distance = dist;
                nc.path = cand.path;
                /* 追加k位 */
                for (int b = 0; b < m_spineK; ++b) {
                    nc.path.append((bits >> b) & 1);
                }
                newBeam.append(nc);
            }
        }

        /* 保留最优的m_searchWidth条路径 */
        std::sort(newBeam.begin(), newBeam.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.distance < b.distance;
                  });
        if (newBeam.size() > m_searchWidth) {
            newBeam.resize(m_searchWidth);
        }
        beam = newBeam;
    }

    /* 选择距离最小的路径 */
    if (!beam.isEmpty()) {
        decoded = beam.first().path;
        /* 截取到消息长度 */
        if (decoded.size() > messageLength) {
            decoded.resize(messageLength);
        }
    }

    m_stats.totalEncoded++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;

    return decoded;
}
