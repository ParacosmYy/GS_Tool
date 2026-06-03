/**
 * @file TimestampAnalyzer.h
 * @brief 时间戳转换分析器 - 提供Unix时间戳与日期时间的相互转换
 *
 * 支持秒级/毫秒级时间戳转换、相对时间格式化、时间差计算。
 * 纯计算工具类，无外部状态依赖。
 */

#ifndef TIMESTAMPANALYZER_H
#define TIMESTAMPANALYZER_H

#include <QDateTime>
#include <QObject>
#include <QString>

/**
 * @brief 时间戳转换引擎
 *
 * 提供时间戳解析、转换和格式化功能。
 * 跟踪累计转换次数供统计面板使用。
 */
class TimestampAnalyzer : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TimestampAnalyzer(QObject *parent = nullptr);

    /** @brief Unix时间戳转日期时间 */
    QDateTime unixToDatetime(qint64 timestamp, bool isMillis = false) const;

    /** @brief 日期时间转Unix时间戳 */
    qint64 datetimeToUnix(const QDateTime &datetime, bool asMillis = false) const;

    /** @brief 获取当前Unix时间戳 */
    static qint64 currentUnix(bool millis = false);

    /** @brief 解析时间戳字符串 */
    QDateTime parseTimestamp(const QString &text) const;

    /** @brief 格式化相对时间（如"3分钟前"） */
    static QString formatRelativeTime(const QDateTime& datetime);

    /** @brief 计算两个时间点之间的差异 */
    static QString formatDifference(const QDateTime& from, const QDateTime& to);

    /** @brief 获取累计转换次数 */
    quint64 totalConversions() const;
    /** @brief 获取累计解析次数 */
    quint64 totalParses() const;
    /** @brief 重置统计计数器 */
    void resetStats();

private:
    mutable quint64 m_totalConversions = 0; ///< 累计转换次数
    mutable quint64 m_totalParses = 0;      ///< 累计解析次数
};

#endif // TIMESTAMPANALYZER_H
