#ifndef SERIAL_LOG_SERVICE_H
#define SERIAL_LOG_SERVICE_H

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>

namespace serial_station {

/**
 * @brief Serial Station 日志方向。
 */
enum class SerialLogDirection {
    Rx,
    Tx,
    System,
    Error
};

/**
 * @brief Serial Station 结构化日志记录。
 */
struct SerialLogRecord {
    QDateTime timestamp;
    SerialLogDirection direction = SerialLogDirection::System;
    QString source;
    QString text;
    QByteArray payload;
    QVariantMap fields;
};

/**
 * @brief Serial Station 日志查询条件。
 */
struct SerialLogFilter {
    QVector<SerialLogDirection> directions;
    QString sourceContains;
    QString textContains;
    QDateTime from;
    QDateTime to;
};

/**
 * @brief Serial Station 服务层日志仓库。
 *
 * 只负责结构化日志记录、查询和导出文本生成，不触碰 QWidget、串口线程或协议解析。
 */
class SerialLogService {
public:
    /**
     * @brief 默认最大日志条数。
     */
    static constexpr int kDefaultMaxRecords = 2000;

    /**
     * @brief 创建日志服务。
     * @param maxRecords 最大保留记录数，最小值会钳制为 1
     */
    explicit SerialLogService(int maxRecords = kDefaultMaxRecords);

    /**
     * @brief 追加结构化日志。
     * @param record 输入记录，timestamp/source/text 会被规范化
     * @return 是否成功追加
     */
    bool append(const SerialLogRecord& record);

    /**
     * @brief 追加发送日志。
     */
    bool appendTx(const QString& text,
                  const QByteArray& payload = QByteArray(),
                  const QString& source = QString(),
                  const QVariantMap& fields = QVariantMap());

    /**
     * @brief 追加接收日志。
     */
    bool appendRx(const QString& text,
                  const QByteArray& payload = QByteArray(),
                  const QString& source = QString(),
                  const QVariantMap& fields = QVariantMap());

    /**
     * @brief 追加系统日志。
     */
    bool appendSystem(const QString& text,
                      const QString& source = QString(),
                      const QVariantMap& fields = QVariantMap());

    /**
     * @brief 追加错误日志。
     */
    bool appendError(const QString& text,
                     const QString& source = QString(),
                     const QVariantMap& fields = QVariantMap());

    /**
     * @brief 清空所有日志。
     */
    void clear();

    /**
     * @brief 当前记录数。
     */
    int count() const;

    /**
     * @brief 是否没有记录。
     */
    bool isEmpty() const;

    /**
     * @brief 当前最大保留记录数。
     */
    int maxRecords() const;

    /**
     * @brief 设置最大保留记录数。
     * @param maxRecords 最大记录数，最小值会钳制为 1
     */
    void setMaxRecords(int maxRecords);

    /**
     * @brief 返回完整快照。
     */
    QVector<SerialLogRecord> records() const;

    /**
     * @brief 根据条件返回过滤快照。
     */
    QVector<SerialLogRecord> records(const SerialLogFilter& filter) const;

    /**
     * @brief 生成完整纯文本日志。
     */
    QString toPlainText() const;

    /**
     * @brief 生成过滤后的纯文本日志。
     */
    QString toPlainText(const SerialLogFilter& filter) const;

    /**
     * @brief 生成完整 JSON Lines 日志。
     */
    QString toJsonLines() const;

    /**
     * @brief 生成过滤后的 JSON Lines 日志。
     */
    QString toJsonLines(const SerialLogFilter& filter) const;

    /**
     * @brief 方向转换为稳定小写名称。
     */
    static QString directionName(SerialLogDirection direction);

    /**
     * @brief payload 转换为大写空格分隔 HEX。
     */
    static QString payloadHex(const QByteArray& payload);

private:
    static SerialLogRecord normalizedRecord(const SerialLogRecord& record);
    static bool isMeaningful(const SerialLogRecord& record);
    static bool matchesFilter(const SerialLogRecord& record, const SerialLogFilter& filter);
    static bool directionAccepted(SerialLogDirection direction,
                                  const QVector<SerialLogDirection>& directions);
    static QString recordToPlainTextLine(const SerialLogRecord& record);
    static QString recordToJsonLine(const SerialLogRecord& record);

    void trimOverflow();

    QVector<SerialLogRecord> m_records;
    int m_maxRecords = kDefaultMaxRecords;
};

} // namespace serial_station

#endif // SERIAL_LOG_SERVICE_H
