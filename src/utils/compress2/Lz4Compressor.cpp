/**
 * @file Lz4Compressor.cpp
 * @brief Lz4Compressor 实现 — LZ4风格高速压缩/解压
 *
 * 压缩流程:
 * 1. 逐字节扫描输入, 对每个位置在滑动窗口中搜索最长匹配
 * 2. 匹配成功: 输出token(literal长度+match长度) + 偏移量(2字节小端)
 * 3. 匹配失败: 累积为字面量字节
 * 4. 级别控制窗口搜索精细度
 *
 * 解压流程:
 * 1. 读取token解码literal长度和match长度
 * 2. 复制literal字节到输出
 * 3. 读取偏移量, 从已解压数据中复制match区域
 */

#include "utils/compress2/Lz4Compressor.h"

#include <QByteArray>
#include <algorithm>
#include <cstring>

// ── 构造 / 析构 ──

/**
 * @brief 构造函数
 * @param compressionLevel 压缩级别
 * @param parent           父QObject
 */
Lz4Compressor::Lz4Compressor(int compressionLevel, QObject* parent)
    : QObject(parent)
    , m_level(qBound(1, compressionLevel, 9))
{
    setObjectName(QStringLiteral("Lz4Compressor"));
    m_timer.start();
}

Lz4Compressor::~Lz4Compressor() = default;

// ── 核心操作 ──

/**
 * @brief 压缩数据
 *
 * 使用LZ4风格的token编码: 每个序列包含:
 * - 1字节token: 高4位literal长度, 低4位match长度(超出部分用扩展编码)
 * - literal数据(若literal长度>0)
 * - 2字节偏移量(小端序, match起始位置的距离)
 * @param data 原始数据
 * @return 带头部的压缩数据
 */
QByteArray Lz4Compressor::compress(const QByteArray& data)
{
    m_timer.restart();

    if (data.isEmpty()) {
        emit error(tr("压缩失败: 输入数据为空"));
        return {};
    }

    const int srcLen = data.size();
    const char* src = data.constData();

    /* 输出缓冲区预分配 */
    QByteArray output;
    output.reserve(srcLen + srcLen / 64 + 16);

    int srcPos = 0;
    int anchor = 0; // 当前literal序列的起始位置

    while (srcPos < srcLen) {
        int bestLen = 0;
        int bestOffset = 0;

        /* 搜索窗口范围 */
        int windowStart = qMax(0, srcPos - DEFAULT_WINDOW_SIZE);
        int step = windowStep(m_level);

        /* 在窗口中搜索最长匹配 */
        for (int w = srcPos - 1; w >= windowStart; w -= step) {
            int matchLen = 0;
            int maxLen = qMin(srcLen - srcPos, 65535);
            while (matchLen < maxLen && src[w + matchLen] == src[srcPos + matchLen]) {
                ++matchLen;
            }
            if (matchLen > bestLen) {
                bestLen = matchLen;
                bestOffset = srcPos - w;
                if (bestLen >= 65535) break; // 已达最大长度
            }
        }

        if (bestLen >= MIN_MATCH) {
            /* 输出literal + match序列 */
            int litLen = srcPos - anchor;

            /* Token字节: 高4位literal长度, 低4位match长度(减去MIN_MATCH) */
            quint8 token = 0;
            int matchCode = bestLen - MIN_MATCH;

            if (litLen >= 15) token |= 0xF0;
            else token |= static_cast<quint8>(litLen << 4);

            if (matchCode >= 15) token |= 0x0F;
            else token |= static_cast<quint8>(matchCode);

            output.append(static_cast<char>(token));

            /* 扩展literal长度(若>=15) */
            int remaining = litLen - 15;
            while (remaining >= 255) {
                output.append(static_cast<char>(255));
                remaining -= 255;
            }
            if (litLen >= 15) {
                output.append(static_cast<char>(remaining));
            }

            /* 复制literal字节 */
            if (litLen > 0) {
                output.append(src + anchor, litLen);
            }

            /* 写入偏移量(2字节小端) */
            output.append(static_cast<char>(bestOffset & 0xFF));
            output.append(static_cast<char>((bestOffset >> 8) & 0xFF));

            /* 扩展match长度(若>=15+MIN_MATCH) */
            remaining = matchCode - 15;
            while (remaining >= 255) {
                output.append(static_cast<char>(255));
                remaining -= 255;
            }
            if (matchCode >= 15) {
                output.append(static_cast<char>(remaining));
            }

            srcPos += bestLen;
            anchor = srcPos;
        } else {
            /* 无匹配, 当前字节归入literal */
            ++srcPos;
        }
    }

    /* 处理末尾剩余literal */
    int litLen = srcLen - anchor;
    if (litLen > 0) {
        quint8 token = 0;
        if (litLen >= 15) token |= 0xF0;
        else token |= static_cast<quint8>(litLen << 4);

        output.append(static_cast<char>(token));

        int remaining = litLen - 15;
        while (remaining >= 255) {
            output.append(static_cast<char>(255));
            remaining -= 255;
        }
        if (litLen >= 15) {
            output.append(static_cast<char>(remaining));
        }
        output.append(src + anchor, litLen);
    }

    /* 组装最终输出: 头部 + 载荷 */
    QByteArray result = writeHeader(static_cast<quint32>(srcLen),
                                    static_cast<quint8>(m_level));
    result.append(output);

    /* 更新统计 */
    ++m_stats.totalCompressed;
    m_stats.totalBytesIn += static_cast<quint64>(srcLen);
    m_stats.totalBytesOut += static_cast<quint64>(result.size());

    double ratio = static_cast<double>(result.size()) / qMax(static_cast<double>(srcLen), 1.0);
    quint64 n = m_stats.totalCompressed;
    m_stats.avgCompressionRatio =
        m_stats.avgCompressionRatio * static_cast<double>(n - 1) / static_cast<double>(n)
        + ratio / static_cast<double>(n);

    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * static_cast<double>(n - 1) / static_cast<double>(n)
        + elapsed / static_cast<double>(n);

    emit compressionCompleted(srcLen, result.size());
    return result;
}

/**
 * @brief 解压数据
 *
 * 读取token解码literal长度和match长度, 读取偏移量从已解压缓冲区复制。
 * @param data 压缩数据(含头部)
 * @return 原始数据
 */
QByteArray Lz4Compressor::decompress(const QByteArray& data)
{
    m_timer.restart();

    if (data.size() <= HEADER_SIZE) {
        emit error(tr("解压失败: 数据过短(无有效头部)"));
        return {};
    }

    int originalSize = readHeader(data);
    if (originalSize < 0) {
        emit error(tr("解压失败: 头部校验错误"));
        return {};
    }

    const char* src = data.constData() + HEADER_SIZE;
    int srcLen = data.size() - HEADER_SIZE;
    int srcPos = 0;

    QByteArray output;
    output.reserve(originalSize);

    while (srcPos < srcLen) {
        /* 读取token */
        quint8 token = static_cast<quint8>(src[srcPos++]);

        /* 解码literal长度 */
        int litLen = (token >> 4) & 0x0F;
        if (litLen == 15) {
            while (srcPos < srcLen) {
                quint8 extra = static_cast<quint8>(src[srcPos++]);
                litLen += extra;
                if (extra != 255) break;
            }
        }

        /* 复制literal字节 */
        if (litLen > 0 && srcPos + litLen <= srcLen) {
            output.append(src + srcPos, litLen);
            srcPos += litLen;
        }

        /* 检查是否到达末尾(最后一个序列可能没有match) */
        if (srcPos >= srcLen) break;

        /* 读取偏移量(2字节小端) */
        if (srcPos + 1 >= srcLen) break;
        int offset = static_cast<quint8>(src[srcPos])
                   | (static_cast<quint8>(src[srcPos + 1]) << 8);
        srcPos += 2;

        if (offset == 0 || offset > output.size()) {
            emit error(tr("解压失败: 无效偏移量"));
            return {};
        }

        /* 解码match长度 */
        int matchCode = token & 0x0F;
        if (matchCode == 15) {
            while (srcPos < srcLen) {
                quint8 extra = static_cast<quint8>(src[srcPos++]);
                matchCode += extra;
                if (extra != 255) break;
            }
        }
        int matchLen = matchCode + MIN_MATCH;

        /* 从已解压缓冲区复制match区域 */
        int matchStart = output.size() - offset;
        for (int i = 0; i < matchLen; ++i) {
            output.append(output.at(matchStart + i));
        }
    }

    /* 更新统计 */
    ++m_stats.totalDecompressed;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(output.size());

    quint64 n = m_stats.totalCompressed + m_stats.totalDecompressed;
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * static_cast<double>(n - 1) / static_cast<double>(n)
        + elapsed / static_cast<double>(n);

    emit decompressionCompleted(data.size(), output.size());
    return output;
}

// ── 参数配置 ──

/**
 * @brief 设置压缩级别
 * @param level 级别1~9
 */
void Lz4Compressor::setLevel(int level)
{
    m_level = qBound(1, level, 9);
}

/** @brief 获取当前压缩级别 */
int Lz4Compressor::level() const
{
    return m_level;
}

// ── 统计 ──

/**
 * @brief 获取当前统计数据快照
 * @return Stats结构体副本
 */
Lz4Compressor::Stats Lz4Compressor::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器
 */
void Lz4Compressor::resetStatistics()
{
    m_stats = Stats{};
}

// ── 私有方法 ──

/**
 * @brief 根据压缩级别返回窗口搜索步进
 * @param level 压缩级别
 * @return 步进值(级别越高步进越小搜索越精细)
 */
int Lz4Compressor::windowStep(int level) const
{
    /* 级别1步进64(最快), 级别9步进1(最精细) */
    return qMax(1, 10 - level);
}

/**
 * @brief 写入压缩头部
 * @param originalSize 原始数据大小
 * @param level        压缩级别
 * @return 头部字节数组(7字节)
 */
QByteArray Lz4Compressor::writeHeader(quint32 originalSize, quint8 level) const
{
    QByteArray header(HEADER_SIZE, '\0');
    header[0] = MAGIC_0;
    header[1] = MAGIC_1;
    header[2] = static_cast<char>(originalSize & 0xFF);
    header[3] = static_cast<char>((originalSize >> 8) & 0xFF);
    header[4] = static_cast<char>((originalSize >> 16) & 0xFF);
    header[5] = static_cast<char>((originalSize >> 24) & 0xFF);
    header[6] = static_cast<char>(level);
    return header;
}

/**
 * @brief 读取并校验压缩头部
 * @param data 完整压缩数据(含头部)
 * @return 原始数据大小; 失败返回-1
 */
int Lz4Compressor::readHeader(const QByteArray& data) const
{
    if (data.size() < HEADER_SIZE) return -1;
    if (data[0] != MAGIC_0 || data[1] != MAGIC_1) return -1;

    quint32 size = static_cast<quint8>(data[2])
                 | (static_cast<quint32>(static_cast<quint8>(data[3])) << 8)
                 | (static_cast<quint32>(static_cast<quint8>(data[4])) << 16)
                 | (static_cast<quint32>(static_cast<quint8>(data[5])) << 24);
    return static_cast<int>(size);
}
