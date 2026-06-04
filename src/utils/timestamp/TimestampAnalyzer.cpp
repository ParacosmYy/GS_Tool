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

/** @brief Unix时间戳转日期时间，累计转换计数 @param timestamp 时间戳值 @param isMillis true=毫秒精度 false=秒精度 @return 对应的QDateTime对象 */
QDateTime TimestampAnalyzer::unixToDatetime(qint64 timestamp, bool isMillis) const
{
    ++m_totalConversions;
    qint64 secs = isMillis ? (timestamp / 1000) : timestamp;
    return QDateTime::fromSecsSinceEpoch(secs);
}

/** @brief 日期时间转Unix时间戳，累计转换计数 @param datetime 日期时间对象 @param asMillis true=返回毫秒精度 false=返回秒精度 @return Unix时间戳 */
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

/** @brief 解析时间戳字符串(支持Unix秒/毫秒/ISO日期)，累计解析计数和字节数 @param text 时间戳文本 @return 解析后的QDateTime，失败返回无效对象 */
QDateTime TimestampAnalyzer::parseTimestamp(const QString &text) const
{
    ++m_totalParses;
    ++m_totalAnalyses;
    m_totalBytesAnalyzed += static_cast<quint64>(text.toUtf8().size());
    if (text.isEmpty()) { ++m_totalParseErrors; return {}; }

    bool ok = false;
    qint64 value = text.toLongLong(&ok);

    if (ok) {
        if (value > 1000000000000LL) {
            return unixToDatetime(value, true);
        }
        return unixToDatetime(value, false);
    }

    /* 数字解析失败，切换到日期格式尝试 — 累计格式变更 */
    ++m_totalFormatChanges;

    QDateTime dt = QDateTime::fromString(text, Qt::ISODate);
    if (dt.isValid()) return dt;

    dt = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (dt.isValid()) return dt;

    dt = QDateTime::fromString(text, QStringLiteral("yyyy/MM/dd HH:mm:ss"));
    if (dt.isValid()) return dt;

    /* 所有格式尝试均失败 — 累计解析错误 */
    ++m_totalParseErrors;
    return {};
}

/** @brief 格式化为相对时间描述(如"3分钟前"、"2小时后") @param datetime 目标日期时间 @return 相对时间字符串 */
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

/** @brief 格式化两个时间点之间的差值(如"2天3小时15分钟") @param from 起始时间 @param to 结束时间 @return 差值描述字符串 */
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

/** @brief 获取累计时间戳转换次数 @return 转换总次数 */
quint64 TimestampAnalyzer::totalConversions() const { return m_totalConversions; }
/** @brief 获取累计时间戳解析次数 @return 解析总次数 */
quint64 TimestampAnalyzer::totalParses() const { return m_totalParses; }
/** @brief 获取累计解析的字节总数 @return 字节总数 */
quint64 TimestampAnalyzer::totalBytesAnalyzed() const { return m_totalBytesAnalyzed; }
/** @brief 重置所有统计计数器(转换次数/解析次数/字节数/分析次数/格式变更/解析错误归零) */
void TimestampAnalyzer::resetStats() { m_totalConversions = 0; m_totalParses = 0; m_totalBytesAnalyzed = 0; m_totalAnalyses = 0; m_totalFormatChanges = 0; m_totalParseErrors = 0; }
