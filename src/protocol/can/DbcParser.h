/**
 * @file DbcParser.h
 * @brief DBC(CAN数据库)文件解析器
 *
 * 解析Vector CANdb++格式的DBC文件，提取消息定义、信号定义、
 * 值表(VAL_)等。供CanBusMonitor显示解码后的信号值。
 */

#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

/**
 * @brief DBC信号定义
 */
struct DbcSignal {
    QString name;           ///< 信号名
    int startBit = 0;       ///< 起始位
    int bitLength = 0;      ///< 位长度
    int byteOrder = 1;      ///< 字节序(1=小端,0=大端)
    double factor = 1.0;    ///< 缩放因子
    double offset = 0.0;    ///< 偏移量
    double minimum = 0.0;   ///< 最小值
    double maximum = 1.0;   ///< 最大值
    QString unit;           ///< 单位
    QString receiver;       ///< 接收节点
    QString comment;        ///< 信号注释(CM_ SG_ ...)
    QMap<int, QString> valueTable; ///< 值表(原始值→描述)
};

/**
 * @brief DBC消息定义
 */
struct DbcMessage {
    uint32_t id = 0;               ///< 消息ID(29位扩展帧或11位标准帧)
    QString name;                   ///< 消息名
    int dlc = 8;                    ///< 数据长度码
    QString transmitter;            ///< 发送节点
    QString comment;                ///< 注释
    QList<DbcSignal> signalList;    ///< 信号列表
    QMap<QString, QString> attributes; ///< 自定义属性
};

/**
 * @brief DBC文件解析器
 *
 * 支持解析DBC文件的以下关键字段:
 * - BO_: 消息定义
 * - SG_: 信号定义
 * - VAL_: 值表
 * - CM_: 注释
 * - BA_: 属性定义
 * - BU_: 节点定义
 */
class DbcParser : public QObject {
    Q_OBJECT

public:
    explicit DbcParser(QObject* parent = nullptr);

    /** @brief 从文件加载DBC
     *  @param filePath DBC文件路径
     *  @return true=解析成功
     */
    bool loadFromFile(const QString& filePath);

    /** @brief 从文本内容解析DBC
     *  @param content DBC文件文本内容
     *  @return true=解析成功
     */
    bool parseFromText(const QString& content);

    /** @brief 获取所有消息定义 */
    QList<DbcMessage> messages() const;

    /** @brief 根据消息ID查找消息定义 */
    DbcMessage messageById(uint32_t id) const;

    /** @brief 根据消息名查找消息定义 */
    DbcMessage messageByName(const QString& name) const;

    /** @brief 解码CAN帧数据为信号值映射
     *  @param msgId 消息ID
     *  @param data 帧数据(最多8字节)
     *  @return 信号名→物理值映射
     */
    QMap<QString, double> decodeFrame(uint32_t msgId, const QByteArray& data) const;

    /** @brief 获取信号物理值的文本描述(含值表翻译)
     *  @param msgId 消息ID
     *  @param signalName 信号名
     *  @param rawValue 原始值
     *  @return 格式化字符串(如 "45.0 km/h" 或 "Active")
     */
    QString formatSignalValue(uint32_t msgId, const QString& signalName, double rawValue) const;

    /** @brief 获取解析错误信息 */
    QString lastError() const;

    /** @brief 获取已解析的消息数量 */
    int messageCount() const;

    /** @brief 获取所有节点名称 */
    QStringList nodes() const;

    /** @brief 清除解析数据 */
    void clear();

    // ---- 统计接口 ----
    quint64 totalParses() const;            ///< 累计解析次数
    quint64 totalMessagesParsed() const;    ///< 累计解析消息总数
    quint64 totalSignalsDecoded() const;    ///< 累计解码信号总数
    quint64 totalParseErrors() const;       ///< 累计解析错误次数
    void resetDbcStatistics();              ///< 重置统计计数器

signals:
    /** @brief 解析完成信号 */
    void parseCompleted(int messageCount);

private:
    bool parseMessageLine(const QString& line);
    bool parseSignalLine(const QString& line, uint32_t currentMsgId);
    bool parseValueTableLine(const QString& line);
    bool parseCommentLine(const QString& line);
    bool parseAttributeLine(const QString& line);
    bool parseNodeLine(const QString& line);

    /** @brief 从字节数据中提取指定位域的原始值 */
    uint64_t extractBits(const QByteArray& data, int startBit, int bitLength, int byteOrder) const;

    QMap<uint32_t, DbcMessage> m_messages;   ///< ID→消息映射
    QMap<QString, uint32_t> m_nameToId;      ///< 消息名→ID映射
    QStringList m_nodes;                      ///< 节点列表
    QString m_lastError;                      ///< 最后错误信息

    // ---- 统计计数器 ----
    quint64 m_totalParses = 0;               ///< 累计解析次数
    quint64 m_totalMessagesParsed = 0;       ///< 累计解析消息总数
    mutable quint64 m_totalSignalsDecoded = 0; ///< 累计解码信号总数(const方法中递增)
    quint64 m_totalParseErrors = 0;          ///< 累计解析错误次数
};

#endif // DBCPARSER_H
