/**
 * @file RunLengthEncoder.cpp
 * @brief 增强型游程编码器实现
 */

#include "RunLengthEncoder.h"

#include <QElapsedTimer>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

RunLengthEncoder::RunLengthEncoder(QObject* parent)
    : QObject(parent)
{
}

RunLengthEncoder::~RunLengthEncoder() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void RunLengthEncoder::setMode(Mode mode) { m_mode = mode; }
RunLengthEncoder::Mode RunLengthEncoder::mode() const { return m_mode; }
void RunLengthEncoder::setMinRun(int threshold) { m_minRun = std::max(threshold, 2); }

// ═══════════════════════════════════════════════════════════
// 编码/解码分发
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthEncoder::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    QByteArray result;
    switch (m_mode) {
    case PackBits:    result = encodePackBits(data); break;
    case CcittGroup3: result = encodeCcitt(data); break;
    case CustomRle:   result = encodeCustom(data); break;
    }

    m_stats.totalEncodes += 1;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());

    const double ratio = static_cast<double>(result.size()) /
                         static_cast<double>(std::max(data.size(), qsizetype(1)));
    if (m_stats.totalEncodes == 1) {
        m_stats.avgRatio = ratio;
    } else {
        m_stats.avgRatio += (ratio - m_stats.avgRatio) /
                            static_cast<double>(m_stats.totalEncodes);
    }

    emit encoded(result, ratio);
    return result;
}

QByteArray RunLengthEncoder::decode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalDecodes += 1;
        emit decoded({});
        return {};
    }

    QByteArray result;
    // 读取模式标记(首字节)
    const auto modeUsed = static_cast<Mode>(static_cast<quint8>(data[0]));
    const QByteArray payload = data.mid(1);

    switch (modeUsed) {
    case PackBits:    result = decodePackBits(payload); break;
    case CcittGroup3: result = decodeCcitt(payload); break;
    case CustomRle:   result = decodeCustom(payload); break;
    }

    m_stats.totalDecodes += 1;
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

RunLengthEncoder::Stats RunLengthEncoder::stats() const { return m_stats; }

void RunLengthEncoder::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// PackBits编解码
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthEncoder::encodePackBits(const QByteArray& data)
{
    QByteArray result;
    result.append(static_cast<char>(PackBits)); // 模式标记
    int i = 0;
    const int n = data.size();

    while (i < n) {
        // 检测重复序列
        int run = 1;
        while (i + run < n && data[i + run] == data[i] && run < 128) {
            run++;
        }

        if (run >= 2) {
            // 重复: 计数字节 = -(run-1), 后跟一个数据字节
            result.append(static_cast<char>(-(run - 1) & 0xFF));
            result.append(data[i]);
            i += run;
        } else {
            // 不重复: 收集不重复字节
            int litStart = i;
            int litLen = 0;
            while (i + litLen < n && litLen < 128) {
                if (i + litLen + 1 < n && data[i + litLen] == data[i + litLen + 1]) {
                    if (litLen == 0) {
                        // 下一个是重复序列的开始, 停止
                        break;
                    }
                    break;
                }
                litLen++;
            }
            if (litLen == 0) litLen = 1;
            result.append(static_cast<char>(litLen - 1));
            result.append(data.mid(litStart, litLen));
            i += litLen;
        }
    }
    return result;
}

QByteArray RunLengthEncoder::decodePackBits(const QByteArray& data)
{
    QByteArray result;
    int i = 0;
    while (i < data.size()) {
        auto count = static_cast<qint8>(data[i]);
        i++;
        if (count >= 0) {
            // 不重复: 后面(count+1)个字节
            int len = count + 1;
            if (i + len > data.size()) break;
            result.append(data.mid(i, len));
            i += len;
        } else {
            // 重复: 1个字节重复(-count+1)次
            int rep = -count + 1;
            if (i >= data.size()) break;
            result.append(QByteArray(rep, data[i]));
            i++;
        }
    }
    return result;
}

// ═══════════════════════════════════════════════════════════
// CCITT Group 3编解码(简化: 逐行黑白行程编码)
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthEncoder::encodeCcitt(const QByteArray& data)
{
    QByteArray result;
    result.append(static_cast<char>(CcittGroup3));

    // 简化CCITT: 对每个字节统计连续0和1的行程
    int pos = 0;
    bool whiteRun = true; // 白色行程先开始

    while (pos < data.size()) {
        int runLen = 0;
        quint8 targetBit = whiteRun ? 0 : 1;

        for (int byteIdx = pos; byteIdx < data.size() && runLen < 2560; ++byteIdx) {
            for (int bit = 7; bit >= 0; --bit) {
                quint8 b = (static_cast<quint8>(data[byteIdx]) >> bit) & 1;
                if (b == targetBit) {
                    runLen++;
                } else {
                    goto end_run;
                }
            }
        }
    end_run:

        // 编码行程长度(两字节: 高字节+低字节)
        result.append(static_cast<char>((runLen >> 8) & 0xFF));
        result.append(static_cast<char>(runLen & 0xFF));
        whiteRun = !whiteRun;

        // 跳过已处理的位
        int bytesToSkip = runLen / 8;
        pos += bytesToSkip;
    }

    return result;
}

QByteArray RunLengthEncoder::decodeCcitt(const QByteArray& data)
{
    QByteArray result;
    int i = 0;
    bool whiteRun = true;

    while (i + 1 < data.size()) {
        int runLen = (static_cast<quint8>(data[i]) << 8) |
                     static_cast<quint8>(data[i + 1]);
        i += 2;

        quint8 fillByte = whiteRun ? 0x00 : 0xFF;
        int fullBytes = runLen / 8;
        int extraBits = runLen % 8;

        result.append(QByteArray(fullBytes, static_cast<char>(fillByte)));
        if (extraBits > 0) {
            quint8 partial = whiteRun ? 0x00 : static_cast<quint8>(0xFF << (8 - extraBits));
            result.append(static_cast<char>(partial));
        }

        whiteRun = !whiteRun;
    }

    return result;
}

// ═══════════════════════════════════════════════════════════
// 自定义RLE编解码
// ═══════════════════════════════════════════════════════════

QByteArray RunLengthEncoder::encodeCustom(const QByteArray& data)
{
    QByteArray result;
    result.append(static_cast<char>(CustomRle));

    int i = 0;
    while (i < data.size()) {
        char val = data[i];
        int run = 1;
        while (i + run < data.size() && data[i + run] == val && run < 255) {
            run++;
        }

        if (run >= m_minRun) {
            // 使用转义标记: 0xFF + 计数 + 值
            result.append(static_cast<char>(0xFF));
            result.append(static_cast<char>(run));
            result.append(val);
            i += run;
        } else {
            for (int j = 0; j < run; ++j) {
                if (static_cast<quint8>(data[i + j]) == 0xFF) {
                    result.append(static_cast<char>(0xFF));
                    result.append(static_cast<char>(1));
                }
                result.append(data[i + j]);
            }
            i += run;
        }
    }

    return result;
}

QByteArray RunLengthEncoder::decodeCustom(const QByteArray& data)
{
    QByteArray result;
    int i = 0;

    while (i < data.size()) {
        if (static_cast<quint8>(data[i]) == 0xFF) {
            i++;
            if (i >= data.size()) break;
            auto count = static_cast<quint8>(data[i]);
            i++;
            if (i >= data.size()) break;
            result.append(QByteArray(count, data[i]));
            i++;
        } else {
            result.append(data[i]);
            i++;
        }
    }

    return result;
}
