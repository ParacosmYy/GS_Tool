/**
 * @file SpinalCode2.cpp
 * @brief Spinal码增强实现 — 基于哈希的编码/堆栈式顺序解码器
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code32/SpinalCode2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
SpinalCode2::SpinalCode2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SpinalCode2"));
}

/**
 * @brief 设置spine长度（哈希状态数）
 * @param length spine长度（最小为1）
 */
void SpinalCode2::setSpineLength(int length)
{
    m_spineLen = qMax(1, length);
}

/**
 * @brief 设置每个符号的比特数
 * @param bits 符号位宽（最小为1，最大为16）
 */
void SpinalCode2::setSymbolSize(int bits)
{
    m_symbolSize = qBound(1, bits, 16);
}

/**
 * @brief 设置解码器遍数（堆栈深度）
 * @param passes 解码遍数（最小为1）
 */
void SpinalCode2::setNumPasses(int passes)
{
    m_numPasses = qMax(1, passes);
}

/**
 * @brief RNG辅助: 基于spine值和位置生成确定性伪随机高斯符号
 *
 * 使用简单的线性同余+Box-Muller变换产生确定性的高斯分布星座点，
 * 保证编码端和解码端使用相同的映射。
 *
 * @param spineValue 当前spine状态值
 * @param position 符号位置索引
 * @return 高斯随机值
 */
static double rngGaussian(quint32 spineValue, int position)
{
    /* LCG混合spine和position */
    quint64 s = static_cast<quint64>(spineValue) * 2654435761u + static_cast<quint64>(position) * 40503u;
    s = s ^ (s >> 16);
    s = s * 0x45d9f3bU;
    double u1 = (s & 0xFFFFFF) / 16777216.0 + 1e-10;
    s = s * 0x45d9f3bU + 0x1b56c4e9U;
    double u2 = (s & 0xFFFFFF) / 16777216.0 + 1e-10;
    return qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
}

/**
 * @brief 哈希函数: 将spine值和消息比特混合生成新spine值
 *
 * 使用FNV-1a变体混合输入。
 *
 * @param spine 当前spine状态
 * @param bits 输入消息比特（低位有效）
 * @param numBits 比特数
 * @return 新的spine状态
 */
static quint32 hashSpine(quint32 spine, int bits, int numBits)
{
    quint32 h = spine ^ 0x811c9dc5;
    for (int i = 0; i < numBits; ++i) {
        quint8 b = static_cast<quint8>((bits >> i) & 1);
        h ^= b;
        h *= 0x01000193;
    }
    return h;
}

/**
 * @brief 编码消息比特为高斯符号序列
 *
 * 将消息按symbolSize分组，逐组推进spine状态，
 * 每个spine状态通过RNG映射生成多个高斯符号。
 *
 * @param message 消息比特向量（每个元素为0或1的整数）
 * @return 高斯符号序列
 */
QVector<int> SpinalCode2::encode(const QVector<int>& message) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> symbols;
    if (message.isEmpty()) {
        return symbols;
    }

    /* 将消息比特打包为整数组 */
    int totalBits = message.size();
    int numSymbols = (totalBits + m_symbolSize - 1) / m_symbolSize;
    symbols.reserve(numSymbols * m_numPasses);

    quint32 spine = 0;
    int bitPos = 0;

    for (int s = 0; s < numSymbols; ++s) {
        /* 从消息中提取symbolSize个比特 */
        int bits = 0;
        for (int b = 0; b < m_symbolSize && bitPos < totalBits; ++b, ++bitPos) {
            if (message[bitPos]) {
                bits |= (1 << b);
            }
        }

        /* 推进spine状态 */
        spine = hashSpine(spine, bits, m_symbolSize);

        /* 为每个pass生成一个高斯符号 */
        for (int p = 0; p < m_numPasses; ++p) {
            double val = rngGaussian(spine, p);
            /* 量化为定点整数便于传输 */
            symbols.append(static_cast<int>(qRound(val * 256.0)));
        }
    }

    return symbols;
}

/**
 * @brief 使用堆栈式顺序解码器从软符号恢复消息
 *
 * 解码器维护一个候选路径栈，每步扩展所有可能的symbolSize比特组合，
 * 保留距离最优的前N条路径继续搜索，直到遍历所有spine位置。
 *
 * @param softSymbols 接收到的软符号序列
 * @param messageLength 原始消息比特长度
 * @return 解码后的消息比特向量
 */
QVector<int> SpinalCode2::decode(const QVector<double>& softSymbols, int messageLength)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded(messageLength, 0);
    if (softSymbols.isEmpty() || messageLength <= 0) {
        emit decodeComplete(0);
        return decoded;
    }

    int numSpinePositions = (messageLength + m_symbolSize - 1) / m_symbolSize;
    int symbolsPerSpine = m_numPasses;

    /* 候选路径: (累积距离, spine值, 已解码比特列表) */
    struct Path {
        double metric;
        quint32 spine;
        QVector<int> bits;
    };

    int beamWidth = 16;
    QVector<Path> currentPaths;
    currentPaths.append({0.0, 0, {}});

    for (int pos = 0; pos < numSpinePositions; ++pos) {
        QVector<Path> nextPaths;
        int numBitsThisPos = qMin(m_symbolSize, messageLength - pos * m_symbolSize);
        int numCandidates = 1 << numBitsThisPos;

        for (const auto& path : currentPaths) {
            for (int c = 0; c < numCandidates; ++c) {
                quint32 newSpine = hashSpine(path.spine, c, numBitsThisPos);
                double dist = path.metric;

                /* 计算该候选与接收符号的距离 */
                int symStart = pos * symbolsPerSpine;
                for (int p = 0; p < symbolsPerSpine; ++p) {
                    if (symStart + p < softSymbols.size()) {
                        double expected = rngGaussian(newSpine, p) * 256.0;
                        double diff = softSymbols[symStart + p] - expected;
                        dist += diff * diff;
                    }
                }

                /* 记录解码比特 */
                QVector<int> newBits = path.bits;
                for (int b = 0; b < numBitsThisPos; ++b) {
                    newBits.append((c >> b) & 1);
                }

                nextPaths.append({dist, newSpine, std::move(newBits)});
            }
        }

        /* 保留距离最小的beamWidth条路径 */
        if (nextPaths.size() > beamWidth) {
            std::partial_sort(nextPaths.begin(),
                              nextPaths.begin() + beamWidth,
                              nextPaths.end(),
                              [](const Path& a, const Path& b) {
                                  return a.metric < b.metric;
                              });
            nextPaths.resize(beamWidth);
        }

        currentPaths = std::move(nextPaths);
    }

    /* 选择最优路径 */
    if (!currentPaths.isEmpty()) {
        int bestIdx = 0;
        double bestMetric = currentPaths[0].metric;
        for (int i = 1; i < currentPaths.size(); ++i) {
            if (currentPaths[i].metric < bestMetric) {
                bestMetric = currentPaths[i].metric;
                bestIdx = i;
            }
        }
        const auto& best = currentPaths[bestIdx];
        for (int i = 0; i < qMin(messageLength, best.bits.size()); ++i) {
            decoded[i] = best.bits[i];
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += messageLength;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeComplete(messageLength);
    return decoded;
}

/**
 * @brief 重置所有累积统计信息
 */
void SpinalCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
