/**
 * @file XModemTransfer.h
 * @brief XMODEM协议传输器 - PC端Sender实现
 *
 * 支持三种模式: Checksum(Sum8), CRC16, 1K(1024字节块+CRC16)。
 * 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑。
 *
 * 增强特性:
 *   - 传输速率计算和ETA(预计剩余时间)显示
 *   - 取消传输支持(发送CAN取消帧)
 *   - 超时重传机制(每块最多重试10次)
 *   - 完善错误处理和状态转换
 *
 * 协议交互流程(Sender端):
 *   1. 等待接收方发送NAK(Checksum模式)或'C'(CRC模式)
 *   2. 循环发送数据块，等待ACK/NAK/CAN
 *   3. 发送EOT，等待ACK
 *   4. 传输完成
 *
 * 协作关系:
 *   - BaseTransfer: 提供传输框架、超时重试、连接管理
 *   - IConnection: 数据收发通道(串口/TCP/UDP)
 *   - CRC: 提供CRC16-CCITT和算术校验和计算
 */
#ifndef XMODEMTRANSFER_H
#define XMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

#include <QElapsedTimer>

class XModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    /** @brief XMODEM传输模式 */
    enum Mode {
        Checksum, ///< XMODEM-Checksum: SOH + 128B + Sum8
        CRC,      ///< XMODEM-CRC: SOH + 128B + CRC16
        OneK      ///< XMODEM-1K: STX + 1024B + CRC16
    };

    explicit XModemTransfer(QObject* parent = nullptr);

    /** @brief 设置传输模式 */
    void setMode(Mode mode);

    /** @brief 设置要传输的文件路径 */
    void setFilePath(const QString& path);

    /** @brief 直接设置要传输的数据(无需文件) */
    void setData(const QByteArray& data);

    /**
     * @brief 获取当前传输速率(字节/秒)
     * @return 传输速率，未开始传输时返回0
     */
    double transferRate() const;

    /**
     * @brief 获取预计剩余时间(秒)
     * @return ETA秒数，无法估算时返回-1
     */
    double etaSeconds() const;

signals:
    /**
     * @brief 传输速率和ETA更新信号
     * @param rateBytesPerSec 当前传输速率(字节/秒)
     * @param etaSec 预计剩余时间(秒)，-1表示无法估算
     */
    void transferStats(double rateBytesPerSec, double etaSec);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout() override;

private:
    // ---- XMODEM协议控制字节 ----
    static constexpr char SOH = 0x01;      ///< 128字节块头
    static constexpr char STX = 0x02;      ///< 1024字节块头
    static constexpr char EOT = 0x04;      ///< 传输结束
    static constexpr char ACK = 0x06;      ///< 确认
    static constexpr char NAK = 0x15;      ///< 否定确认(Checksum模式)
    static constexpr char CAN = 0x18;      ///< 取消传输
    static constexpr char CRC_CHAR = 'C';  ///< CRC模式请求

    /** @brief XMODEM内部状态(独立于BaseTransfer的TransferState) */
    enum class State {
        Idle,
        WaitingForStart,  ///< 等待接收方发送NAK或'C'
        SendingBlock,     ///< 发送数据块等待ACK
        SendingEOT,       ///< 发送EOT等待ACK
        Done,
        Error
    };

    // ---- 状态管理 ----
    void setState(State newState);

    // ---- 数据发送 ----
    void sendBlock();
    void sendEOT();

    /**
     * @brief 构建XMODEM数据包
     * @param blockNum 块序号(1-255循环)
     * @param blockData 块数据(已填充至块大小)
     * @return 完整数据包(头+序号+数据+校验)
     */
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);

    /** @brief 计算XMODEM CRC16(等同于CRC::crc16Ccitt) */
    quint16 xmodemCrc(const QByteArray& data);

    // ---- 速率统计 ----
    void updateTransferStats();

    /** @brief 获取当前模式的块大小 */
    int blockSize() const {
        return (m_mode == OneK) ? 1024 : 128;
    }

    // ---- 成员变量 ----
    Mode m_mode = CRC;               ///< 传输模式
    QString m_filePath;              ///< 文件路径
    QByteArray m_data;               ///< 待传输数据

    State m_xmodemState = State::Idle; ///< 协议内部状态
    int m_blockNumber = 1;           ///< 当前块序号(1-255循环)
    qint64 m_bytesSent = 0;          ///< 已发送字节数

    int m_blockRetryCount = 0;       ///< 当前块的重试次数(每块最多10次)

    // ---- 速率计算相关 ----
    QElapsedTimer m_transferTimer;   ///< 传输耗时计时器
    qint64 m_lastStatsBytes = 0;     ///< 上次统计时的已发送字节数
    double m_currentRate = 0.0;      ///< 当前传输速率(字节/秒)
};

#endif // XMODEMTRANSFER_H
