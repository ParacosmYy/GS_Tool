/**
 * @file RunLengthCodec.cpp
 * @brief 游程编码(RLE)编解码器实现
 */

#include "RunLengthCodec.h"

#include <QElapsedTimer>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

RunLengthCodec::RunLengthCodec(QObject* parent)
    : QObject(parent)
{
}

RunLengthCodec::~RunLengthCodec() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void RunLengthCodec::setEscapeByte(quint8 escape)
{
    m_escape = escape;
}

quint8 RunLengthCodec::escapeByte() const
{
    return m_escape;
}

void RunLengthCodec::setMinRunLength(int threshold)
{
    m_minRun = qMax(2, threshold);
}

int RunLengthCodec::minRunLength() const
{
    return m_minRun;
}

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthCodec::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    QByteArray result;
    result.reserve(data.size());

    const int len = data.size();
    int pos = 0;

    while (pos < len) {
        const char current = data[pos];
        int runLen = 1;

        // 计算连续相同字节数(最大255)
        while (pos + runLen < len &&
               data[pos + runLen] == current &&
               runLen < 255) {
            ++runLen;
        }

        if (runLen >= m_minRun) {
            // 输出转义序列: ESC + value + count
            result.append(static_cast<char>(m_escape));
            result.append(current);
            result.append(static_cast<char>(runLen));
            pos += runLen;
        } else {
            // 不足阈值: 原样输出(转义字节本身也要转义)
            for (int i = 0; i < runLen; ++i) {
                const char byte = data[pos + i];
                if (static_cast<quint8>(byte) == m_escape) {
                    result.append(static_cast<char>(m_escape));
                    result.append(static_cast<char>(m_escape));
                    result.append(static_cast<char>(1));
                } else {
                    result.append(byte);
                }
            }
            pos += runLen;
        }
    }

    // 更新统计
    m_stats.totalEncodes += 1;
    const double ratio = (data.size() > 0)
        ? static_cast<double>(result.size()) /
          static_cast<double>(data.size())
        : 0.0;
    updateCompressionRatio(ratio);

    emit encoded(result, ratio);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthCodec::decode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalDecodes += 1;
        emit decoded({});
        return {};
    }

    QByteArray result;
    result.reserve(data.size() * 2);

    const int len = data.size();
    int pos = 0;

    while (pos < len) {
        const quint8 byte = static_cast<quint8>(data[pos]);

        if (byte == m_escape) {
            // 转义序列: ESC + value + count
            if (pos + 2 >= len) {
                emit error(tr("RLE解码错误: 转义序列不完整, 位置 %1")
                           .arg(pos));
                return {};
            }

            const char value = data[pos + 1];
            const int count = static_cast<quint8>(data[pos + 2]);

            if (count <= 0) {
                emit error(tr("RLE解码错误: 无效重复次数 %1, 位置 %2")
                           .arg(count).arg(pos));
                return {};
            }

            for (int i = 0; i < count; ++i) {
                result.append(value);
            }

            pos += 3;
        } else {
            result.append(data[pos]);
            pos += 1;
        }
    }

    m_stats.totalDecodes += 1;
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

RunLengthCodec::Stats RunLengthCodec::stats() const
{
    return m_stats;
}

void RunLengthCodec::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void RunLengthCodec::updateCompressionRatio(double ratio)
{
    const auto n = m_stats.totalEncodes;
    if (n == 1) {
        m_stats.compressionRatio = ratio;
    } else {
        m_stats.compressionRatio =
            m_stats.compressionRatio *
                static_cast<double>(n - 1) / static_cast<double>(n) +
            ratio / static_cast<double>(n);
    }
}
