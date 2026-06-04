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

    /** @brief Unix时间戳转日期时间 @param timestamp 时间戳值 @param isMillis true=毫秒精度，false=秒精度 @return 对应的QDateTime对象 */
    QDateTime unixToDatetime(qint64 timestamp, bool isMillis = false) const;

    /** @brief 日期时间转Unix时间戳 @param datetime 日期时间对象 @param asMillis true=返回毫秒精度，false=返回秒精度 @return Unix时间戳 */
    qint64 datetimeToUnix(const QDateTime &datetime, bool asMillis = false) const;

    /** @brief 获取当前Unix时间戳 @param millis true=返回毫秒精度，false=返回秒精度 @return 当前时间戳 */
    static qint64 currentUnix(bool millis = false);

    /** @brief 解析时间戳字符串(支持Unix秒/毫秒/ISO日期) @param text 时间戳文本 @return 解析后的QDateTime，失败返回无效对象 */
    QDateTime parseTimestamp(const QString &text) const;

    /** @brief 格式化相对时间描述(如"3分钟前"、"2小时后") @param datetime 目标日期时间 @return 相对时间字符串 */
    static QString formatRelativeTime(const QDateTime& datetime);

    /** @brief 计算两个时间点之间的差值(如"2天3小时15分钟") @param from 起始时间 @param to 结束时间 @return 差值描述字符串 */
    static QString formatDifference(const QDateTime& from, const QDateTime& to);

    /** @brief 获取累计转换次数 */
    quint64 totalConversions() const;
    /** @brief 获取累计解析次数 */
    quint64 totalParses() const;
    /** @brief 获取累计分析字节数(输入字符串长度总和) */
    quint64 totalBytesAnalyzed() const;
    /** @brief 重置统计计数器 */
    void resetStats();

private:
    mutable quint64 m_totalConversions = 0; ///< 累计转换次数
    mutable quint64 m_totalParses = 0;      ///< 累计解析次数
    mutable quint64 m_totalBytesAnalyzed = 0; ///< 累计分析字节数
};

#endif // TIMESTAMPANALYZER_H
