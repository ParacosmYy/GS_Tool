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

    /** @brief CAN错误帧分类枚举 */
    enum ErrorType : quint8 {
        ErrorNone = 0,      ///< 无错误
        BitError,           ///< 位错误: 发送的电平与监视的电平不一致
        StuffError,         ///< 填充错误: 连续6个相同电平位(违反位填充规则)
        CrcError,           ///< CRC错误: 接收端计算的CRC与收到的CRC不一致
        FormError,          ///< 格式错误: 固定格式位域中出现非法位
        AckError            ///< 应答错误: 发送端在ACK槽未检测到应答信号
    };
    ErrorType errorType = ErrorNone; ///< 错误帧分类(仅error=true时有效)

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

signals:
    /** @brief CAN错误帧检测信号 @param frame 包含错误信息的CanFrame @param errorType 错误分类 */
    void errorFrameDetected(const CanFrame& frame, CanFrame::ErrorType errorType);

public:
    /** @brief 构造CAN帧解析器 @param parent 父QObject指针 */
    explicit CanFrameParser(QObject* parent = nullptr);
    /** @brief 解析LAWICEL原始字符串为CanFrame @param rawData 原始帧数据 @return 解析后的CanFrame */
    CanFrame parseFrame(const QByteArray& rawData);
    /** @brief 将CanFrame编码为LAWICEL格式 @param frame CAN帧数据 @return 编码后的字节序列(不含\r) */
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
    /** @brief 获取累计解析帧数 */
    quint64 totalFramesParsed() const { return m_totalFramesParsed; }
    /** @brief 获取累计解析错误数 */
    quint64 totalParseErrors() const { return m_totalParseErrors; }
    /** @brief 获取累计处理字节数 */
    quint64 totalBytesProcessed() const { return m_totalBytesProcessed; }
    /** @brief 获取标准帧计数 */
    quint64 totalStandardFrames() const { return m_totalStandardFrames; }
    /** @brief 获取扩展帧计数 */
    quint64 totalExtendedFrames() const { return m_totalExtendedFrames; }
    /** @brief 获取CAN-FD帧计数 */
    quint64 totalFdFrames() const { return m_totalFdFrames; }
    /** @brief 获取RTR帧计数 */
    quint64 totalRtrFrames() const { return m_totalRtrFrames; }
    /** @brief 获取累计CRC校验错误数 */
    quint64 totalCrcErrors() const { return m_totalCrcErrors; }
    /** @brief 获取累计CAN错误帧总数 */
    quint64 totalErrorFrames() const { return m_totalErrorFrames; }
    /** @brief 获取累计位错误数 */
    quint64 totalBitErrors() const { return m_totalBitErrors; }
    /** @brief 获取累计填充错误数 */
    quint64 totalStuffErrors() const { return m_totalStuffErrors; }
    /** @brief 获取累计CRC错误帧数 */
    quint64 totalCrcFrameErrors() const { return m_totalCrcFrameErrors; }
    /** @brief 获取累计格式错误数 */
    quint64 totalFormErrors() const { return m_totalFormErrors; }
    /** @brief 获取累计应答错误数 */
    quint64 totalAckErrors() const { return m_totalAckErrors; }
    /** @brief 重置所有解析器统计计数器 */
    void resetParserStatistics();

private:
    /** @brief 解析两个十六进制字符为一个字节 @param hex 指向两个十六进制字符的指针 @return 解析后的字节值 */
    static quint8 parseHexByte(const char* hex);
    /** @brief 解析数据区中所有十六进制字节 @param d 数据区指针 @param byteCount 要解析的字节数 @return 解析后的QByteArray */
    static QByteArray parseHexData(const char* d, int byteCount);
    /** @brief 解析CAN错误帧数据，分类错误类型并发射信号 @param frame 接收到的错误帧引用 */
    void parseErrorFrame(CanFrame& frame);

    DbcParser* m_dbcParser = nullptr;   ///< DBC解析器(延迟创建)
    QString m_dbcFilePath;              ///< DBC文件路径

    // ---- 统计计数器 ----
    quint64 m_totalFramesParsed = 0;        ///< 累计解析帧数
    quint64 m_totalParseErrors = 0;         ///< 累计解析错误数
    quint64 m_totalBytesProcessed = 0;      ///< 累计处理字节数
    quint64 m_totalStandardFrames = 0;      ///< 标准帧计数
    quint64 m_totalExtendedFrames = 0;      ///< 扩展帧计数
    quint64 m_totalFdFrames = 0;            ///< CAN-FD帧计数
    quint64 m_totalRtrFrames = 0;           ///< RTR帧计数
    quint64 m_totalCrcErrors = 0;           ///< CRC校验错误计数
    quint64 m_totalErrorFrames = 0;         ///< CAN错误帧总数
    quint64 m_totalBitErrors = 0;           ///< 位错误计数
    quint64 m_totalStuffErrors = 0;         ///< 填充错误计数
    quint64 m_totalCrcFrameErrors = 0;      ///< CRC错误帧计数(与协议层CRC校验错误区分)
    quint64 m_totalFormErrors = 0;          ///< 格式错误计数
    quint64 m_totalAckErrors = 0;           ///< 应答错误计数
};

#endif // CANFRAMEPARSER_H
