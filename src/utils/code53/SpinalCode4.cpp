/**
 * @file SpinalCode4.cpp
 * @brief Spinal码编解码器实现，基于哈希函数的速率兼容码
 *
 * Spinal码是一种新型的速率兼容纠错码，利用哈希函数将消息
 * 分段映射到符号序列。解码端使用基于树的搜索算法（beam search）
 * 逐步恢复原始消息。
 *
 * 特点：
 * - 无限精度的速率兼容性
 * - 基于哈希函数的确定性映射
 * - 解码复杂度与消息长度线性相关
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code53/SpinalCode4.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认种子
 * @param parent 父QObject对象指针
 */
SpinalCode4::SpinalCode4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置随机数生成器种子
 * @param seed 种子值
 */
void SpinalCode4::setSeed(quint32 seed)
{
    m_seed = seed;
}

/**
 * @brief 脊椎函数(Spine Function)：将状态和输入比特映射到新状态
 *
 * 使用简单的哈希混合函数：
 * h(s, bits) = rotate(s XOR bits) * constant + bits
 *
 * @param sv 当前脊椎状态值
 * @param bits 输入的m_k比特数据（打包为32位整数）
 * @return 新的脊椎状态值
 */
quint32 SpinalCode4::spineFn(quint32 sv, quint32 bits) const
{
    /* 哈希混合：使用乘法和旋转 */
    quint32 x = sv ^ bits;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

/**
 * @brief RNG符号映射函数：从脊椎状态生成星座符号
 *
 * 利用脊椎状态作为伪随机种子，通过RNG映射到
 * 固定范围的整数符号值。
 *
 * @param sv 脊椎状态值
 * @param idx 符号索引（同一状态可生成多个符号）
 * @return 映射的符号值（0-255范围）
 */
int SpinalCode4::rngSym(quint32 sv, int idx) const
{
    /* 使用脊椎状态和索引生成伪随机符号 */
    quint32 h = sv ^ (idx * 0x9e3779b9);
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    return static_cast<int>(h & 0xFF);
}

/**
 * @brief 编码消息为Spinal码符号序列
 *
 * 编码流程：
 * 1. 将消息按m_k比特分组
 * 2. 每组通过spineFn更新脊椎状态
 * 3. 从每个状态生成多个符号（passes决定符号数）
 *
 * @param msg 待编码的消息比特向量（0/1）
 * @param passes 每个脊椎段生成的符号数，默认4
 * @return 编码后的符号序列
 */
QVector<int> SpinalCode4::encode(const QVector<int>& msg, int passes)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> symbols;

    /* 按m_k比特分组处理消息 */
    int numSegments = (msg.size() + m_k - 1) / m_k;
    quint32 sv = m_seed;

    for (int seg = 0; seg < numSegments; ++seg) {
        /* 提取m_k比特并打包为32位整数 */
        quint32 bits = 0;
        for (int b = 0; b < m_k; ++b) {
            int idx = seg * m_k + b;
            if (idx < msg.size() && msg[idx])
                bits |= (1u << b);
        }

        /* 更新脊椎状态 */
        sv = spineFn(sv, bits);

        /* 从当前状态生成passes个符号 */
        for (int p = 0; p < passes; ++p) {
            symbols.append(rngSym(sv, p));
        }
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return symbols;
}

/**
 * @brief 解码Spinal码符号序列
 *
 * 使用Beam Search算法在脊椎状态空间中搜索：
 * 1. 对每个脊椎段，扩展所有可能的m_k比特组合
 * 2. 计算每种扩展与接收符号的欧氏距离
 * 3. 保留距离最小的beamWidth条路径
 * 4. 最终选择距离最小的路径作为解码结果
 *
 * @param rx 接收到的符号序列
 * @param beamWidth 搜索宽度，越大精度越高但越慢，默认16
 * @param msgLen 原始消息长度（比特数），-1表示自动推断
 * @return 解码后的消息比特向量
 */
QVector<int> SpinalCode4::decode(const QVector<int>& rx, int beamWidth, int msgLen)
{
    QElapsedTimer timer;
    timer.start();

    int passes = 4;
    if (msgLen < 0) {
        /* 自动推断消息长度 */
        int numSegments = rx.size() / passes;
        msgLen = numSegments * m_k;
    }

    int numSegments = (msgLen + m_k - 1) / m_k;
    int bitsPerSeg = (1 << m_k); /* 2^k种可能的比特组合 */
    beamWidth = qMax(1, beamWidth);

    /* 候选路径：(脊椎状态, 路径代价, 解码比特) */
    struct Path {
        quint32 spine;
        double cost;
        QVector<int> bits;
    };

    QVector<Path> candidates;
    candidates.append({m_seed, 0.0, {}});

    for (int seg = 0; seg < numSegments; ++seg) {
        QVector<Path> newCandidates;

        /* 提取当前段对应的接收符号 */
        int symStart = seg * passes;
        int symEnd = qMin(symStart + passes, rx.size());

        for (const auto& path : candidates) {
            /* 枚举所有2^k种可能的比特组合 */
            for (int b = 0; b < bitsPerSeg; ++b) {
                quint32 newSpine = spineFn(path.spine, static_cast<quint32>(b));

                /* 计算路径代价（欧氏距离） */
                double segCost = 0.0;
                for (int p = 0; p < symEnd - symStart; ++p) {
                    int sym = rngSym(newSpine, p);
                    int rxIdx = symStart + p;
                    if (rxIdx < rx.size()) {
                        double diff = static_cast<double>(sym) - rx[rxIdx];
                        segCost += diff * diff;
                    }
                }

                /* 构建新路径 */
                Path newPath;
                newPath.spine = newSpine;
                newPath.cost = path.cost + segCost;
                newPath.bits = path.bits;

                /* 解包比特 */
                int bitsToAdd = qMin(m_k, msgLen - seg * m_k);
                for (int i = 0; i < bitsToAdd; ++i) {
                    newPath.bits.append((b >> i) & 1);
                }

                newCandidates.append(newPath);
            }
        }

        /* 按代价排序，保留前beamWidth条路径 */
        std::sort(newCandidates.begin(), newCandidates.end(),
                  [](const Path& a, const Path& b) { return a.cost < b.cost; });

        if (newCandidates.size() > beamWidth)
            newCandidates.resize(beamWidth);

        candidates = newCandidates;
    }

    /* 选择代价最小的路径 */
    QVector<int> result;
    bool ok = false;
    if (!candidates.isEmpty()) {
        result = candidates[0].bits;
        ok = (result.size() >= static_cast<int>(msgLen));
        result.resize(msgLen);
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit decodeCompleted(msgLen, ok);
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void SpinalCode4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
