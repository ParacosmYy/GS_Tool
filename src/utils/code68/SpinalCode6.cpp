/**
 * @file SpinalCode6.cpp
 * @brief Spinal码编解码器实现
 *
 * 实现基于哈希函数的Spinal码，支持编码和基于束搜索的解码。
 * Spinal码是一种无速率码，适用于渐进式传输场景。
 */

#include "utils/code68/SpinalCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SpinalCode6::SpinalCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置spine值长度
 * @param n spine段数
 */
void SpinalCode6::setSpineLength(int n)
{
    m_spineLen = qMax(1, n);
}

/**
 * @brief 设置束搜索宽度
 * @param w 束宽度
 */
void SpinalCode6::setBeamWidth(int w)
{
    m_beamWidth = qMax(1, w);
}

/**
 * @brief 设置解码遍数
 * @param p 遍数
 */
void SpinalCode6::setNumPasses(int p)
{
    m_numPasses = qMax(1, p);
}

/**
 * @brief 设置每段spine的比特数
 * @param k 每段比特数
 */
void SpinalCode6::setK(int k)
{
    m_k = qBound(1, k, 32);
}

/**
 * @brief 编码信息比特
 * @param bits 输入信息比特
 * @return 编码后的符号序列
 */
QVector<double> SpinalCode6::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    int numSpines = bits.size() / m_k;
    QVector<double> symbols;

    for (int i = 0; i < numSpines; ++i) {
        // 提取k位作为当前spine输入
        QVector<int> spineBits(m_k);
        for (int j = 0; j < m_k; ++j) {
            spineBits[j] = bits[i * m_k + j];
        }

        // 计算spine值（使用哈希函数）
        quint32 spine = hashFunc(static_cast<quint32>(i), spineBits);

        // 使用spine值作为RNG种子生成多个符号
        QVector<double> syms = rngFunc(spine, 8);
        for (double s : syms) {
            symbols.append(s);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return symbols;
}

/**
 * @brief 解码接收到的符号
 * @param symbols 接收到的符号序列
 * @return 解码后的比特序列
 */
QVector<int> SpinalCode6::decode(const QVector<double>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    int numSpines = m_spineLen;
    int symsPerSpine = 8;
    if (symbols.size() < numSpines * symsPerSpine) {
        symsPerSpine = symbols.size() / qMax(numSpines, 1);
    }

    // 束搜索：维护m_beamWidth个候选路径
    struct Candidate {
        quint32 spine;
        QVector<int> bits;
        double metric;
    };

    QVector<Candidate> beam;
    beam.append({0, {}, 0.0});

    for (int i = 0; i < numSpines; ++i) {
        QVector<Candidate> newBeam;
        int numBits = qMin(m_k, 16); // 每段尝试的比特数

        // 对每个候选路径，枚举所有可能的k位组合
        int combos = qMin(1 << numBits, 256); // 限制组合数

        for (const auto& cand : beam) {
            for (int b = 0; b < combos; ++b) {
                QVector<int> bits(m_k, 0);
                for (int j = 0; j < m_k && j < numBits; ++j) {
                    bits[j] = (b >> j) & 1;
                }

                quint32 newSpine = hashFunc(cand.spine, bits);

                // 计算当前段度量
                QVector<double> expected = rngFunc(newSpine, symsPerSpine);
                double metric = cand.metric;
                int baseIdx = i * 8;
                for (int s = 0; s < symsPerSpine && (baseIdx + s) < symbols.size(); ++s) {
                    double diff = symbols[baseIdx + s] - expected[s];
                    metric -= diff * diff;
                }

                QVector<int> newBits = cand.bits;
                for (int bit : bits) newBits.append(bit);
                newBeam.append({newSpine, newBits, metric});
            }
        }

        // 保留top beamWidth个候选
        std::partial_sort(newBeam.begin(),
                          newBeam.begin() + qMin(m_beamWidth, newBeam.size()),
                          newBeam.end(),
                          [](const Candidate& a, const Candidate& b) {
                              return a.metric > b.metric;
                          });
        newBeam.resize(qMin(m_beamWidth, newBeam.size()));
        beam = newBeam;
    }

    // 选择最优路径
    QVector<int> result;
    if (!beam.isEmpty()) {
        result = beam[0].bits;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(m_numPasses, !beam.isEmpty());
    return result;
}

/**
 * @brief 重置统计信息
 */
void SpinalCode6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 哈希函数：将spine值和比特映射为新的spine值
 * @param spine 当前spine值
 * @param bits 输入比特
 * @return 新的spine值
 */
quint32 SpinalCode6::hashFunc(quint32 spine, const QVector<int>& bits)
{
    quint32 h = spine;
    // 将bits打包为整数
    quint32 packed = 0;
    for (int i = 0; i < bits.size() && i < 32; ++i) {
        packed |= (static_cast<quint32>(bits[i] & 1) << i);
    }
    // MurmurHash3混合
    h ^= packed;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    h += packed * 0x9e3779b9;
    h ^= h >> 16;
    return h;
}

/**
 * @brief 伪随机数生成函数
 * @param seed 种子值
 * @param count 生成数量
 * @return 生成的符号序列
 */
QVector<double> SpinalCode6::rngFunc(quint32 seed, int count)
{
    QVector<double> result;
    result.reserve(count);
    quint32 s = seed;
    for (int i = 0; i < count; ++i) {
        // 线性同余 + 哈希混合
        s = s * 1103515245 + 12345;
        s ^= s >> 16;
        s *= 0x45d9f3b;
        // 映射到[-1, 1]
        result.append(static_cast<double>(static_cast<int>(s & 0xFFFF)) / 32768.0 - 1.0);
    }
    return result;
}
