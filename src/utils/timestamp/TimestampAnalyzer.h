/**
 * @file TimestampAnalyzer.h
 * @brief 时间戳转换分析器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供 Unix 时间戳与日期时间之间的相互转换。
 */

#ifndef TIMESTAMPANALYZER_H
#define TIMESTAMPANALYZER_H

#include <QDateTime>
#include <QObject>
#include <QString>

/**
 * @class TimestampAnalyzer
 * @brief 时间戳转换引擎，纯计算无状态
 */
class TimestampAnalyzer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TimestampAnalyzer(QObject *parent = nullptr);

    /**
     * @brief Unix 时间戳转日期时间
     * @param timestamp Unix 时间戳（秒或毫秒）
     * @param isMillis 是否为毫秒级时间戳
     * @return 对应的日期时间
     */
    QDateTime unixToDatetime(qint64 timestamp, bool isMillis = false) const;

    /**
     * @brief 日期时间转 Unix 时间戳
     * @param datetime 日期时间
     * @param asMillis 是否输出毫秒级时间戳
     * @return Unix 时间戳
     */
    qint64 datetimeToUnix(const QDateTime &datetime, bool asMillis = false) const;

    /**
     * @brief 获取当前 Unix 时间戳
     * @param millis 是否返回毫秒级
     * @return 当前时间戳
     */
    static qint64 currentUnix(bool millis = false);

    /**
     * @brief 解析时间戳字符串
     * @param text 时间戳文本（秒/毫秒/ISO日期）
     * @return 解析后的日期时间
     */
    QDateTime parseTimestamp(const QString &text) const;

    /**
     * @brief 格式化相对时间（如"3分钟前"、"2小时前"）
     * @param datetime 目标日期时间
     * @return 相对时间描述字符串
     */
    static QString formatRelativeTime(const QDateTime& datetime);

    /**
     * @brief 计算两个时间点之间的差异
     * @param from 起始时间
     * @param to 结束时间
     * @return 格式化的差异字符串（如"2天3小时15分钟"）
     */
    static QString formatDifference(const QDateTime& from, const QDateTime& to);
};

#endif // TIMESTAMPANALYZER_H
