/**
 * @file TimestampAnalyzer.cpp
 * @brief 时间戳转换分析器实现
 */

#include "utils/timestamp/TimestampAnalyzer.h"

/** @brief 构造函数 @param parent 父对象 */
TimestampAnalyzer::TimestampAnalyzer(QObject *parent)
    : QObject(parent)
{
}

/** @brief Unix时间戳转日期时间 — 累计转换计数 */
QDateTime TimestampAnalyzer::unixToDatetime(qint64 timestamp, bool isMillis) const
{
    ++m_totalConversions;
    qint64 secs = isMillis ? (timestamp / 1000) : timestamp;
    return QDateTime::fromSecsSinceEpoch(secs);
}

/** @brief 日期时间转Unix时间戳 — 累计转换计数 */
qint64 TimestampAnalyzer::datetimeToUnix(const QDateTime &datetime, bool asMillis) const
{
    ++m_totalConversions;
    qint64 secs = datetime.toSecsSinceEpoch();
    return asMillis ? (secs * 1000) : secs;
}

/** @brief 获取当前Unix时间戳 @param millis 是否返回毫秒精度 @return 当前时间戳 */
qint64 TimestampAnalyzer::currentUnix(bool millis)
{
    qint64 secs = QDateTime::currentSecsSinceEpoch();
    return millis ? (secs * 1000) : secs;
}

/** @brief 解析时间戳字符串 — 累计解析计数和字节数 */
QDateTime TimestampAnalyzer::parseTimestamp(const QString &text) const
{
    ++m_totalParses;
    m_totalBytesAnalyzed += static_cast<quint64>(text.toUtf8().size());
    if (text.isEmpty()) return {};

    bool ok = false;
    qint64 value = text.toLongLong(&ok);

    if (ok) {
        if (value > 1000000000000LL) {
            return unixToDatetime(value, true);
        }
        return unixToDatetime(value, false);
    }

    QDateTime dt = QDateTime::fromString(text, Qt::ISODate);
    if (dt.isValid()) return dt;

    dt = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (dt.isValid()) return dt;

    dt = QDateTime::fromString(text, QStringLiteral("yyyy/MM/dd HH:mm:ss"));
    if (dt.isValid()) return dt;

    return {};
}

QString TimestampAnalyzer::formatRelativeTime(const QDateTime& datetime)
{
    if (!datetime.isValid()) return QString();

    const qint64 secs = datetime.secsTo(QDateTime::currentDateTime());

    if (secs < 0) {
        qint64 absSecs = -secs;
        if (absSecs < 60) return QObject::tr("%1秒后").arg(absSecs);
        if (absSecs < 3600) return QObject::tr("%1分钟后").arg(absSecs / 60);
        if (absSecs < 86400) return QObject::tr("%1小时后").arg(absSecs / 3600);
        return QObject::tr("%1天后").arg(absSecs / 86400);
    }

    if (secs < 5) return QObject::tr("刚刚");
    if (secs < 60) return QObject::tr("%1秒前").arg(secs);
    if (secs < 3600) return QObject::tr("%1分钟前").arg(secs / 60);
    if (secs < 86400) return QObject::tr("%1小时前").arg(secs / 3600);
    if (secs < 2592000) return QObject::tr("%1天前").arg(secs / 86400);
    if (secs < 31536000) return QObject::tr("%1个月前").arg(secs / 2592000);
    return QObject::tr("%1年前").arg(secs / 31536000);
}

QString TimestampAnalyzer::formatDifference(const QDateTime& from, const QDateTime& to)
{
    if (!from.isValid() || !to.isValid()) return QString();

    qint64 totalSecs = qAbs(from.secsTo(to));
    const qint64 days = totalSecs / 86400;
    totalSecs %= 86400;
    const qint64 hours = totalSecs / 3600;
    totalSecs %= 3600;
    const qint64 minutes = totalSecs / 60;
    totalSecs %= 60;
    const qint64 seconds = totalSecs;

    QStringList parts;
    if (days > 0) parts.append(QObject::tr("%1天").arg(days));
    if (hours > 0) parts.append(QObject::tr("%1小时").arg(hours));
    if (minutes > 0) parts.append(QObject::tr("%1分钟").arg(minutes));
    if (seconds > 0 && days == 0) parts.append(QObject::tr("%1秒").arg(seconds));

    return parts.isEmpty() ? QObject::tr("0秒") : parts.join(QString());
}

quint64 TimestampAnalyzer::totalConversions() const { return m_totalConversions; }
quint64 TimestampAnalyzer::totalParses() const { return m_totalParses; }
quint64 TimestampAnalyzer::totalBytesAnalyzed() const { return m_totalBytesAnalyzed; }
void TimestampAnalyzer::resetStats() { m_totalConversions = 0; m_totalParses = 0; m_totalBytesAnalyzed = 0; }
