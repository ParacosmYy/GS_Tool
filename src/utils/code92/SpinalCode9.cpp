#include "SpinalCode9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Spinal码编解码器
 * @param parent 父对象指针
 */
SpinalCode9::SpinalCode9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置脊柱长度
 * @param length 脊柱序列的长度
 */
void SpinalCode9::setSpineLength(int length)
{
    m_spineLength = qMax(1, length);
}

/**
 * @brief 简化哈希函数，用于生成脊柱值
 * @param value 输入值
 * @return 哈希输出
 */
static quint32 spinalHash(quint32 value)
{
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
    return value;
}

/**
 * @brief 对整数序列执行Spinal编码
 *
 * 将输入数据按k位分组，每组通过哈希函数生成脊柱值，
 * 再从脊柱值派生多个输出符号，实现速率兼容编码。
 *
 * @param data 输入整数序列
 */
void SpinalCode9::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    int k = 4; /* 每次哈希消耗的比特数 */
    int numPasses = 3; /* 每个脊柱值的编码通过数 */
    int symbolCount = 0;

    quint32 spineValue = 0;
    int spinePos = 0;
    quint32 accumulated = 0;
    int bitPos = 0;

    for (int val : data) {
        accumulated |= (static_cast<quint32>(val & 0xFF) << bitPos);
        bitPos += 8;

        while (bitPos >= k) {
            /* 提取k位输入到哈希函数 */
            quint32 input = accumulated & ((1u << k) - 1);
            accumulated >>= k;
            bitPos -= k;

            /* 更新脊柱值 */
            spineValue = spinalHash(spineValue ^ input);
            spinePos++;

            /* 从脊柱值生成输出符号 */
            for (int p = 0; p < numPasses; ++p) {
                quint32 symbol = spinalHash(spineValue + p * 12345);
                symbolCount++;
            }

            if (spinePos >= m_spineLength) break;
        }
        if (spinePos >= m_spineLength) break;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(symbolCount);
}

/**
 * @brief 对软信息执行Spinal解码
 *
 * 使用气泡解码器(Balloon Decoder)：对每个脊柱位置
 * 维护一组候选路径，通过欧氏距离度量选择最优路径。
 *
 * @param symbols 接收的软信息符号序列
 */
void SpinalCode9::decode(const QVector<double>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    if (symbols.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    int k = 4;
    int beamWidth = 16; /* 候选路径数量 */
    int numPasses = 3;

    /* 初始化：单条空路径 */
    struct Path {
        quint32 spineValue = 0;
        double metric = 0.0;
        QVector<int> decodedBits;
    };

    QVector<Path> candidates;
    candidates.append(Path());

    for (int spinePos = 0; spinePos < m_spineLength; ++spinePos) {
        QVector<Path> newCandidates;

        for (const auto& path : candidates) {
            /* 尝试所有可能的k位输入 */
            int numInputs = (1 << k);
            for (int input = 0; input < numInputs; ++input) {
                quint32 newSpine = spinalHash(path.spineValue ^ static_cast<quint32>(input));

                Path newPath = path;
                newPath.spineValue = newSpine;

                /* 计算与接收符号的距离度量 */
                double dist = 0.0;
                for (int p = 0; p < numPasses; ++p) {
                    int symIdx = spinePos * numPasses + p;
                    if (symIdx < symbols.size()) {
                        quint32 expected = spinalHash(newSpine + p * 12345);
                        double diff = symbols[symIdx] - static_cast<double>(expected);
                        dist += diff * diff;
                    }
                }
                newPath.metric += dist;
                newPath.decodedBits.append(input);
                newCandidates.append(newPath);
            }
        }

        /* 保留最好的beamWidth条路径 */
        std::sort(newCandidates.begin(), newCandidates.end(),
                  [](const Path& a, const Path& b) { return a.metric < b.metric; });
        if (newCandidates.size() > beamWidth) {
            newCandidates.resize(beamWidth);
        }
        candidates = newCandidates;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(symbols.size());
}

/**
 * @brief 重置统计数据
 */
void SpinalCode9::resetStatistics()
{
    m_stats.totalEncoded = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
