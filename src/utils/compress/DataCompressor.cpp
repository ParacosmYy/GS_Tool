/**
 * @file DataCompressor.cpp
 * @brief 数据压缩器实现 — RLE/Huffman/LZ77/Deflate 压缩解压
 *
 * 所有压缩算法均在本地实现，不依赖 zlib 等外部库。
 * RLE 对重复字节高效; Huffman 对已知频率分布的数据效果好;
 * LZ77 利用滑动窗口回溯匹配; Deflate 组合 LZ77+Huffman 获得最佳压缩比。
 * 压缩数据携带自定义头部(魔数+算法+原始大小+级别)，解压时校验头部完整性。
 */

#include "utils/compress/DataCompressor.h"

#include <QMap>
#include <QQueue>
#include <algorithm>
#include <cstring>

// ── 常量定义 ──

/** @brief 压缩数据魔数 'E' 'D' (EmbedDebug) */
static constexpr char MAGIC_0 = 'E';
static constexpr char MAGIC_1 = 'D';

/** @brief 头部大小: 魔数(2) + 算法(1) + 原始大小(4) + 压缩级别(1) = 8字节 */
static constexpr int HEADER_SIZE = 8;

/** @brief LZ77默认滑动窗口大小 */
static constexpr int LZ77_WINDOW_BASE = 4096;

/** @brief LZ77最大匹配长度上限 */
static constexpr int LZ77_MAX_MATCH = 258;

/** @brief RLE最大重复计数(用quint8表示, 最大255) */
static constexpr int RLE_MAX_RUN = 255;

// ── 构造 / 析构 ──

/**
 * @brief 构造函数，设置 objectName 用于 QSS 依赖
 * @param parent 父 QObject
 */
DataCompressor::DataCompressor(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataCompressor"));
}

DataCompressor::~DataCompressor() = default;

// ── 核心操作 ──

/**
 * @brief 压缩数据入口，根据算法分发到对应实现
 * @param data 原始数据
 * @param algorithm 压缩算法
 * @return 带头部的压缩数据; 失败返回空并发射 error 信号
 */
QByteArray DataCompressor::compress(const QByteArray& data, Algorithm algorithm)
{
    if (data.isEmpty()) {
        emit error(tr("压缩失败: 输入数据为空"));
        ++m_stats.compressErrors;
        return {};
    }

    /* None 算法直接透传，仍写入头部以便解压端识别 */
    if (algorithm == Algorithm::None) {
        QByteArray result = writeHeader(algorithm, static_cast<quint32>(data.size()));
        result.append(data);
        ++m_stats.totalCompressions;
        m_stats.totalBytesIn += static_cast<quint64>(data.size());
        m_stats.totalBytesOut += static_cast<quint64>(result.size());
        ++m_stats.operationsByAlgorithm[static_cast<int>(Algorithm::None)];
        emit compressed(result, algorithm);
        return result;
    }

    QByteArray payload;
    switch (algorithm) {
    case Algorithm::RunLength: payload = compressRle(data);   break;
    case Algorithm::Huffman:   payload = compressHuffman(data); break;
    case Algorithm::LZ77:      payload = compressLz77(data);  break;
    case Algorithm::Deflate:   payload = compressDeflate(data); break;
    default:
        emit error(tr("压缩失败: 不支持的算法"));
        ++m_stats.compressErrors;
        return {};
    }

    if (payload.isEmpty()) {
        emit error(tr("压缩失败: 算法内部错误"));
        ++m_stats.compressErrors;
        return {};
    }

    /* 组装最终输出: 头部 + 压缩载荷 */
    QByteArray result = writeHeader(algorithm, static_cast<quint32>(data.size()));
    result.append(payload);

    ++m_stats.totalCompressions;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());

    /* 更新平均压缩比 */
    double ratio = static_cast<double>(result.size()) / static_cast<double>(data.size());
    quint64 n = m_stats.totalCompressions;
    m_stats.avgRatio = m_stats.avgRatio * static_cast<double>(n - 1) / static_cast<double>(n) + ratio / static_cast<double>(n);

    ++m_stats.operationsByAlgorithm[static_cast<int>(algorithm)];
    emit compressed(result, algorithm);
    return result;
}

/**
 * @brief 解压数据入口，根据算法分发到对应实现
 * @param data 带头部的压缩数据
 * @param algorithm 解压算法(须与压缩时一致)
 * @return 原始数据; 失败返回空并发射 error 信号
 */
QByteArray DataCompressor::decompress(const QByteArray& data, Algorithm algorithm)
{
    if (data.size() <= HEADER_SIZE) {
        emit error(tr("解压失败: 数据过短(无有效头部)"));
        ++m_stats.compressErrors;
        return {};
    }

    int originalSize = readHeader(data, algorithm);
    if (originalSize < 0) {
        emit error(tr("解压失败: 头部校验错误"));
        ++m_stats.compressErrors;
        return {};
    }

    QByteArray payload = data.mid(HEADER_SIZE);

    /* None 算法直接返回载荷 */
    if (algorithm == Algorithm::None) {
        ++m_stats.totalDecompressions;
        m_stats.totalBytesIn += static_cast<quint64>(data.size());
        m_stats.totalBytesOut += static_cast<quint64>(payload.size());
        ++m_stats.operationsByAlgorithm[static_cast<int>(Algorithm::None)];
        emit decompressed(payload, algorithm);
        return payload;
    }

    QByteArray result;
    switch (algorithm) {
    case Algorithm::RunLength: result = decompressRle(payload);     break;
    case Algorithm::Huffman:   result = decompressHuffman(payload); break;
    case Algorithm::LZ77:      result = decompressLz77(payload);    break;
    case Algorithm::Deflate:   result = decompressDeflate(payload); break;
    default:
        emit error(tr("解压失败: 不支持的算法"));
        ++m_stats.compressErrors;
        return {};
    }

    if (result.isEmpty() && originalSize > 0) {
        emit error(tr("解压失败: 算法内部错误"));
        ++m_stats.compressErrors;
        return {};
    }

    ++m_stats.totalDecompressions;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());
    ++m_stats.operationsByAlgorithm[static_cast<int>(algorithm)];
    emit decompressed(result, algorithm);
    return result;
}

// ── 辅助功能 ──

/**
 * @brief 预估压缩比 — 基于信息熵和重复模式分析
 * @param data 待分析数据
 * @param algorithm 目标算法
 * @return 预估压缩比(0.0~1.0, 越低表示压缩效果越好)
 */
double DataCompressor::estimateRatio(const QByteArray& data, Algorithm algorithm) const
{
    if (data.size() < 4) return 1.0;

    double entropy = calculateEntropy(data);

    switch (algorithm) {
    case Algorithm::RunLength: {
        /* RLE: 统计连续重复字节占比 */
        int runs = 1;
        for (int i = 1; i < data.size(); ++i) {
            if (data[i] != data[i - 1]) ++runs;
        }
        /* 估算: 每段需2字节(值+计数), 总段数为runs */
        double estimated = static_cast<double>(runs * 2) / static_cast<double>(data.size());
        return qBound(0.05, estimated, 1.5);
    }
    case Algorithm::Huffman: {
        /* Huffman: 压缩比约等于 entropy/8.0 */
        double ratio = entropy / 8.0;
        /* 加上频率表开销(256*4字节) */
        double overhead = 1024.0 / static_cast<double>(data.size());
        return qBound(0.05, ratio + overhead, 1.5);
    }
    case Algorithm::LZ77: {
        /* LZ77: 基于重复模式密度估算 */
        int matches = 0;
        int step = qMax(1, data.size() / 500);
        for (int i = 1; i < data.size(); i += step) {
            LzMatch m = findLzMatch(data, i, qMin(LZ77_WINDOW_BASE, i));
            if (m.length >= 3) matches += m.length;
        }
        double matchRatio = static_cast<double>(matches * step) / static_cast<double>(data.size());
        return qBound(0.05, 1.0 - matchRatio * 0.7, 1.5);
    }
    case Algorithm::Deflate: {
        /* Deflate ≈ LZ77的80%(Huffman二次压缩) */
        double lzRatio = estimateRatio(data, Algorithm::LZ77);
        return qBound(0.05, lzRatio * 0.8, 1.5);
    }
    case Algorithm::None:
        return 1.0 + static_cast<double>(HEADER_SIZE) / static_cast<double>(data.size());
    }
    return 1.0;
}

/**
 * @brief 检测数据是否已被压缩 — 基于魔数头部和高熵值判断
 * @param data 待检测数据
 * @return true 表示数据可能已被压缩
 */
bool DataCompressor::isCompressed(const QByteArray& data) const
{
    /* 检查是否有本模块的压缩头部 */
    if (data.size() > HEADER_SIZE) {
        if (data[0] == MAGIC_0 && data[1] == MAGIC_1) {
            quint8 algo = static_cast<quint8>(data[2]);
            return algo < 5; /* 0~4对应五种算法 */
        }
    }

    /* 基于熵值判断: 高熵(>7.5)通常意味着已压缩或加密 */
    if (data.size() >= 64) {
        double entropy = calculateEntropy(data);
        if (entropy > 7.5) return true;
    }

    return false;
}

/**
 * @brief 设置压缩级别
 * @param level 压缩级别 1(最快)~9(最佳压缩)
 */
void DataCompressor::setCompressionLevel(int level)
{
    m_level = qBound(1, level, 9);
}

int DataCompressor::compressionLevel() const
{
    return m_level;
}

// ── 统计 ──

DataCompressor::Stats DataCompressor::stats() const
{
    return m_stats;
}

void DataCompressor::resetStatistics()
{
    m_stats = Stats{};
}
