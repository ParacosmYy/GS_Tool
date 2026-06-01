/**
 * @file XModemTransfer.h
 * @brief XMODEM协议传输器 - PC端Sender实现
 *
 * 支持三种模式: Checksum(Sum8), CRC16, 1K(1024字节块+CRC16)。
 * 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑。
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

    /** @brief 构造XMODEM传输器，默认CRC模式 */
    explicit XModemTransfer(QObject* parent = nullptr);

    /** @brief 设置传输模式，须在start()前调用。接收方NAK会自动回退Checksum */
    void setMode(Mode mode);

    /** @brief 设置文件路径，须在start()前调用。文件限制kMaxFileSize(16MB)，setData优先 */
    void setFilePath(const QString& path);

    /** @brief 直接设置传输数据(清除filePath)，适用于内存中已有数据场景 */
    void setData(const QByteArray& data);

    /** @brief 获取当前传输速率(字节/秒)，未开始时返回0 */
    double transferRate() const;

    /** @brief 获取ETA(秒)，无法估算时返回-1 */
    double etaSeconds() const;

signals:
    /** @brief 传输速率和ETA更新信号 @param rateBytesPerSec 速率 @param etaSec ETA秒数，-1无法估算 */
    void transferStats(double rateBytesPerSec, double etaSec);

    /** @brief 协议模式自动降级通知 @param fromMode 降级前模式名 @param toMode 降级后模式名 */
    void modeDegraded(const QString& fromMode, const QString& toMode);

protected:
    // === BaseTransfer 钩子实现 ===

    /**
     * @brief 协议初始化: 加载文件→校验→重置计数器→等待接收方启动信号
     * @return true=成功进入WaitingForStart, false=文件加载失败
     *
     * 边界: 文件>kMaxFileSize(16MB)拒绝, 文件不可读拒绝, 数据为空拒绝。
     * 连接断开后不再收到数据，由超时机制兜底。
     */
    bool onStartInit() override;

    /** @brief 发送2个CAN字节取消传输。m_conn为空时空操作 */
    void sendCancelBytes() override;

    /**
     * @brief 接收数据状态机: WaitingForStart→SendingBlock→SendingEOT→Done
     *
     * 每块最多重试10次。块序号1-255循环(不用于文件偏移)。
     * m_bytesSent累计已发送字节数作为文件偏移。
     */
    void processReceivedData() override;

    /**
     * @brief 超时重发: SendingBlock重发当前块, SendingEOT重发EOT,
     *        WaitingForStart延长等待(3倍超时)
     */
    void handleTimeout() override;

private:
    static constexpr char STX = 0x02;      ///< 1024字节块头(XMODEM-1K专用)

    /** @brief XMODEM内部状态(独立于BaseTransfer::TransferState) */
    enum class State {
        Idle,
        WaitingForStart,  ///< 等待接收方发送NAK或'C'
        SendingBlock,     ///< 发送数据块等待ACK
        SendingEOT,       ///< 发送EOT等待ACK
        Done,
        Error
    };

    /** @brief 设置XMODEM内部状态，独立于基类TransferState */
    void setState(State newState);

    /** @brief 发送当前数据块。数据不足时0x1A填充，发完自动切EOT。m_conn为空不写入 */
    void sendBlock();

    /** @brief 发送EOT字节，等待ACK后由processReceivedData调finishTransfer */
    void sendEOT();

    /** @brief 构建XMODEM数据包: 头(SOH/STX) + 序号+反码 + 数据 + 校验(Sum8/CRC16) */
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);

    /** @brief 计算XMODEM CRC16，委托给CRC::crc16Xmodem */
    quint16 xmodemCrc(const QByteArray& data);

    /** @brief 更新平均速率和ETA，发射transferStats信号。elapsed<=0时跳过 */
    void updateTransferStats();

    /** @brief 获取当前模式块大小: OneK=1024, Checksum/CRC=128 */
    int blockSize() const { return (m_mode == OneK) ? 1024 : 128; }

    // ---- 状态处理方法(processReceivedData状态分发) ----
    /** @brief 处理WaitingForStart状态: 收到NAK/C后启动数据发送 */
    void handleStateWaitingForStart(char ch);

    /** @brief 处理SendingBlock状态: 收到ACK/NAK/CAN后更新进度或重发 */
    void handleStateSendingBlock(char ch, int& readIdx);

    /** @brief 处理SendingEOT状态: 收到ACK/NAK/CAN后完成或重发 */
    void handleStateSendingEOT(char ch, int& readIdx);

    // ---- 成员变量 ----
    Mode m_mode = CRC;               ///< 传输模式
    QString m_filePath;              ///< 文件路径
    QByteArray m_data;               ///< 待传输数据

    State m_xmodemState = State::Idle; ///< 协议内部状态
    int m_blockNumber = 1;           ///< 当前块序号(1-255循环)
    qint64 m_bytesSent = 0;          ///< 已发送字节数

    int m_blockRetryCount = 0;       ///< 当前块的重试次数(每块最多10次)

    // ---- 速率计算 ----
    QElapsedTimer m_transferTimer;   ///< 传输耗时计时器
    qint64 m_lastStatsBytes = 0;     ///< 上次统计时的已发送字节数
    double m_currentRate = 0.0;      ///< 当前传输速率(字节/秒)
};

#endif // XMODEMTRANSFER_H
