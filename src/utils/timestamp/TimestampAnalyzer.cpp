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
    return QDateTime::fromSecsSinceEpoch(secs);
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

/**
 * @brief 格式化相对时间描述
 *
 * 将目标时间与当前时间比较，返回人类可读的相对描述。
 * 例如: "刚刚", "5分钟前", "2小时前", "3天前"
 *
 * @param datetime 目标日期时间
 * @return 相对时间描述字符串
 */
QString TimestampAnalyzer::formatRelativeTime(const QDateTime& datetime)
{
    if (!datetime.isValid()) {
        return QString();
    }

    const qint64 secs = datetime.secsTo(QDateTime::currentDateTime());

    if (secs < 0) {
        /* 未来时间 */
        qint64 absSecs = -secs;
        if (absSecs < 60) {
            return QObject::tr("%1秒后").arg(absSecs);
        } else if (absSecs < 3600) {
            return QObject::tr("%1分钟后").arg(absSecs / 60);
        } else if (absSecs < 86400) {
            return QObject::tr("%1小时后").arg(absSecs / 3600);
        } else {
            return QObject::tr("%1天后").arg(absSecs / 86400);
        }
    }

    /* 过去时间 */
    if (secs < 5) {
        return QObject::tr("刚刚");
    } else if (secs < 60) {
        return QObject::tr("%1秒前").arg(secs);
    } else if (secs < 3600) {
        return QObject::tr("%1分钟前").arg(secs / 60);
    } else if (secs < 86400) {
        return QObject::tr("%1小时前").arg(secs / 3600);
    } else if (secs < 2592000) {
        return QObject::tr("%1天前").arg(secs / 86400);
    } else if (secs < 31536000) {
        return QObject::tr("%1个月前").arg(secs / 2592000);
    } else {
        return QObject::tr("%1年前").arg(secs / 31536000);
    }
}

/**
 * @brief 计算并格式化两个时间点的差异
 *
 * 返回格式如 "2天3小时15分钟" 或 "5小时30分钟"
 *
 * @param from 起始时间
 * @param to 结束时间
 * @return 格式化的时间差异字符串
 */
QString TimestampAnalyzer::formatDifference(const QDateTime& from, const QDateTime& to)
{
    if (!from.isValid() || !to.isValid()) {
        return QString();
    }

    qint64 totalSecs = qAbs(from.secsTo(to));

    const qint64 days = totalSecs / 86400;
    totalSecs %= 86400;
    const qint64 hours = totalSecs / 3600;
    totalSecs %= 3600;
    const qint64 minutes = totalSecs / 60;
    totalSecs %= 60;
    const qint64 seconds = totalSecs;

    QStringList parts;

    if (days > 0) {
        parts.append(QObject::tr("%1天").arg(days));
    }
    if (hours > 0) {
        parts.append(QObject::tr("%1小时").arg(hours));
    }
    if (minutes > 0) {
        parts.append(QObject::tr("%1分钟").arg(minutes));
    }
    if (seconds > 0 && days == 0) {
        /* 仅在小于1天时显示秒数 */
        parts.append(QObject::tr("%1秒").arg(seconds));
    }

    return parts.isEmpty() ? QObject::tr("0秒") : parts.join(QString());
}
