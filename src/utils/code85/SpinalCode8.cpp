#include "SpinalCode8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <random>

/**
 * @class SpinalCode8
 * @brief Spinal码编码器/解码器实现
 *
 * Spinal码是一种基于散列函数的率兼容纠错编码。
 * 编码过程将消息分为k位小块，每块通过散列函数产生
 * 无限长的编码符号。解码使用堆栈式搜索逐步恢复消息。
 *
 * 特点: 短包场景下接近Shannon极限的性能，码率可灵活调整。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SpinalCode8::SpinalCode8(QObject* parent)
    : QObject(parent)
    , m_spineBits(8)
{
}

/**
 * @brief 简单散列函数(用于Spinal码的脊柱函数)
 *
 * 将脊柱值和消息块组合产生新的脊柱值和输出符号。
 * 使用简单的线性同余+位移混合实现。
 *
 * @param spine 当前的脊柱值
 * @param msgBits 消息块(整数形式)
 * @param rngSeed 随机种子
 * @return 散列输出值
 */
static double spinalHash(quint32 spine, int msgBits, quint32 rngSeed)
{
    quint32 h = spine ^ static_cast<quint32>(msgBits);
    h ^= rngSeed;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    /* 映射到[-1, 1]范围 */
    return (static_cast<double>(h & 0xFFFF) / 65536.0) * 2.0 - 1.0;
}

/**
 * @brief 编码数据
 *
 * 将输入数据按spineValueBits位分块，每块通过散列函数
 * 生成脊柱值链，然后从每个脊柱值产生多个编码符号。
 * 最终编码符号为BPSK映射的实数值。
 *
 * @param data 输入数据(每个元素为0或1的比特序列)
 * @param spineValueBits 每个脊柱块包含的比特数
 * @return 编码后的符号序列(BPSK调制)
 */
QVector<double> SpinalCode8::encode(const QVector<int>& data, int spineValueBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> encoded;

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        return encoded;
    }

    m_spineBits = spineValueBits;
    quint32 spine = 0;
    quint32 rngSeed = 0x12345678;

    /* 逐块处理 */
    int bitsPerBlock = spineValueBits;
    for (int i = 0; i < data.size(); i += bitsPerBlock) {
        /* 将一组比特组合为整数 */
        int msgBits = 0;
        for (int b = 0; b < bitsPerBlock && (i + b) < data.size(); ++b) {
            msgBits |= (data[i + b] & 1) << b;
        }

        /* 更新脊柱值 */
        spine = spine ^ static_cast<quint32>(msgBits);
        spine = ((spine >> 16) ^ spine) * 0x45d9f3b;
        spine = (spine >> 16) ^ spine;

        /* 从脊柱值产生多个编码符号 */
        const int symbolsPerSpine = 4;
        for (int s = 0; s < symbolsPerSpine; ++s) {
            double symbol = spinalHash(spine, s, rngSeed);
            encoded.append(symbol);
        }
    }

    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    return encoded;
}

/**
 * @brief 解码数据(基于堆栈解码器)
 *
 * 使用深度优先搜索的堆栈解码器逐步恢复消息。
 * 每一步保留置信度最高的若干候选路径，通过比较
 * 接收符号与候选符号的欧氏距离筛选最优路径。
 *
 * @param received 接收到的符号序列(含噪声)
 * @param spineValueBits 脊柱块比特数
 * @param depth 搜索深度(候选路径数)
 * @return 解码后的比特序列
 */
QVector<int> SpinalCode8::decode(const QVector<double>& received, int spineValueBits, int depth)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;

    if (received.isEmpty()) {
        m_timeSum += timer.elapsed();
        return decoded;
    }

    int bitsPerBlock = spineValueBits;
    int symbolsPerSpine = 4;
    int numSpines = received.size() / symbolsPerSpine;

    quint32 rngSeed = 0x12345678;

    /* 堆栈式解码: 保留top候选路径 */
    struct Candidate {
        quint32 spine;
        QVector<int> message;
        double cost;
    };

    QVector<Candidate> stack;
    stack.append({0, {}, 0.0});

    for (int s = 0; s < numSpines; ++s) {
        QVector<Candidate> newStack;

        for (const auto& cand : stack) {
            /* 尝试所有可能的消息块 */
            int numMsgs = 1 << bitsPerBlock;
            for (int msg = 0; msg < numMsgs; ++msg) {
                quint32 newSpine = cand.spine ^ static_cast<quint32>(msg);
                newSpine = ((newSpine >> 16) ^ newSpine) * 0x45d9f3b;
                newSpine = (newSpine >> 16) ^ newSpine;

                /* 计算距离代价 */
                double cost = cand.cost;
                for (int sym = 0; sym < symbolsPerSpine; ++sym) {
                    double expected = spinalHash(newSpine, sym, rngSeed);
                    int idx = s * symbolsPerSpine + sym;
                    if (idx < received.size()) {
                        double diff = received[idx] - expected;
                        cost += diff * diff;
                    }
                }

                Candidate newCand;
                newCand.spine = newSpine;
                newCand.message = cand.message;
                /* 分解消息比特 */
                for (int b = 0; b < bitsPerBlock; ++b) {
                    newCand.message.append((msg >> b) & 1);
                }
                newCand.cost = cost;
                newStack.append(newCand);
            }
        }

        /* 保留代价最小的depth个候选 */
        std::sort(newStack.begin(), newStack.end(),
                  [](const Candidate& a, const Candidate& b) { return a.cost < b.cost; });
        if (newStack.size() > depth) {
            newStack.resize(depth);
        }

        stack = newStack;
    }

    /* 返回最优路径的消息 */
    if (!stack.isEmpty()) {
        decoded = stack[0].message;
    }

    m_stats.totalBlocksDecoded++;
    double confidence = stack.isEmpty() ? 0.0 : qExp(-stack[0].cost);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, confidence);

    return decoded;
}

/**
 * @brief 重置所有统计数据
 *
 * 将编码/解码计数和计时归零。
 */
void SpinalCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
