/**
 * @file Lz77Compressor.cpp
 * @brief LZ77滑动窗口压缩实现
 */

#include "utils/lz77/Lz77Compressor.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Lz77Compressor::Lz77Compressor(QObject* parent)
    : QObject(parent)
    , m_windowSize(4096)
    , m_maxMatchLength(258)
    , m_timeSum(0.0)
{
}

/** @brief 压缩数据
 *  @param data 输入数据
 *  @return 压缩后的token序列 */
QVector<Lz77Compressor::Token> Lz77Compressor::compress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Token> tokens;
    int n = data.size();
    int pos = 0;

    while (pos < n) {
        quint16 bestOffset = 0;
        quint16 bestLength = 0;

        findLongestMatch(data, pos, bestOffset, bestLength);

        Token token;
        token.offset = bestOffset;
        token.length = bestLength;

        /* 匹配后的下一个字节作为字面量 */
        int nextPos = pos + bestLength;
        token.literal = (nextPos < n) ? static_cast<quint8>(data[nextPos]) : 0;

        tokens.append(token);
        pos = nextPos + 1;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalCompressions;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalCompressions + m_stats.totalDecompressions;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    int outputSize = tokens.size() * static_cast<int>(sizeof(Token));
    emit compressionCompleted(n, outputSize);
    return tokens;
}

/** @brief 解压数据
 *  @param tokens 压缩token序列
 *  @return 解压后的数据 */
QByteArray Lz77Compressor::decompress(const QVector<Token>& tokens)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    for (const auto& token : tokens) {
        /* 复制匹配 */
        if (token.offset > 0 && token.length > 0) {
            int startPos = result.size() - token.offset;
            for (int i = 0; i < token.length; ++i) {
                int idx = startPos + i;
                if (idx >= 0 && idx < result.size()) {
                    result.append(result.at(idx));
                }
            }
        }
        /* 追加字面量 */
        result.append(static_cast<char>(token.literal));
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecompressions;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalCompressions + m_stats.totalDecompressions;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit decompressionCompleted(result.size());
    return result;
}

/** @brief 设置滑动窗口大小 @param size 窗口大小 */
void Lz77Compressor::setWindowSize(int size)
{
    m_windowSize = qMax(64, size);
}

/** @brief 设置最大匹配长度 @param len 最大匹配长度 */
void Lz77Compressor::setMaxMatchLength(int len)
{
    m_maxMatchLength = qMax(3, len);
}

/** @brief 重置统计 */
void Lz77Compressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 在窗口中搜索最长匹配 */
void Lz77Compressor::findLongestMatch(const QByteArray& data, int pos,
                                       quint16& offset, quint16& length) const
{
    int n = data.size();
    int windowStart = qMax(0, pos - m_windowSize);
    int maxLen = qMin(m_maxMatchLength, n - pos);

    quint16 bestOffset = 0;
    quint16 bestLength = 0;

    for (int i = windowStart; i < pos; ++i) {
        int matchLen = 0;
        while (matchLen < maxLen && data[i + matchLen] == data[pos + matchLen]) {
            ++matchLen;
        }
        if (matchLen > bestLength) {
            bestLength = static_cast<quint16>(matchLen);
            bestOffset = static_cast<quint16>(pos - i);
        }
    }

    offset = bestOffset;
    length = bestLength;
}
