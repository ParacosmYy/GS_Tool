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

#include "ota/protocols/base/BaseTransfer.h"
#include "utils/crypto/CRC.h"

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

    /** @brief 设置协议状态 @param s 目标状态 */
    void setState(State s);

    /** @brief 将状态枚举转换为字符串(调试用) @param s 状态枚举 @return 状态名称字符串 */
    static QString stateToString(State s);

    // ---- 帧解析与构建 ----

    /** @brief 解析HEX帧头部 @param data 原始数据 @param type 输出: 帧类型 @param headerData 输出: 帧头数据 @return true=解析成功 */
    bool parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData);

    /** @brief 构建HEX帧头 @param frameType 帧类型 @param data 帧数据 @return HEX编码的帧字节数组 */
    QByteArray buildHexHeader(quint8 frameType, const QByteArray& data = QByteArray());

    /** @brief 构建BIN帧头 @param frameType 帧类型 @param data 帧数据 @return BIN编码的帧字节数组 */
    QByteArray buildBinHeader(quint8 frameType, const QByteArray& data = QByteArray());

    /** @brief 构建数据子帧 @param endFlag 结束标记(ZCRCE/ZCRCG/ZCRCQ/ZCRCW) @param data 载荷数据 @return 完整的数据子帧字节数组 */
    QByteArray buildDataSubpacket(char endFlag, const QByteArray& data);

    /** @brief ZDLE转义: 对data中的控制字符进行转义 @param data 原始数据 @return 转义后的字节数组 */
    QByteArray escapeZdle(const QByteArray& data) const;

    // ---- 发送流程 ----

    /** @brief 发送初始化请求(ZRQINIT) */
    void sendZRQINIT();

    /** @brief 发送文件信息(ZFILE) */
    void sendZFILE();

    /** @brief 发送数据帧头(ZDATA) */
    void sendZDATA();

    /** @brief 批量发送数据子帧 */
    void sendDataSubpackets();

    /** @brief 发送文件结束(ZEOF) */
    void sendZEOF();

    /** @brief 发送会话结束(ZFIN) */
    void sendZFIN();

    /** @brief 将数值转换为HEX ASCII字符串 @param val 数值 @param digits 位数 @return HEX字符串 */
    QByteArray toHex(quint32 val, int digits);

    /**
     * @brief 安全写入数据到连接，检测连接断开
     * @param data 待写入数据
     * @return true=写入成功, false=连接断开(已触发Error状态)
     */
    bool writeChecked(const QByteArray& data);

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

    // ---- 统计计数器 ----
    quint64 m_totalBlocksSent = 0;     ///< 已发送数据块总数
    quint64 m_totalRetries = 0;        ///< 重传总次数
    quint64 m_totalCrcErrors = 0;      ///< CRC校验错误总次数
    quint64 m_errorCount = 0;          ///< 协议错误总次数
    quint64 m_totalTimeouts = 0;       ///< 累计超时事件次数

public:
    /** @brief 获取已发送数据块总数 @return 数据块计数 */
    quint64 totalBlocksSent() const { return m_totalBlocksSent; }
    /** @brief 获取重传总次数 @return 重传计数 */
    quint64 totalRetries() const { return m_totalRetries; }
    /** @brief 获取CRC校验错误总次数 @return CRC错误计数 */
    quint64 totalCrcErrors() const { return m_totalCrcErrors; }
    /** @brief 获取协议错误总次数 @return 错误计数 */
    quint64 errorCount() const { return m_errorCount; }
    /** @brief 获取累计超时事件次数 @return 超时计数 */
    quint64 totalTimeouts() const { return m_totalTimeouts; }
    /** @brief 重置ZModem统计计数器(不影响传输状态) */
    void resetZmodemStatistics();
};

#endif // ZMODEMTRANSFER_H
