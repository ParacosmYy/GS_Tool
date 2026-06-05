/**
 * @file SpinalCode5.cpp
 * @brief Spinal码编解码器实现
 *
 * 实现基于哈希函数的Spinal码:
 * - 编码: 将输入比特分块，通过哈希函数生成"脊柱"(spine)值序列，
 *   再由RNG映射生成输出符号
 * - 解码: 使用beam search在可能的脊柱值空间中搜索最优路径
 * Spinal码是一种率兼容的纠错码，在短码长下接近信道容量。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code59/SpinalCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Spinal码编解码器
 * @param parent 父QObject指针
 */
SpinalCode5::SpinalCode5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置脊柱长度
 * @param n 脊柱值的比特长度 (默认 64)
 *
 * 脊柱越长，哈希空间越大，碰撞概率越低
 */
void SpinalCode5::setSpineLength(int n)
{
    m_spineLen = qMax(8, n);
}

/**
 * @brief 设置编码遍数
 * @param p 编码遍数 (默认 4)，每遍生成一组输出符号
 */
void SpinalCode5::setNumPasses(int p)
{
    m_numPasses = qMax(1, p);
}

/**
 * @brief 设置beam search宽度
 * @param w beam宽度 (默认 16)，解码时保留的候选路径数
 */
void SpinalCode5::setBeamWidth(int w)
{
    m_beamWidth = qMax(1, w);
}

/**
 * @brief Spinal码编码
 *
 * 编码流程:
 * 1. 将输入比特分成 m_spineLen 位的块
 * 2. 对每个块，将当前脊柱值与块比特通过哈希函数产生新脊柱值
 * 3. 用RNG以脊柱值为种子生成输出符号
 * 4. 重复多遍编码
 *
 * @param bits 输入比特流 (0或1)
 * @return 编码后的符号流 (整数映射)
 */
QVector<int> SpinalCode5::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> encoded;

    if (bits.isEmpty()) {
        m_stats.totalEncodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
            ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
        return encoded;
    }

    quint32 spine = 0x12345678; /* 初始脊柱值 */

    /* 将比特分成脊柱长度块并编码 */
    int bitIdx = 0;
    int blockBits = qMin(m_spineLen, 32); /* 限制为32位以匹配quint32 */

    while (bitIdx < bits.size()) {
        /* 提取一个比特块 */
        QVector<int> block;
        for (int i = 0; i < blockBits && bitIdx < bits.size(); ++i, ++bitIdx) {
            block.append(bits[bitIdx]);
        }

        /* 哈希脊柱: spine = hash(spine XOR block) */
        spine = hashSpine(spine, block);

        /* 用RNG从脊柱值生成输出符号 */
        QVector<double> symbols = rng(spine, m_numPasses);
        for (double s : symbols) {
            /* 将连续符号映射到整数 [-3, -1, 1, 3] */
            int mapped = static_cast<int>(qRound(s * 3.0));
            mapped = qBound(-3, mapped, 3);
            encoded.append(mapped);
        }
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    return encoded;
}

/**
 * @brief Spinal码解码 (beam search)
 *
 * 解码流程:
 * 1. 对每个脊柱位置，扩展所有beam候选
 * 2. 对每个候选，计算与接收符号的欧氏距离作为度量
 * 3. 保留度量最优的beamWidth条路径
 * 4. 最终选择最优路径回溯得到解码比特
 *
 * @param symbols 接收的符号流
 * @return 解码后的比特流
 */
QVector<int> SpinalCode5::decode(const QVector<double>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    bool converged = false;

    if (symbols.isEmpty()) {
        m_stats.totalDecodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
            ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
        emit decodeCompleted(0, false);
        return decoded;
    }

    /* beam search结构: (度量, 脊柱值, 已解码比特) */
    struct Beam {
        double metric;
        quint32 spine;
        QVector<int> bits;
    };

    QVector<Beam> beams;
    beams.append({0.0, 0x12345678, {}}); /* 初始beam */

    int blockBits = qMin(m_spineLen, 32);
    int symPerBlock = m_numPasses; /* 每个脊柱位置生成的符号数 */
    int totalBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(symbols.size()) / symPerBlock)));

    /* 逐块进行beam search */
    int symIdx = 0;
    for (int block = 0; block < totalBlocks && symIdx < symbols.size(); ++block) {
        QVector<Beam> newBeams;

        /* 对每个现有beam，尝试所有可能的比特块 */
        for (const Beam& beam : beams) {
            /* 尝试几种代表性比特组合 */
            for (int trial = 0; trial < 4; ++trial) {
                QVector<int> trialBits;
                for (int b = 0; b < blockBits; ++b) {
                    trialBits.append((trial >> (b % 2)) & 1);
                }

                /* 计算新的脊柱值 */
                quint32 newSpine = hashSpine(beam.spine, trialBits);

                /* 生成参考符号并计算度量 */
                QVector<double> refSymbols = rng(newSpine, m_numPasses);
                double metric = beam.metric;
                for (int s = 0; s < m_numPasses && symIdx + s < symbols.size(); ++s) {
                    double diff = symbols[symIdx + s] - refSymbols[s] * 3.0;
                    metric += diff * diff;
                }

                Beam newBeam;
                newBeam.metric = metric;
                newBeam.spine = newSpine;
                newBeam.bits = beam.bits;
                newBeam.bits.append(trialBits);
                newBeams.append(newBeam);
            }
        }

        /* 按度量排序，保留最优的beamWidth条 */
        std::sort(newBeams.begin(), newBeams.end(),
            [](const Beam& a, const Beam& b) { return a.metric < b.metric; });

        if (newBeams.size() > m_beamWidth) {
            newBeams.resize(m_beamWidth);
        }

        beams = newBeams;
        symIdx += symPerBlock;
    }

    /* 选择最优beam */
    if (!beams.isEmpty()) {
        decoded = beams[0].bits;
        converged = (beams.size() > 0);
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(m_numPasses, converged);
    return decoded;
}

/**
 * @brief 重置所有统计数据
 */
void SpinalCode5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 哈希脊柱函数
 *
 * 使用简单的混合哈希: 将比特块混合进脊柱值
 * spine = (spine * 1103515245 + 12345) XOR block_value
 *
 * @param spine 当前脊柱值
 * @param bits 输入比特块
 * @return 更新后的脊柱值
 */
quint32 SpinalCode5::hashSpine(quint32 spine, const QVector<int>& bits)
{
    /* 将比特块打包为整数 */
    quint32 blockVal = 0;
    for (int i = 0; i < bits.size() && i < 32; ++i) {
        if (bits[i]) {
            blockVal |= (1u << i);
        }
    }

    /* 哈希混合 */
    spine ^= blockVal;
    spine = spine * 1103515245u + 12345u;
    spine ^= (spine >> 16);
    spine *= 2654435761u;

    return spine;
}

/**
 * @brief 伪随机数生成器 (以脊柱值为种子)
 *
 * 使用线性同余生成器产生[-1, 1]范围内的均匀分布值
 *
 * @param seed 种子值 (脊柱值)
 * @param count 生成值的数量
 * @return 伪随机数序列
 */
QVector<double> SpinalCode5::rng(quint32 seed, int count)
{
    QVector<double> result(count);
    quint32 state = seed;

    for (int i = 0; i < count; ++i) {
        state = state * 1103515245u + 12345u;
        /* 映射到 [-1, 1] */
        result[i] = (static_cast<double>(state & 0x7FFFFFFF) / 0x7FFFFFFF) * 2.0 - 1.0;
    }

    return result;
}
