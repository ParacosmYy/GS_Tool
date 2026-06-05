/**
 * @file SpinalCode.cpp
 * @brief Spinal码实现 — 序列哈希编码器 + BFS渐进解码器
 */

#include "utils/code12/SpinalCode.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <random>

/* ── 构造/配置 ── */

/** @brief 构造函数 @param parent 父对象 */
SpinalCode::SpinalCode(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置编码参数 @param params 参数 */
void SpinalCode::setParams(const CodeParams& params)
{
    m_params = params;
}

/* ── 编码 ── */

/** @brief 编码数据 @param data 输入字节 @return 编码符号(浮点映射) */
QVector<double> SpinalCode::encode(const QByteArray& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    /* 将字节展开为k-bit符号 */
    QVector<int> symbols = bytesToSymbols(data);
    int spineLen = symbols.size();
    m_params.spineLength = spineLen;

    /* 逐脊骨编码 */
    QVector<double> coded;
    coded.reserve(spineLen * m_params.numPasses);

    quint32 spine = 0; /* 脊骨值初始化为0 */
    for (int i = 0; i < spineLen; ++i) {
        spine = spineHash(spine, symbols[i]);
        for (int p = 0; p < m_params.numPasses; ++p) {
            coded.append(mapToSymbol(spine, p));
        }
    }

    /* 更新统计 */
    ++m_stats.totalEncodes;
    m_stats.totalBytesProcessed += data.size();
    m_stats.totalSymbolsGenerated += static_cast<int>(coded.size());
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeComplete(data.size() * 8, coded.size());
    return coded;
}

/* ── 解码 ── */

/** @brief BFS渐进解码 @param symbols 接收符号 @param originalSize 原始字节数 @return 解码结果 */
SpinalCode::DecodeResult SpinalCode::decode(
    const QVector<double>& symbols, int originalSize)
{
    DecodeResult result;
    if (symbols.isEmpty() || originalSize <= 0) return result;

    QElapsedTimer timer;
    timer.start();

    int k = m_params.k;
    int spineLen = (originalSize * 8 + k - 1) / k; /* 脊骨长度 */
    int symbolsPerSpine = m_params.numPasses;
    int W = m_params.bfsWidth; /* 搜索宽度 */

    /* BFS状态: (spine值, 符号序列, 累计距离) */
    struct State {
        quint32 spine;                   ///< 当前脊骨值
        QVector<int> symbols;            ///< 已解码符号序列
        double cost;                     ///< 累计距离
    };

    QList<State> beam;
    beam.append({0, {}, 0.0});

    for (int s = 0; s < spineLen; ++s) {
        /* 提取当前脊骨对应的接收符号 */
        QList<double> rxSymbols;
        int baseIdx = s * symbolsPerSpine;
        for (int p = 0; p < symbolsPerSpine; ++p) {
            int idx = baseIdx + p;
            if (idx < symbols.size()) {
                rxSymbols.append(symbols[idx]);
            }
        }

        /* 对beam中每条路径，展开所有可能的2^k个符号 */
        QList<State> candidates;
        int symbolCount = 1 << k;

        for (const auto& state : beam) {
            for (int sym = 0; sym < symbolCount; ++sym) {
                quint32 newSpine = spineHash(state.spine, sym);

                /* 计算该符号对应的编码与接收符号的距离 */
                double dist = 0.0;
                for (int p = 0; p < rxSymbols.size(); ++p) {
                    double tx = mapToSymbol(newSpine, p);
                    dist += symbolDistance(tx, rxSymbols[p]);
                }

                State newState;
                newState.spine = newSpine;
                newState.symbols = state.symbols;
                newState.symbols.append(sym);
                newState.cost = state.cost + dist;
                candidates.append(std::move(newState));
            }
        }

        /* 保留前W条最优路径 */
        std::sort(candidates.begin(), candidates.end(),
                  [](const State& a, const State& b) {
                      return a.cost < b.cost;
                  });

        if (candidates.size() > W) {
            candidates = candidates.mid(0, W);
        }
        beam = std::move(candidates);
    }

    /* 取最优路径 */
    if (!beam.isEmpty()) {
        result.decodedData = symbolsToBytes(beam[0].symbols, originalSize);
        result.passesUsed = symbolsPerSpine;

        /* 计算置信度: 基于最优与次优路径的代价比 */
        if (beam.size() >= 2) {
            double best = beam[0].cost;
            double second = beam[1].cost;
            if (second > 1e-10) {
                result.confidence = qBound(0.0, 1.0 - best / second, 1.0);
            } else {
                result.confidence = 1.0;
            }
        } else {
            result.confidence = 1.0;
        }
        result.success = true;
    }

    /* 更新统计 */
    ++m_stats.totalDecodes;
    m_stats.totalBytesProcessed += originalSize;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeComplete(result.success, result.confidence);
    return result;
}

/* ── 辅助方法 ── */

/** @brief 计算当前码率 */
double SpinalCode::codeRate(int dataLen, int symbolCount) const
{
    if (symbolCount <= 0 || dataLen <= 0) return 0.0;
    return static_cast<double>(dataLen * 8)
         / static_cast<double>(symbolCount * m_params.k);
}

/* ── 私有: 脊骨哈希 ── */

/** @brief 脊骨哈希函数: MurmurHash3变体 */
quint32 SpinalCode::spineHash(quint32 spine, int symbol) const
{
    quint32 h = spine;
    h ^= static_cast<quint32>(symbol) * 0xcc9e2d51;
    h = (h << 15) | (h >> 17);
    h *= 0x1b873593;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

/** @brief 从脊骨值映射到编码符号 */
double SpinalCode::mapToSymbol(quint32 spineValue, int passIndex) const
{
    /* 使用脊骨值和遍索引生成伪随机符号 */
    quint32 seed = spineValue + static_cast<quint32>(passIndex) * 2654435761u;
    seed ^= seed >> 16;
    seed *= 0x45d9f3b;
    seed ^= seed >> 16;
    seed *= 0x45d9f3b;
    seed ^= seed >> 16;

    /* 映射到[-1, 1]的BPSK符号 */
    return (static_cast<double>(seed & 1) - 0.5) * 2.0;
}

/** @brief 字节展开为k-bit符号 */
QVector<int> SpinalCode::bytesToSymbols(const QByteArray& data) const
{
    QVector<int> symbols;
    int k = m_params.k;
    int totalBits = data.size() * 8;
    int symbolCount = (totalBits + k - 1) / k;
    symbols.reserve(symbolCount);

    for (int s = 0; s < symbolCount; ++s) {
        int val = 0;
        for (int b = 0; b < k; ++b) {
            int bitIdx = s * k + b;
            int byteIdx = bitIdx / 8;
            int bitPos = 7 - (bitIdx % 8);
            if (byteIdx < data.size()) {
                int bit = (static_cast<quint8>(data[byteIdx]) >> bitPos) & 1;
                val = (val << 1) | bit;
            }
        }
        symbols.append(val);
    }
    return symbols;
}

/** @brief 符号序列还原为字节 */
QByteArray SpinalCode::symbolsToBytes(const QVector<int>& symbols,
                                       int byteCount) const
{
    QByteArray result(byteCount, 0);
    int k = m_params.k;

    for (int s = 0; s < symbols.size(); ++s) {
        for (int b = 0; b < k; ++b) {
            int bitIdx = s * k + b;
            int byteIdx = bitIdx / 8;
            int bitPos = 7 - (bitIdx % 8);
            if (byteIdx < byteCount) {
                int bit = (symbols[s] >> (k - 1 - b)) & 1;
                result[byteIdx] = static_cast<char>(
                    static_cast<quint8>(result[byteIdx]) | (bit << bitPos));
            }
        }
    }
    return result;
}

/** @brief 符号距离度量(欧氏距离平方) */
double SpinalCode::symbolDistance(double a, double b)
{
    double d = a - b;
    return d * d;
}

/* ── 统计 ── */

/** @brief 重置统计 */
void SpinalCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
