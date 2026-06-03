/**
 * @file ByteFormatExtended.h
 * @brief 字节格式化扩展工具函数
 *
 * 从 ByteFormat.h 拆分而来，包含紧凑格式、百分比、精确时间、
 * ETA 预估等扩展格式化函数。基础格式化函数见 ByteFormat.h。
 */

#ifndef BYTEFORMAT_EXTENDED_H
#define BYTEFORMAT_EXTENDED_H

#include <QString>
#include <QCoreApplication>
#include <QtGlobal>

namespace ByteFormat {

/**
 * @brief 格式化传输速率（紧凑格式，无空格）
 *
 * 与 formatRate 类似但输出不含空格，适用于空间受限的UI控件。
 * @param bytes 传输字节数
 * @param seconds 传输耗时（秒）
 * @return 紧凑格式速率字符串，如 "2.3KB/s"
 */
inline QString formatRateCompact(qint64 bytes, double seconds)
{
    if (seconds <= 0.0) {
        return QStringLiteral("--");
    }

    double bps = static_cast<double>(bytes) / seconds;

    if (bps >= 1024.0 * 1024.0 * 1024.0) {
        return QString("%1GB/s").arg(bps / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
    } else if (bps >= 1024.0 * 1024.0) {
        return QString("%1MB/s").arg(bps / (1024.0 * 1024.0), 0, 'f', 1);
    } else if (bps >= 1024.0) {
        return QString("%1KB/s").arg(bps / 1024.0, 0, 'f', 1);
    } else {
        return QString("%1B/s").arg(static_cast<qint64>(bps));
    }
}

/**
 * @brief 格式化百分比
 *
 * 将比值（0.0~1.0）格式化为百分比字符串。
 * 超出范围时钳制到 [0%, 100%]。
 *
 * @param ratio 比值（0.0~1.0）
 * @param decimals 小数位数，默认1
 * @return 百分比字符串，如 "75.3%"
 */
inline QString formatPercent(double ratio, int decimals = 1)
{
    double pct = qBound(0.0, ratio, 1.0) * 100.0;
    return QString("%1%").arg(pct, 0, 'f', decimals);
}

/**
 * @brief 格式化时间长度（带毫秒精度）
 *
 * 输出格式: "HH:MM:SS.mmm"，适用于精确时间戳显示。
 * @param ms 时间长度（毫秒）
 * @return 格式化后的时间字符串
 */
inline QString formatDurationPrecise(qint64 ms)
{
    if (ms <= 0) {
        return QStringLiteral("00:00:00.000");
    }

    qint64 totalSeconds = ms / 1000;
    int hours = static_cast<int>(totalSeconds / 3600);
    int minutes = static_cast<int>((totalSeconds % 3600) / 60);
    int seconds = static_cast<int>(totalSeconds % 60);
    int millis = static_cast<int>(ms % 1000);

    return QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}

/**
 * @brief 格式化预计剩余时间(ETA)
 *
 * 根据已完成量、总数量和已用时间推算剩余时间，输出人类可读的中文文本。
 * 返回格式根据剩余时长自动选择："即将完成"、"X秒"、"X分钟"、"X小时"。
 *
 * @param completed 已完成的数量
 * @param total 总数量
 * @param elapsedMs 已用时间（毫秒）
 * @return 预计剩余时间字符串，如 "预计剩余 3分钟"、"即将完成"、"--"
 */
inline QString formatEta(qint64 completed, qint64 total, qint64 elapsedMs)
{
    if (completed <= 0 || total <= 0 || elapsedMs <= 0 || completed >= total) {
        if (completed >= total) return QCoreApplication::translate("ByteFormat", "即将完成");
        return QStringLiteral("--");
    }

    double rate = static_cast<double>(completed) / static_cast<double>(elapsedMs);
    qint64 remainingMs = static_cast<qint64>((total - completed) / rate);

    if (remainingMs < 1000) return QCoreApplication::translate("ByteFormat", "即将完成");
    if (remainingMs < 60000) return QCoreApplication::translate("ByteFormat", "预计剩余 %1秒").arg(remainingMs / 1000);
    if (remainingMs < 3600000) return QCoreApplication::translate("ByteFormat", "预计剩余 %1分钟").arg(remainingMs / 60000);
    return QCoreApplication::translate("ByteFormat", "预计剩余 %1小时").arg(remainingMs / 3600000);
}

} // namespace ByteFormat

#endif // BYTEFORMAT_EXTENDED_H
