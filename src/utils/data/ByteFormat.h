/**
 * @file ByteFormat.h
 * @brief 字节格式化工具函数 - 纯函数，无状态，全局可复用
 *
 * 提供字节大小、传输速率、时间长度等人可读的格式化输出。
 * 所有函数均为纯函数（无副作用、无状态），可安全在任意线程调用。
 *
 * 使用示例:
 * @code
 *   QString rate = ByteFormat::formatRate(2304, 1.0);     // "2.3 KB/s"
 *   QString size = ByteFormat::formatSize(1572864);        // "1.5 MB"
 *   QString dur  = ByteFormat::formatDuration(155000);     // "02:35"
 *   QString dur2 = ByteFormat::formatDuration(3661500);    // "01:01:01"
 * @endcode
 *
 * 扩展函数(紧凑速率/百分比/精确时间/ETA)见 ByteFormatExtended.h
 */
#ifndef BYTEFORMAT_H
#define BYTEFORMAT_H

#include <QString>
#include <QCoreApplication>
#include <QtGlobal>

/**
 * @brief 字节格式化工具命名空间
 *
 * 包含格式化字节大小、传输速率、时间长度等通用工具函数。
 * 纯静态方法，无需实例化。
 */
namespace ByteFormat {

/**
 * @brief 格式化传输速率
 *
 * 根据字节数和时间自动选择最合适的单位（B/s, KB/s, MB/s, GB/s），
 * 保留一位小数。秒数为零或负数时返回 "--"。
 *
 * @param bytes    传输的字节数（必须 >= 0）
 * @param seconds  传输耗时（秒），零或负数返回 "--"
 * @return         格式化后的速率字符串，如 "2.3 KB/s", "1.5 MB/s"
 */
inline QString formatRate(qint64 bytes, double seconds)
{
    if (seconds <= 0.0) {
        return QStringLiteral("--");
    }

    double bps = static_cast<double>(bytes) / seconds;

    struct UnitEntry {
        double threshold;
        const char* suffix;
    };
    static const UnitEntry units[] = {
        { 1.0,           "B/s"  },
        { 1024.0,        "KB/s" },
        { 1024.0 * 1024, "MB/s" },
        { 1024.0 * 1024.0 * 1024, "GB/s" }
    };

    for (int i = 3; i >= 0; --i) {
        if (bps >= units[i].threshold) {
            double value = bps / units[i].threshold;
            if (i == 0) {
                return QString("%1 %2").arg(static_cast<qint64>(bps)).arg(units[i].suffix);
            }
            return QString("%1 %2").arg(value, 0, 'f', 1).arg(units[i].suffix);
        }
    }

    return QString("%1 B/s").arg(bps, 0, 'f', 1);
}

/**
 * @brief 格式化字节大小
 *
 * 根据字节数自动选择最合适的单位（B, KB, MB, GB, TB），
 * 保留一位小数。零字节返回 "0 B"。
 *
 * @param bytes  字节数（必须 >= 0）
 * @return       格式化后的大小字符串，如 "512 B", "1.5 MB"
 */
inline QString formatSize(qint64 bytes)
{
    if (bytes <= 0) {
        return QStringLiteral("0 B");
    }

    struct UnitEntry {
        qint64 threshold;
        const char* suffix;
    };
    static const UnitEntry units[] = {
        { 1LL,                  "B"  },
        { 1024LL,               "KB" },
        { 1024LL * 1024,        "MB" },
        { 1024LL * 1024 * 1024, "GB" },
        { 1024LL * 1024 * 1024 * 1024, "TB" }
    };

    for (int i = 4; i >= 0; --i) {
        if (bytes >= units[i].threshold) {
            double value = static_cast<double>(bytes) / static_cast<double>(units[i].threshold);
            if (i == 0) {
                return QString("%1 %2").arg(bytes).arg(units[i].suffix);
            }
            return QString("%1 %2").arg(value, 0, 'f', 1).arg(units[i].suffix);
        }
    }

    return QString("%1 B").arg(bytes);
}

/**
 * @brief 格式化时间长度
 *
 * 将毫秒数转换为 "MM:SS" 或 "HH:MM:SS" 格式。
 * < 1 小时: "MM:SS"，>= 1 小时: "HH:MM:SS"。
 *
 * @param ms  时间长度（毫秒），必须 >= 0
 * @return    格式化后的时间字符串
 */
inline QString formatDuration(qint64 ms)
{
    if (ms <= 0) {
        return QStringLiteral("00:00");
    }

    qint64 totalSeconds = ms / 1000;
    qint64 hours   = totalSeconds / 3600;
    int minutes = static_cast<int>((totalSeconds % 3600) / 60);
    int seconds = static_cast<int>(totalSeconds % 60);

    if (hours > 0) {
        int h = static_cast<int>(qMin(hours, qint64(99)));
        return QString("%1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

} // namespace ByteFormat

/* 引入扩展格式化函数(紧凑速率/百分比/精确时间/ETA) */
#include "utils/data/ByteFormatExtended.h"

#endif // BYTEFORMAT_H
