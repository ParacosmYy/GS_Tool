/**
 * @file ZModemTransfer.h
 * @brief ZMODEM协议传输器 - PC端Sender实现
 *
 * HEX帧: ZPAD ZDLE ZHEX <type:2hex> <data:8hex> <crc:4hex> CR LF
 * BIN帧: ZPAD ZDLE ZBIN32 <type:1byte> <data:4bytes> <crc:4bytes>
 * 状态流: Idle→WaitingRinit→SendingFile→SendingData→WaitingZAck→SendingEof→SendingFin→Done
 * 协作: BaseTransfer(框架), IConnection(通道), CRC(校验)
 */
#ifndef ZMODEMTRANSFER_H
#define ZMODEMTRANSFER_H

#include "ota/protocols/base/BaseTransfer.h"
#include "utils/crypto/CRC.h"

/** @brief ZMODEM协议传输器 - PC端Sender，实现4个纯虚钩子 */
class ZModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    // ── 统计 ──
    /** @brief ZMODEM传输统计结构体 */
    struct Stats {
        quint64 blocksSent = 0;        ///< 已发送数据块总数
        quint64 retries = 0;           ///< 重传总次数
        quint64 crcErrors = 0;         ///< CRC校验错误总次数
        quint64 errors = 0;            ///< 协议错误总次数
        quint64 timeouts = 0;          ///< 累计超时事件次数
        quint64 zrposReceived = 0;     ///< 累计接收ZRPOS次数
        quint64 zdataFrames = 0;       ///< 累计发送ZDATA帧次数
        quint64 zfileSent = 0;         ///< 累计发送ZFILE帧次数
        quint64 zfinSent = 0;          ///< 累计发送ZFIN帧次数
    };

    explicit ZModemTransfer(QObject* parent = nullptr); ///< 构造 @param parent 父对象
    void setFilePath(const QString& path); ///< 设置文件路径(绝对路径,start()前调用)

    /** @brief 获取统计数据的只读引用 @return Stats常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置ZModem统计计数器 */
    void resetStats() { m_stats = Stats{}; }

    // ── 向后兼容的便捷 Getter ──
    quint64 totalBlocksSent() const { return m_stats.blocksSent; }      ///< 获取已发送数据块总数
    quint64 totalRetries() const { return m_stats.retries; }            ///< 获取重传总次数
    quint64 totalCrcErrors() const { return m_stats.crcErrors; }        ///< 获取CRC校验错误总次数
    quint64 zmodemErrorCount() const { return m_stats.errors; }         ///< 获取协议错误总次数
    quint64 totalTimeouts() const { return m_stats.timeouts; }          ///< 获取累计超时事件次数
    quint64 totalZrposReceived() const { return m_stats.zrposReceived; } ///< 获取累计ZRPOS次数
    quint64 totalZdataFrames() const { return m_stats.zdataFrames; }    ///< 获取累计ZDATA帧次数
    quint64 totalZfileSent() const { return m_stats.zfileSent; }        ///< 获取累计ZFILE帧次数
    quint64 totalZfinSent() const { return m_stats.zfinSent; }          ///< 获取累计ZFIN帧次数
    void resetZmodemStatistics() { resetStats(); }                       ///< 向后兼容别名

protected:
    bool onStartInit() override;           ///< 加载文件+校验+发送ZRQINIT
    void sendCancelBytes() override;       ///< 发送取消序列: 8x0x08 + 2xCAN
    void processReceivedData() override;   ///< 状态机: 解析HEX帧并执行状态转移
    void handleTimeout() override;         ///< 超时: 根据当前状态重发对应帧

private:
    // ---- 帧类型常量 ----
    static constexpr quint8 ZRQINIT = 0;   ///< 请求初始化(Sender→Receiver)
    static constexpr quint8 ZRINIT  = 1;   ///< 接收方能力声明
    static constexpr quint8 ZACK    = 3;   ///< 确认应答
    static constexpr quint8 ZFILE   = 4;   ///< 文件信息帧
    static constexpr quint8 ZSKIP   = 5;   ///< 跳过文件
    static constexpr quint8 ZFIN    = 8;   ///< 结束会话
    static constexpr quint8 ZRPOS   = 9;   ///< 重传位置(断点续传)
    static constexpr quint8 ZDATA   = 10;  ///< 数据帧
    static constexpr quint8 ZEOF    = 11;  ///< 文件结束
    static constexpr quint8 ZCRC    = 13;  ///< CRC请求
    // ---- 帧头标志 ----
    static constexpr char ZPAD = '*', ZDLE = 0x18, ZBIN = 'A', ZBIN32 = 'B', ZHEX = 'C';
    // ---- 数据子帧结束标记 ----
    static constexpr char ZCRCE = 'h', ZCRCG = 'i', ZCRCQ = 'j', ZCRCW = 'k';
    static constexpr int kDataLen = 1024;  ///< 数据子帧最大载荷
    /** @brief ZMODEM内部状态 */
    enum class State { Idle, WaitingRinit, SendingFile, SendingData, WaitingZAck, SendingEof, SendingFin, Done, Error };
    void setState(State s);                ///< 设置协议状态
    static QString stateToString(State s); ///< 状态枚举转字符串(调试用)
    // ---- 帧解析与构建 ----
    /** @brief 解析HEX帧头部 @param data 原始数据 @param type 输出帧类型 @param headerData 输出帧头数据 @return true=解析成功 */
    bool parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData);
    QByteArray buildHexHeader(quint8 frameType, const QByteArray& data = QByteArray()); ///< 构建HEX帧头
    QByteArray buildBinHeader(quint8 frameType, const QByteArray& data = QByteArray()); ///< 构建BIN帧头
    /** @brief 构建数据子帧 @param endFlag 结束标记 @param data 载荷数据 @return 完整数据子帧 */
    QByteArray buildDataSubpacket(char endFlag, const QByteArray& data);
    QByteArray escapeZdle(const QByteArray& data) const; ///< ZDLE转义
    // ---- 发送流程 ----
    void sendZRQINIT();   ///< 发送初始化请求
    void sendZFILE();     ///< 发送文件信息
    void sendZDATA();     ///< 发送数据帧头
    void sendDataSubpackets(); ///< 批量发送数据子帧
    void sendZEOF();      ///< 发送文件结束
    void sendZFIN();      ///< 发送会话结束
    QByteArray toHex(quint32 val, int digits); ///< 数值转HEX ASCII字符串
    /** @brief 安全写入数据到连接 @param data 待写入数据 @return true=成功, false=连接断开 */
    bool writeChecked(const QByteArray& data);
    // ---- 状态处理方法 ----
    void handleStateWaitingRinit(int type); ///< WaitingRinit: 收到ZRINIT后发送ZFILE
    void handleStateSendingFile(int type, const QByteArray& headerData); ///< SendingFile: ZRPOS/ZSKIP/ZRINIT
    void handleStateSendingData(int type, const QByteArray& headerData); ///< SendingData: ZRPOS/ZACK
    void handleStateWaitingZAck(int type);  ///< WaitingZAck: ZACK/ZRPOS后发送ZEOF
    void handleStateSendingEof(int type);   ///< SendingEof: ZRINIT/ZSKIP后发送ZFIN
    void handleStateSendingFin(int type);   ///< SendingFin: 收到ZFIN后完成传输
    // ---- 成员 ----
    QString m_filePath;                ///< 文件路径
    QByteArray m_fileData;             ///< 文件内容(一次性读入)
    State m_zmodemState = State::Idle; ///< 协议状态
    qint64 m_bytesSent = 0;            ///< 已发送字节
    qint64 m_fileOffset = 0;           ///< 当前偏移(断点续传)
    quint32 m_senderCrc32 = 0;         ///< CRC32累积值

    Stats m_stats;                    ///< ZMODEM统计实例
};

#endif // ZMODEMTRANSFER_H
