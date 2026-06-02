/**
 * @file CanFrameParser.h
 * @brief CAN帧解析器 — 解析/构建CAN/CAN-FD帧，支持DBC文件加载
 *
 * 职责: 原始字节与CanFrame结构体之间的转换，
 * 可选加载DBC数据库文件解码帧数据。
 */
#ifndef CANFRAMEPARSER_H
#define CANFRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QString>

/**
 * @brief CAN/CAN-FD帧结构体
 *
 * 包含帧ID、数据负载、帧类型标志和DLC(数据长度码)。
 */
struct CanFrame {
    quint32 id = 0;         ///< 帧ID(标准11位或扩展29位)
    QByteArray data;        ///< 帧数据负载(经典CAN最多8字节，CAN-FD最多64字节)
    bool extended = false;  ///< true=扩展帧(29位ID)
    bool rtr = false;       ///< true=远程帧(请求数据，无负载)
    bool fd = false;        ///< true=CAN-FD帧(支持更长数据和更高速率)
    quint8 dlc = 0;         ///< 数据长度码(Data Length Code)
};

/**
 * @brief CAN帧解析与构建工具
 *
 * 提供原始字节序列与CanFrame结构体之间的双向转换，
 * 以及DBC数据库文件加载(用于信号解码)。
 */
class CanFrameParser : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造CAN帧解析器
     * @param parent 父对象
     */
    explicit CanFrameParser(QObject* parent = nullptr);

    /**
     * @brief 从原始字节解析CAN帧
     * @param rawData 原始字节序列
     * @return 解析后的CanFrame
     */
    CanFrame parseFrame(const QByteArray& rawData);

    /**
     * @brief 将CanFrame编码为原始字节
     * @param frame CAN帧结构体
     * @return 编码后的字节序列
     */
    QByteArray buildFrame(const CanFrame& frame);

    /**
     * @brief 加载DBC数据库文件
     * @param filePath DBC文件路径
     * @return true=加载成功
     */
    bool loadDbcFile(const QString& filePath);

private:
    /** @brief 当前加载的DBC文件路径 */
    QString m_dbcFilePath;
};

#endif // CANFRAMEPARSER_H
