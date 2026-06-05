/**
 * @file Lz78Compressor.cpp
 * @brief LZ78压缩引擎实现 — 基于字典的压缩与解压
 */

#include "utils/lz78/Lz78Compressor.h"

#include <QElapsedTimer>
#include <QDataStream>

/** @brief 构造函数 @param parent 父对象 */
Lz78Compressor::Lz78Compressor(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 压缩数据 @param data 原始数据 @return 压缩后数据 */
QByteArray Lz78Compressor::compress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    /* 写入原始大小用于解压校验 */
    out << static_cast<quint32>(data.size());

    /* LZ78字典: 使用QMap存储(字典索引, 字节) -> 新索引 */
    QMap<QPair<int, quint8>, int> dictionary;
    int nextIndex = 1;  /* 索引0保留为空串 */

    int pos = 0;
    int n = data.size();

    while (pos < n) {
        int currentDict = 0;
        int matchLen = 0;

        /* 在字典中寻找最长匹配 */
        while (pos + matchLen < n) {
            quint8 ch = static_cast<quint8>(data[pos + matchLen]);
            auto key = qMakePair(currentDict, ch);
            auto it = dictionary.find(key);

            if (it != dictionary.end()) {
                currentDict = it.value();
                ++matchLen;
            } else {
                /* 输出(字典索引, 新字符) */
                out << static_cast<quint32>(currentDict);
                out << ch;

                /* 添加新条目(限制字典大小防止内存溢出) */
                if (nextIndex < 0x7FFFFFFF) {
                    dictionary[key] = nextIndex++;
                }
                break;
            }
        }

        /* 处理到末尾且仍在字典中的情况 */
        if (pos + matchLen >= n && matchLen > 0 && currentDict != 0) {
            /* 尝试找到字典中的映射值(即最后匹配的字符) */
            /* 输出最后匹配的字典索引和0终止符 */
            out << static_cast<quint32>(currentDict);
            out << static_cast<quint8>(0);
        }

        pos += matchLen + 1;
    }

    /* 更新统计 */
    ++m_stats.totalCompressed;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalCompressed + m_stats.totalDecompressed;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit compressionCompleted(data.size(), result.size());
    return result;
}

/** @brief 解压数据 @param data 压缩数据 @return 原始数据 */
QByteArray Lz78Compressor::decompress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QDataStream in(data);
    in.setByteOrder(QDataStream::BigEndian);

    /* 读取原始大小 */
    quint32 originalSize = 0;
    in >> originalSize;

    QByteArray result;
    result.reserve(static_cast<int>(originalSize));

    /* 反向字典: 索引 -> (父索引, 字节) */
    QVector<QPair<int, quint8>> dict;
    dict.append({-1, 0});  /* 索引0为空串 */

    while (!in.atEnd()) {
        quint32 dictIdx = 0;
        quint8 ch = 0;
        in >> dictIdx;
        in >> ch;

        /* 回溯解码: 从字典索引重建字符串 */
        QByteArray decoded;
        int idx = static_cast<int>(dictIdx);

        /* 收集回溯链上的所有字节 */
        QVector<quint8> chain;
        while (idx > 0 && idx < dict.size()) {
            chain.append(dict[idx].second);
            idx = dict[idx].first;
        }

        /* 反转得到正确顺序 */
        for (int i = chain.size() - 1; i >= 0; --i) {
            decoded.append(static_cast<char>(chain[i]));
        }

        /* 添加当前字符(非终止符时) */
        if (ch != 0 || dictIdx != 0) {
            decoded.append(static_cast<char>(ch));
        }

        result.append(decoded);

        /* 添加到字典 */
        int parentIdx = static_cast<int>(dictIdx);
        dict.append({parentIdx, ch});
    }

    /* 截断到原始大小 */
    if (result.size() > static_cast<int>(originalSize)) {
        result.resize(static_cast<int>(originalSize));
    }

    /* 更新统计 */
    ++m_stats.totalDecompressed;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalCompressed + m_stats.totalDecompressed;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/** @brief 重置统计 */
void Lz78Compressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
