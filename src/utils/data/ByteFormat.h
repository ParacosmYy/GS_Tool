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
 * 设计原则:
 *   - 每个函数只做一件事
 *   - 输出格式人类可读，自动选择合适的单位
 *   - 不抛异常，对边界输入（负数、零）做合理处理
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
 * 单位换算: 1 KB = 1024 B, 1 MB = 1024 KB, 1 GB = 1024 MB
 *
 * @param bytes    传输的字节数（必须 >= 0）
 * @param seconds  传输耗时（秒），零或负数返回 "--"
 * @return         格式化后的速率字符串，如 "2.3 KB/s", "1.5 MB/s"
 *
 * 示例:
 *   formatRate(2304, 1.0)     → "2.3 KB/s"
 *   formatRate(1572864, 1.0)  → "1.5 MB/s"
 *   formatRate(500, 0.0)      → "--"
 */
inline QString formatRate(qint64 bytes, double seconds)
{
    // 秒数为零或负数时无法计算速率，返回占位符
    if (seconds <= 0.0) {
        return QStringLiteral("--");
    }

    // 计算每秒字节数
    double bps = static_cast<double>(bytes) / seconds;

    // 定义单位列表和对应阈值（1024倍进位）
    struct UnitEntry {
        double threshold;      ///< 使用该单位的最小字节数
        const char* suffix;    ///< 单位后缀
    };
    static const UnitEntry units[] = {
        { 1.0,           "B/s"  },  // 字节/秒
        { 1024.0,        "KB/s" },  // 千字节/秒
        { 1024.0 * 1024, "MB/s" },  // 兆字节/秒
        { 1024.0 * 1024.0 * 1024, "GB/s" }  // 吉字节/秒
    };

    // 从大到小查找合适的单位
    for (int i = 3; i >= 0; --i) {
        if (bps >= units[i].threshold) {
            double value = bps / units[i].threshold;
            // 单位为 B/s 时不显示小数
            if (i == 0) {
                return QString("%1 %2").arg(static_cast<qint64>(bps)).arg(units[i].suffix);
            }
            return QString("%1 %2").arg(value, 0, 'f', 1).arg(units[i].suffix);
        }
    }

    // bps < 1.0 B/s 时，显示一位小数
    return QString("%1 B/s").arg(bps, 0, 'f', 1);
}

/**
 * @brief 格式化字节大小
 *
 * 根据字节数自动选择最合适的单位（B, KB, MB, GB, TB），
 * 保留一位小数。零字节返回 "0 B"。
 *
 * 单位换算: 1 KB = 1024 B, 1 MB = 1024 KB，以此类推。
 *
 * @param bytes  字节数（必须 >= 0）
 * @return       格式化后的大小字符串，如 "512 B", "1.5 MB"
 *
 * 示例:
 *   formatSize(512)       → "512 B"
 *   formatSize(1536)      → "1.5 KB"
 *   formatSize(1572864)   → "1.5 MB"
 *   formatSize(0)         → "0 B"
 */
inline QString formatSize(qint64 bytes)
{
    // 零字节特殊处理
    if (bytes <= 0) {
        return QStringLiteral("0 B");
    }

    // 定义单位列表
    struct UnitEntry {
        qint64 threshold;      ///< 使用该单位的最小字节数
        const char* suffix;    ///< 单位后缀
    };
    static const UnitEntry units[] = {
        { 1LL,                  "B"  },
        { 1024LL,               "KB" },
        { 1024LL * 1024,        "MB" },
        { 1024LL * 1024 * 1024, "GB" },
        { 1024LL * 1024 * 1024 * 1024, "TB" }
    };

    // 从大到小查找合适的单位
    for (int i = 4; i >= 0; --i) {
        if (bytes >= units[i].threshold) {
            double value = static_cast<double>(bytes) / static_cast<double>(units[i].threshold);
            // 单位为 B 时不显示小数
            if (i == 0) {
                return QString("%1 %2").arg(bytes).arg(units[i].suffix);
            }
            return QString("%1 %2").arg(value, 0, 'f', 1).arg(units[i].suffix);
        }
    }

    // 理论上不会到达这里（bytes >= 1 时至少匹配 B 单位）
    return QString("%1 B").arg(bytes);
}

/**
 * @brief 格式化时间长度
 *
 * 将毫秒数转换为 "MM:SS" 或 "HH:MM:SS" 格式。
 * 自动根据时长选择格式:
 *   - < 1 小时:  "MM:SS"（如 "02:35"）
 *   - >= 1 小时: "HH:MM:SS"（如 "01:02:35"）
 *
 * 零或负数毫秒返回 "00:00"。
 *
 * @param ms  时间长度（毫秒），必须 >= 0
 * @return    格式化后的时间字符串
 *
 * 示例:
 *   formatDuration(0)        → "00:00"
 *   formatDuration(155000)   → "02:35"
 *   formatDuration(3661500)  → "01:01:01"
 *   formatDuration(86400000) → "24:00:00"
 */
inline QString formatDuration(qint64 ms)
{
    // 零或负数特殊处理
    if (ms <= 0) {
        return QStringLiteral("00:00");
    }

    // 将毫秒转换为各时间单位
    qint64 totalSeconds = ms / 1000;
    qint64 hours   = totalSeconds / 3600;
    int minutes = static_cast<int>((totalSeconds % 3600) / 60);
    int seconds = static_cast<int>(totalSeconds % 60);

    // 超过1小时时显示 HH:MM:SS 格式
    if (hours > 0) {
        int h = static_cast<int>(qMin(hours, qint64(99)));
        return QString("%1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    // 不足1小时时显示 MM:SS 格式
    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

} // namespace ByteFormat

// 以下为扩展格式化函数，保持命名空间内

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

#endif // BYTEFORMAT_H
