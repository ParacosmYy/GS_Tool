#include "SpinalCode10.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SpinalCode10.cpp
 * @brief Spinal码编解码器实现
 *
 * Spinal码是一种无率码(Rateless Code):
 * - 编码: 将消息分为k-bit的脊柱段，通过哈希函数映射为编码符号
 * - 解码: 使用气泡解码器(Bubble Decoder)在脊柱树上搜索最大似然路径
 */

/**
 * @brief 构造函数，初始化默认脊柱长度
 * @param parent 父QObject对象指针
 */
SpinalCode10::SpinalCode10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置脊柱长度
 * @param length 每个脊柱段包含的信息比特数
 */
void SpinalCode10::setSpineLength(int length)
{
    m_spineLength = qMax(1, length);
}

/**
 * @brief 简单哈希函数(用于脊柱编码)
 * @param spine 脊柱值
 * @param hashIdx 哈希输出索引
 * @return 哈希值(0或1)
 */
static int spineHash(int spine, int hashIdx)
{
    // 简化的哈希函数(实际实现使用更强的哈希)
    unsigned int h = static_cast<unsigned int>(spine) * 2654435761u;
    h ^= static_cast<unsigned int>(hashIdx) * 2246822519u;
    h = (h ^ (h >> 16)) * 0x45d9f3bu;
    h = (h ^ (h >> 16)) * 0x45d9f3bu;
    return (h ^ (h >> 16)) & 1;
}

/**
 * @brief 编码整数序列
 *
 * 编码流程:
 * 1. 将输入数据按k-bit分组
 * 2. 每组的哈希值与前一组的哈希值级联(脊柱)
 * 3. 通过哈希函数将脊柱值映射为编码符号流
 *
 * @param data 输入比特序列
 * @return 编码输出符号序列
 */
QVector<int> SpinalCode10::encode(const QVector<int>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    QVector<int> symbols;
    int spineValue = 0;

    // 按脊柱长度分组处理
    for (int i = 0; i < data.size(); i += m_spineLength) {
        // 计算当前脊柱值: hash(前脊柱值, 当前k-bit消息)
        int messageBits = 0;
        for (int j = 0; j < m_spineLength && i + j < data.size(); ++j) {
            messageBits |= (data[i + j] << j);
        }

        // 更新脊柱值
        unsigned int h = static_cast<unsigned int>(spineValue);
        h ^= static_cast<unsigned int>(messageBits) * 2654435761u;
        h = (h ^ (h >> 16)) * 0x45d9f3bu;
        spineValue = static_cast<int>(h);

        // 从脊柱值生成多个编码符号
        const int symbolsPerSpine = 4;
        for (int s = 0; s < symbolsPerSpine; ++s) {
            symbols.append(spineHash(spineValue, s));
        }
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit codingCompleted(symbols.size());
    return symbols;
}

/**
 * @brief 解码为整数序列
 *
 * 使用气泡解码器: 在每个脊柱节点保留B个候选路径，
 * 按欧氏距离选择最可能的k-bit消息。
 *
 * @param symbols 接收到的编码符号(实数软比特)
 * @return 解码后的比特序列
 */
QVector<int> SpinalCode10::decode(const QVector<double>& symbols)
{
    if (symbols.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int B = 16; // 保留候选路径数
    const int symbolsPerSpine = 4;
    const int numSpines = symbols.size() / symbolsPerSpine;

    QVector<int> decoded;

    // 气泡解码: 逐脊柱段处理
    QVector<QPair<double, int>> candidates; // (距离, 脊柱值)
    candidates.append(qMakePair(0.0, 0));

    for (int s = 0; s < numSpines && !candidates.isEmpty(); ++s) {
        QVector<QPair<double, int>> newCandidates;

        for (const auto& cand : candidates) {
            // 尝试所有可能的k-bit消息
            const int numMessages = (1 << m_spineLength);
            for (int msg = 0; msg < numMessages; ++msg) {
                // 计算新脊柱值
                unsigned int h = static_cast<unsigned int>(cand.second);
                h ^= static_cast<unsigned int>(msg) * 2654435761u;
                h = (h ^ (h >> 16)) * 0x45d9f3bu;
                int newSpine = static_cast<int>(h);

                // 计算与接收符号的距离
                double dist = cand.first;
                for (int j = 0; j < symbolsPerSpine; ++j) {
                    const int symIdx = s * symbolsPerSpine + j;
                    if (symIdx < symbols.size()) {
                        const double expected = spineHash(newSpine, j) ? 1.0 : -1.0;
                        dist += (symbols[symIdx] - expected) * (symbols[symIdx] - expected);
                    }
                }

                newCandidates.append(qMakePair(dist, newSpine));

                // 解码消息比特
                if (s == numSpines - 1 || newCandidates.size() >= B) {
                    for (int b = 0; b < m_spineLength; ++b) {
                        decoded.append((msg >> b) & 1);
                    }
                }
            }
        }

        // 保留距离最小的B条候选
        std::sort(newCandidates.begin(), newCandidates.end());
        if (newCandidates.size() > B) {
            newCandidates.resize(B);
        }
        candidates = newCandidates;
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit codingCompleted(decoded.size());
    return decoded;
}

/**
 * @brief 重置所有统计信息
 */
void SpinalCode10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
