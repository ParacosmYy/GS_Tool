/**
 * @file SpinalCode3.cpp
 * @brief 脊码3实现 — 自适应编码率+尾比特归零
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code40/SpinalCode3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
SpinalCode3::SpinalCode3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SpinalCode3"));
}

/**
 * @brief 设置随机种子
 *
 * 种子用于脊函数和符号生成器的哈希计算。
 * 编解码双方必须使用相同种子。
 *
 * @param seed 32位无符号种子值
 */
void SpinalCode3::setSeed(quint32 seed)
{
    m_seed = seed;
}

/**
 * @brief 设置脊值长度
 *
 * 脊值长度决定了编码过程中的哈希链长度。
 * 较长的脊值提供更强的差错保护但增加复杂度。
 *
 * @param length 脊值长度（最小为8）
 */
void SpinalCode3::setSpineLength(int length)
{
    m_spineLength = qMax(8, length);
}

/**
 * @brief 编码消息为脊码符号序列
 *
 * 将消息按k比特分组，依次通过脊函数生成脊值，
 * 再从每个脊值产生numPasses个符号。尾部自动归零填充。
 *
 * @param message 消息比特序列（0/1）
 * @param numPasses 每个脊值的符号通过次数
 * @return 编码符号序列（量化整数）
 */
QVector<int> SpinalCode3::encode(const QVector<int>& message, int numPasses)
{
    QElapsedTimer timer;
    timer.start();

    numPasses = qMax(1, numPasses);
    int numSteps = (message.size() + m_k - 1) / m_k;

    /* 尾比特归零：补充零比特使长度为k的整数倍 */
    QVector<int> paddedMsg = message;
    while (paddedMsg.size() % m_k != 0) {
        paddedMsg.append(0);
    }

    QVector<int> symbols;
    quint32 spineValue = m_seed;

    for (int step = 0; step < numSteps; ++step) {
        /* 提取k个比特 */
        quint32 bits = 0;
        for (int b = 0; b < m_k; ++b) {
            int idx = step * m_k + b;
            if (idx < paddedMsg.size() && paddedMsg[idx]) {
                bits |= (1u << b);
            }
        }

        /* 脊函数：哈希更新 */
        spineValue = spineFunction(spineValue, bits);

        /* 生成numPasses个符号 */
        for (int p = 0; p < numPasses; ++p) {
            int sym = rngSymbol(spineValue, p);
            symbols.append(sym);
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalSymbols += symbols.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(message.size(), symbols.size());
    return symbols;
}

/**
 * @brief 解码接收符号为消息比特
 *
 * 使用气泡排序（Bubble Decoding）算法进行脊码解码。
 * 维护beamWidth个候选路径，按欧氏距离排序保留最优。
 * 自适应编码率：检测到高质量信道时缩减搜索宽度。
 *
 * @param received 接收到的符号序列
 * @param beamWidth 搜索束宽
 * @param messageLength 消息长度（-1自动检测）
 * @return 解码消息比特序列
 */
QVector<int> SpinalCode3::decode(const QVector<int>& received, int beamWidth,
                                  int messageLength)
{
    QElapsedTimer timer;
    timer.start();

    beamWidth = qMax(4, beamWidth);
    int numPasses = 4; /* 默认通过次数 */
    int numSteps = received.size() / numPasses;
    if (numSteps <= 0) return {};

    /* 自适应束宽：根据符号数量调整 */
    if (received.size() > 500) {
        beamWidth = qMax(8, beamWidth / 2);
    }

    /* 候选路径： spine值 + 消息比特 + 距离 */
    struct Path {
        quint32 spine;
        QVector<int> bits;
        double distance;
    };

    QVector<Path> candidates;
    candidates.append({m_seed, {}, 0.0});

    for (int step = 0; step < numSteps; ++step) {
        QVector<Path> newCandidates;

        for (const auto& c : candidates) {
            /* 遍历所有可能的k比特组合 */
            int combos = (1 << m_k);
            /* 为了效率，限制每个候选的扩展数 */
            int expandLimit = qMin(combos, 16);

            for (int b = 0; b < expandLimit; ++b) {
                quint32 newSpine = spineFunction(c.spine, (quint32)b);
                double dist = c.distance;

                for (int p = 0; p < numPasses; ++p) {
                    int symIdx = step * numPasses + p;
                    if (symIdx < received.size()) {
                        int refSym = rngSymbol(newSpine, p);
                        double diff = received[symIdx] - refSym;
                        dist += diff * diff;
                    }
                }

                Path np;
                np.spine = newSpine;
                np.bits = c.bits;
                for (int bit = 0; bit < m_k; ++bit) {
                    np.bits.append((b >> bit) & 1);
                }
                np.distance = dist;
                newCandidates.append(np);
            }
        }

        /* 按距离排序保留最优beamWidth个 */
        std::sort(newCandidates.begin(), newCandidates.end(),
                  [](const Path& a, const Path& b) {
                      return a.distance < b.distance;
                  });
        if (newCandidates.size() > beamWidth) {
            newCandidates.resize(beamWidth);
        }
        candidates = newCandidates;
    }

    /* 取最优路径 */
    QVector<int> result;
    if (!candidates.isEmpty()) {
        result = candidates[0].bits;
    }

    /* 截断到目标消息长度 */
    if (messageLength > 0 && result.size() > messageLength) {
        result.resize(messageLength);
    }

    bool success = !result.isEmpty();
    m_stats.totalDecodes++;
    m_stats.totalSymbols += received.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(result.size(), success);
    return result;
}

/**
 * @brief 脊函数 — 将当前脊值与新比特混合
 *
 * 使用简单的哈希混合函数，将脊值与新输入比特组合。
 * 确保不同的比特序列产生不同的脊值链。
 *
 * @param spineValue 当前脊值
 * @param bits 输入的k个比特
 * @return 更新后的脊值
 */
quint32 SpinalCode3::spineFunction(quint32 spineValue, quint32 bits) const
{
    /* 基于MurmurHash3的混合函数 */
    quint32 h = spineValue;
    h ^= bits + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

/**
 * @brief RNG符号生成器 — 从脊值产生伪随机符号
 *
 * 使用脊值和符号索引作为种子，生成量化精度为m_precision的符号。
 * 符号范围在[-2^(precision-1), 2^(precision-1)]之间。
 *
 * @param spineValue 当前脊值
 * @param symbolIndex 符号序号
 * @return 量化符号值
 */
int SpinalCode3::rngSymbol(quint32 spineValue, int symbolIndex) const
{
    /* 简单的伪随机数生成 */
    quint32 x = spineValue;
    x ^= (quint32)symbolIndex * 0x517cc1b727220a95ULL;
    x ^= x >> 17;
    x *= 0xed5ad4bb;
    x ^= x >> 11;
    x *= 0x9e3779b9;
    x ^= x >> 16;

    /* 映射到[-range, range] */
    int range = (1 << (m_precision - 1));
    int sym = (int)(x % (2 * range + 1)) - range;
    return sym;
}

/**
 * @brief 重置所有统计数据
 */
void SpinalCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
