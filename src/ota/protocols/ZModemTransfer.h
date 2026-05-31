/**
 * @file ZModemTransfer.h
 * @brief ZMODEM协议传输器 - PC端Sender实现
 *
 * 基于BaseTransfer模板方法框架，实现ZMODEM协议的发送端逻辑。
 * 支持: 连续文件传输、CRC32/CRC16校验、1024字节数据帧、断点续传(ZRPOS)、ZDLE转义
 *
 * 协议帧格式:
 *   HEX帧: ZPAD ZDLE ZHEX <type:2hex> <data:8hex> <crc:4hex> CR LF
 *   BIN帧: ZPAD ZDLE ZBIN32 <type:1byte> <data:4bytes> <crc:4bytes> (ZDLE转义)
 *   数据子帧: <data:变长> ZDLE <endFlag> <crc32:4bytes> (ZDLE转义)
 *
 * 协作关系: BaseTransfer(框架), IConnection(通道), CRC(校验)
 */
#ifndef ZMODEMTRANSFER_H
#define ZMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

/**
 * @brief ZMODEM协议传输器 - PC端Sender
 *
 * 继承BaseTransfer，4个纯虚钩子: onStartInit/sendCancelBytes/processReceivedData/handleTimeout
 * 独立状态机: Idle->WaitingRinit->SendingFile->SendingData->WaitingZAck->SendingEof->SendingFin->Done
 */
class ZModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    explicit ZModemTransfer(QObject* parent = nullptr);
    /** @brief 设置要传输的文件路径 */
    void setFilePath(const QString& path);

protected:
    // === BaseTransfer 钩子实现 ===
    /** @brief 协议初始化: 加载文件、发送ZRQINIT握手帧 */
    bool onStartInit() override;
    /** @brief 发送取消帧: 8次Backspace + 2次CAN */
    void sendCancelBytes() override;
    /** @brief ZMODEM状态机: 解析HEX帧并执行状态转移 */
    void processReceivedData() override;
    /** @brief 超时处理: 根据当前状态重发对应帧 */
    void handleTimeout() override;

private:
    // ---- ZMODEM帧类型常量 ----
    static constexpr quint8 ZRQINIT = 0;     ///< 请求初始化
    static constexpr quint8 ZRINIT  = 1;     ///< 接收方初始化能力
    static constexpr quint8 ZSINIT  = 2;     ///< 发送方初始化参数
    static constexpr quint8 ZACK    = 3;     ///< 确认应答
    static constexpr quint8 ZFILE   = 4;     ///< 文件信息帧
    static constexpr quint8 ZSKIP   = 5;     ///< 跳过当前文件
    static constexpr quint8 ZNAK    = 6;     ///< 否定确认
    static constexpr quint8 ZABORT  = 7;     ///< 中止传输
    static constexpr quint8 ZFIN    = 8;     ///< 结束会话
    static constexpr quint8 ZRPOS   = 9;     ///< 重传位置(断点续传)
    static constexpr quint8 ZDATA   = 10;    ///< 数据帧(携带偏移)
    static constexpr quint8 ZEOF    = 11;    ///< 文件传输结束
    static constexpr quint8 ZFERR   = 12;    ///< 文件读写错误
    static constexpr quint8 ZCRC    = 13;    ///< 请求文件CRC
    static constexpr quint8 ZCHALLENGE = 14; ///< 安全挑战
    static constexpr quint8 ZCOMPL  = 15;    ///< 命令完成

    // ---- 帧头标志字符 ----
    static constexpr char ZPAD   = '*';      ///< 0x2A 帧填充
    static constexpr char ZDLE   = 0x18;     ///< 0x18 转义前缀
    static constexpr char ZBIN   = 'A';      ///< BIN帧(16bit CRC)
    static constexpr char ZBIN32 = 'B';      ///< BIN帧(32bit CRC)
    static constexpr char ZHEX   = 'C';      ///< HEX帧

    // ---- 数据子帧结束标记 ----
    static constexpr char ZCRCE  = 'h';      ///< CRC后跟下一帧
    static constexpr char ZCRCG  = 'i';      ///< CRC继续发送
    static constexpr char ZCRCQ  = 'j';      ///< CRC + 请求ZACK
    static constexpr char ZCRCW  = 'k';      ///< CRC + 等待ZACK

    static constexpr int kDataLen = 1024;    ///< 数据子帧最大长度

    /** @brief ZMODEM内部协议状态 */
    enum class State {
        Idle, WaitingRinit, SendingFile, SendingData,
        WaitingZAck, SendingEof, SendingFin, Done, Error
    };

    void setState(State s);

    // ---- 帧解析与构建 ----
    /** @brief 解析HEX帧(含CRC16校验) */
    bool parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData);
    /** @brief 构建HEX帧头(CRC16校验) */
    QByteArray buildHexHeader(quint8 frameType, const QByteArray& data = QByteArray());
    /** @brief 构建BIN32帧头(CRC32 + ZDLE转义) */
    QByteArray buildBinHeader(quint8 frameType, const QByteArray& data = QByteArray());
    /** @brief 构建数据子帧(ZDLE转义 + CRC32) */
    QByteArray buildDataSubpacket(char endFlag, const QByteArray& data);

    // ---- 发送流程 ----
    void sendZRQINIT();  ///< 发送初始化请求
    void sendZFILE();    ///< 发送文件信息
    void sendZDATA();    ///< 发送数据帧头(含偏移)
    void sendDataSubpackets(); ///< 批量发送数据子帧
    void sendZEOF();     ///< 发送文件结束
    void sendZFIN();     ///< 发送会话结束

    // ---- 工具方法 ----
    quint32 encodeCrc32(quint32 crc);    ///< CRC32编码(预留)
    QByteArray toHex(quint32 val, int digits); ///< 数值转HEX字符串

    // ---- 成员变量 ----
    QString m_filePath;                ///< 待传输文件路径
    QByteArray m_fileData;             ///< 文件全部内容
    State m_zmodemState = State::Idle; ///< 协议内部状态
    qint64 m_bytesSent = 0;            ///< 已发送字节数
    qint64 m_fileOffset = 0;           ///< 当前文件偏移
    quint32 m_senderCrc32 = 0;         ///< CRC32累积值
};

#endif // ZMODEMTRANSFER_H
