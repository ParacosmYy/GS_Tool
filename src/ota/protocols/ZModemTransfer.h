/**
 * @file ZModemTransfer.h
 * @brief ZMODEM协议传输器 - PC端Sender实现
 *
 * HEX帧: ZPAD ZDLE ZHEX <type:2hex> <data:8hex> <crc:4hex> CR LF
 * BIN帧: ZPAD ZDLE ZBIN32 <type:1byte> <data:4bytes> <crc:4bytes>
 * 数据子帧: <data:变长> ZDLE <endFlag> <crc32:4bytes>
 * 协作: BaseTransfer(框架), IConnection(通道), CRC(校验)
 */
#ifndef ZMODEMTRANSFER_H
#define ZMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

/**
 * @brief ZMODEM协议传输器 - PC端Sender
 *
 * 状态流: Idle→WaitingRinit→SendingFile→SendingData→WaitingZAck→SendingEof→SendingFin→Done
 * 实现4个纯虚钩子: onStartInit/sendCancelBytes/processReceivedData/handleTimeout
 */
class ZModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    explicit ZModemTransfer(QObject* parent = nullptr); ///< @param parent 父对象
    /** @brief 设置文件路径 @param path 绝对路径,start()前调用 */
    void setFilePath(const QString& path);

protected:
    /** @brief 加载文件+校验+发送ZRQINIT @return true=成功 */
    bool onStartInit() override;
    /** @brief 发送取消序列: 8×0x08 + 2×CAN(0x18) */
    void sendCancelBytes() override;
    /** @brief 状态机: 解析HEX帧并执行状态转移 */
    void processReceivedData() override;
    /** @brief 超时: 根据当前状态重发对应帧 */
    void handleTimeout() override;

private:
    // ---- 帧类型常量 ----
    static constexpr quint8 ZRQINIT = 0;     ///< 请求初始化(Sender→Receiver)
    static constexpr quint8 ZRINIT  = 1;     ///< 接收方能力声明
    static constexpr quint8 ZACK    = 3;     ///< 确认应答
    static constexpr quint8 ZFILE   = 4;     ///< 文件信息帧
    static constexpr quint8 ZSKIP   = 5;     ///< 跳过文件
    static constexpr quint8 ZFIN    = 8;     ///< 结束会话
    static constexpr quint8 ZRPOS   = 9;     ///< 重传位置(断点续传)
    static constexpr quint8 ZDATA   = 10;    ///< 数据帧
    static constexpr quint8 ZEOF    = 11;    ///< 文件结束
    static constexpr quint8 ZCRC    = 13;    ///< CRC请求

    // ---- 帧头标志 ----
    static constexpr char ZPAD   = '*';      ///< 帧填充起始
    static constexpr char ZDLE   = 0x18;     ///< 转义前缀
    static constexpr char ZBIN   = 'A';      ///< BIN帧(16bit CRC)
    static constexpr char ZBIN32 = 'B';      ///< BIN帧(32bit CRC)
    static constexpr char ZHEX   = 'C';      ///< HEX帧(ASCII)

    // ---- 数据子帧结束标记 ----
    static constexpr char ZCRCE  = 'h';      ///< CRC后跟下一帧
    static constexpr char ZCRCG  = 'i';      ///< CRC继续(中间帧)
    static constexpr char ZCRCQ  = 'j';      ///< CRC + 请求ZACK
    static constexpr char ZCRCW  = 'k';      ///< CRC + 等待ZACK(最后帧)

    static constexpr int kDataLen = 1024;    ///< 数据子帧最大载荷

    /** @brief ZMODEM内部状态 */
    enum class State {
        Idle, WaitingRinit, SendingFile, SendingData,
        WaitingZAck, SendingEof, SendingFin, Done, Error
    };

    void setState(State s);                            ///< 设置协议状态
    static QString stateToString(State s);             ///< 状态→字符串(调试用)

    // ---- 帧解析与构建 ----
    bool parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData);
    QByteArray buildHexHeader(quint8 frameType, const QByteArray& data = QByteArray());
    QByteArray buildBinHeader(quint8 frameType, const QByteArray& data = QByteArray());
    QByteArray buildDataSubpacket(char endFlag, const QByteArray& data);

    // ---- 发送流程 ----
    void sendZRQINIT();        ///< 发送初始化请求
    void sendZFILE();          ///< 发送文件信息
    void sendZDATA();          ///< 发送数据帧头
    void sendDataSubpackets(); ///< 批量发送数据子帧
    void sendZEOF();           ///< 发送文件结束
    void sendZFIN();           ///< 发送会话结束

    QByteArray toHex(quint32 val, int digits); ///< 数值→HEX ASCII

    // ---- 状态处理方法(processReceivedData状态分发) ----
    /** @brief 处理WaitingRinit状态: 收到ZRINIT后发送ZFILE */
    void handleStateWaitingRinit(int type);

    /** @brief 处理SendingFile状态: 收到ZRPOS/ZSKIP/ZRINIT后转移状态 */
    void handleStateSendingFile(int type, const QByteArray& headerData);

    /** @brief 处理SendingData状态: 收到ZRPOS/ZACK后重传或确认 */
    void handleStateSendingData(int type, const QByteArray& headerData);

    /** @brief 处理WaitingZAck状态: 收到ZACK/ZRPOS后发送ZEOF */
    void handleStateWaitingZAck(int type);

    /** @brief 处理SendingEof状态: 收到ZRINIT/ZSKIP后发送ZFIN */
    void handleStateSendingEof(int type);

    /** @brief 处理SendingFin状态: 收到ZFIN后完成传输 */
    void handleStateSendingFin(int type);

    // ---- 成员 ----
    QString m_filePath;                ///< 文件路径
    QByteArray m_fileData;             ///< 文件内容(一次性读入)
    State m_zmodemState = State::Idle; ///< 协议状态
    qint64 m_bytesSent = 0;            ///< 已发送字节
    qint64 m_fileOffset = 0;           ///< 当前偏移(断点续传)
    quint32 m_senderCrc32 = 0;         ///< CRC32累积值
};

#endif // ZMODEMTRANSFER_H
