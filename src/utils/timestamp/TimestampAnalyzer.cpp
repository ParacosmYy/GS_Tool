/**
 * @file TimestampAnalyzer.cpp
 * @brief 时间戳转换分析器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/timestamp/TimestampAnalyzer.h"

/**
 * @brief 构造函数
 */
TimestampAnalyzer::TimestampAnalyzer(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief Unix 时间戳转日期时间
 */
QDateTime TimestampAnalyzer::unixToDatetime(qint64 timestamp, bool isMillis) const
{
    qint64 secs = isMillis ? (timestamp / 1000) : timestamp;
    return QDateTime::fromSecsSinceEpoch(secs, Qt::LocalTime);
}

/**
 * @brief 日期时间转 Unix 时间戳
 */
qint64 TimestampAnalyzer::datetimeToUnix(const QDateTime &datetime, bool asMillis) const
{
    qint64 secs = datetime.toSecsSinceEpoch();
    return asMillis ? (secs * 1000) : secs;
}

/**
 * @brief 获取当前 Unix 时间戳
 */
qint64 TimestampAnalyzer::currentUnix(bool millis)
{
    qint64 secs = QDateTime::currentSecsSinceEpoch();
    return millis ? (secs * 1000) : secs;
}

/**
 * @brief 解析时间戳字符串（秒/毫秒/ISO格式）
 */
QDateTime TimestampAnalyzer::parseTimestamp(const QString &text) const
{
    if (text.isEmpty()) {
        return {};
    }

    // 尝试作为数字解析
    bool ok = false;
    qint64 value = text.toLongLong(&ok);

    if (ok) {
        // 判断秒级还是毫秒级（毫秒级时间戳通常大于 1e12）
        if (value > 1000000000000LL) {
            return unixToDatetime(value, true);
        }
        return unixToDatetime(value, false);
    }

    // 尝试 ISO 日期格式
    QDateTime dt = QDateTime::fromString(text, Qt::ISODate);
    if (dt.isValid()) {
        return dt;
    }

    // 尝试常见格式
    dt = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (dt.isValid()) {
        return dt;
    }

    dt = QDateTime::fromString(text, QStringLiteral("yyyy/MM/dd HH:mm:ss"));
    if (dt.isValid()) {
        return dt;
    }

    return {};
}
