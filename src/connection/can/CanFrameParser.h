/**
 * @file CanFrameParser.h
 * @brief CAN帧解析器 — 解析/构建CAN/CAN-FD帧，支持DBC信号解码
 *
 * 职责: LAWICEL/SLCAN协议格式与CanFrame结构体之间的双向转换，
 * 可选加载DBC数据库文件解码帧数据。支持经典CAN(8字节)和CAN-FD(64字节)。
 */
#ifndef CANFRAMEPARSER_H
#define CANFRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>

class DbcParser;

/**
 * @brief CAN/CAN-FD帧结构体
 *
 * 包含帧ID、数据负载、帧类型标志和DLC。支持经典CAN(8字节)和CAN-FD(64字节)。
 */
struct CanFrame {
    quint32 id = 0;         ///< 帧ID(标准11位或扩展29位)
    QByteArray data;        ///< 帧数据负载(经典CAN≤8字节，CAN-FD≤64字节)
    bool extended = false;  ///< true=扩展帧(29位ID)
    bool rtr = false;       ///< true=远程帧(请求数据，无负载)
    bool fd = false;        ///< true=CAN-FD帧(支持更长数据和更高速率)
    quint8 dlc = 0;         ///< 数据长度码(Data Length Code)
    bool error = false;     ///< true=错误帧

    /** @brief 根据CAN-FD DLC获取实际数据字节数 */
    static int dlcToBytes(int dlc) {
        switch (dlc) {
        case 0: return 0;   case 1: return 1;   case 2: return 2;
        case 3: return 3;   case 4: return 4;   case 5: return 5;
        case 6: return 6;   case 7: return 7;   case 8: return 8;
        case 9: return 12;  case 10: return 16;  case 11: return 20;
        case 12: return 24; case 13: return 32;  case 14: return 48;
        case 15: return 64; default: return 8;
        }
    }

    /** @brief 根据字节数获取CAN-FD DLC */
    static int bytesToDlc(int bytes) {
        if (bytes <= 8) return bytes;
        if (bytes <= 12) return 9;
        if (bytes <= 16) return 10;
        if (bytes <= 20) return 11;
        if (bytes <= 24) return 12;
        if (bytes <= 32) return 13;
        if (bytes <= 48) return 14;
        return 15;
    }
};

/**
 * @brief CAN帧解析与构建工具
 *
 * 提供LAWICEL/SLCAN协议格式与CanFrame之间的双向转换，
 * 以及DBC数据库文件加载(用于信号解码)。支持CAN-FD帧格式。
 */
class CanFrameParser : public QObject {
    Q_OBJECT

public:
    explicit CanFrameParser(QObject* parent = nullptr);
    /** @brief 解析LAWICEL原始字符串为CanFrame @param rawData 原始帧数据 @return 解析结果 */
    CanFrame parseFrame(const QByteArray& rawData);
    /** @brief 将CanFrame编码为LAWICEL格式 @return 编码后的字节序列(不含\r) */
    QByteArray buildFrame(const CanFrame& frame);
    /** @brief 加载DBC数据库文件 @param filePath DBC文件路径 @return 成功返回true */
    bool loadDbcFile(const QString& filePath);
    /** @brief 使用已加载的DBC解码CAN帧信号值 @return 信号名→物理值映射 */
    QMap<QString, double> decodeSignals(const CanFrame& frame) const;
    /** @brief 获取DBC消息名 @param frameId 帧ID @return 消息名，未匹配返回空 */
    QString messageName(quint32 frameId) const;
    /** @brief 将CanFrame转换为可读字符串 */
    static QString frameToString(const CanFrame& frame);

    // ---- 统计信息接口 ----
    quint64 totalFramesParsed() const { return m_totalFramesParsed; }
    quint64 totalParseErrors() const { return m_totalParseErrors; }
    quint64 totalBytesProcessed() const { return m_totalBytesProcessed; }
    quint64 totalStandardFrames() const { return m_totalStandardFrames; }
    quint64 totalExtendedFrames() const { return m_totalExtendedFrames; }
    quint64 totalFdFrames() const { return m_totalFdFrames; }
    quint64 totalRtrFrames() const { return m_totalRtrFrames; }
    void resetParserStatistics();

private:
    static quint8 parseHexByte(const char* hex);
    static QByteArray parseHexData(const char* d, int byteCount);

    DbcParser* m_dbcParser = nullptr;   ///< DBC解析器(延迟创建)
    QString m_dbcFilePath;              ///< DBC文件路径

    // ---- 统计计数器 ----
    quint64 m_totalFramesParsed = 0;
    quint64 m_totalParseErrors = 0;
    quint64 m_totalBytesProcessed = 0;
    quint64 m_totalStandardFrames = 0;
    quint64 m_totalExtendedFrames = 0;
    quint64 m_totalFdFrames = 0;
    quint64 m_totalRtrFrames = 0;
};

#endif // CANFRAMEPARSER_H
