/**
 * @file XModemTransfer.h
 * @brief XMODEM协议传输器 - PC端Sender实现。支持Checksum/CRC/1K三种模式，继承BaseTransfer通过4个纯虚钩子注入协议逻辑
 */
#ifndef XMODEMTRANSFER_H
#define XMODEMTRANSFER_H

#include "ota/protocols/base/BaseTransfer.h"
#include "utils/crypto/CRC.h"
#include <QElapsedTimer>

/**
 * @brief XMODEM协议传输器 -- PC端Sender实现，支持Checksum/CRC/1K三种模式
 *
 * 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑。
 * 接收方发送NAK时自动回退到Checksum模式，每块最多重试10次。
 * 协作: BaseTransfer(传输框架) / IConnection(收发通道) / CRC(校验)
 * 设计: 模板方法(BaseTransfer骨架) + 策略模式(三种XMODEM模式可互换)
 */
class XModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    /** @brief XMODEM传输模式 */
    enum Mode { Checksum, CRC, OneK }; ///< Checksum=Sum8, CRC=CRC16, OneK=1024B+CRC16

    /** @brief 构造XModemTransfer，默认CRC模式 @param parent 父对象 */
    explicit XModemTransfer(QObject* parent = nullptr);
    /** @brief 设置传输模式(须在start()前调用，NAK自动回退Checksum) @param mode 传输模式 */
    void setMode(Mode mode);
    /** @brief 设置文件路径(限制16MB，setData优先) @param path 固件文件路径 */
    void setFilePath(const QString& path);
    /** @brief 直接设置传输数据(清除filePath) @param data 待传输数据 */
    void setData(const QByteArray& data);
    /** @brief 获取当前传输速率 @return 速率(字节/秒)，未开始返回0 */
    double transferRate() const;
    /** @brief 获取预计剩余时间 @return 秒数，无法估算返回-1 */
    double etaSeconds() const;
    // ---- 统计接口 ----

    /** @brief 获取累计发送块总数 @return 数据块计数 */
    quint64 totalBlocksSent() const;

    /** @brief 获取累计重试次数 @return 重试计数 */
    quint64 totalRetries() const;

    /** @brief 获取累计模式降级次数 @return 降级计数 */
    quint64 totalModeSwitches() const;

    /** @brief 获取累计错误次数 @return 错误计数 */
    quint64 xmodemErrorCount() const;

    /** @brief 获取累计CRC校验被拒次数(NAK触发) @return CRC错误计数 */
    quint64 totalCrcErrors() const { return m_totalCrcErrors; }

    /** @brief 获取累计超时事件次数 @return 超时计数 */
    quint64 totalTimeouts() const { return m_totalTimeouts; }

    /** @brief 获取累计接收NAK次数 @return NAK计数 */
    quint64 totalNakReceived() const { return m_totalNakReceived; }

    /** @brief 获取累计接收CAN次数 @return CAN计数 */
    quint64 totalCanReceived() const { return m_totalCanReceived; }

    /** @brief 获取累计接收ACK次数 @return ACK计数 */
    quint64 totalAcksReceived() const { return m_totalAcksReceived; }

    /** @brief 重置XModem统计计数器 */
    void resetXmodemStatistics();

signals:
    /** @brief 传输速率和ETA更新 @param rateBytesPerSec 当前速率(字节/秒) @param etaSec 预计剩余时间(秒) */
    void transferStats(double rateBytesPerSec, double etaSec);

    /** @brief 协议模式自动降级通知 @param fromMode 降级前模式名 @param toMode 降级后模式名 */
    void modeDegraded(const QString& fromMode, const QString& toMode);

protected:
    // === BaseTransfer 钩子实现 ===
    /** @brief 协议初始化: 加载文件→校验→重置计数器→等待接收方启动信号。文件>16MB/不可读/数据空拒绝 */
    bool onStartInit() override;
    /** @brief 发送2个CAN字节取消传输 */
    void sendCancelBytes() override;
    /** @brief 接收数据状态机: WaitingForStart→SendingBlock→SendingEOT→Done。每块最多重试10次 */
    void processReceivedData() override;
    /** @brief 超时重发: SendingBlock重发当前块, SendingEOT重发EOT, WaitingForStart延长等待(3倍超时) */
    void handleTimeout() override;

private:
    static constexpr char STX = 0x02; ///< 1024字节块头(XMODEM-1K专用)
    /** @brief XMODEM内部状态(独立于BaseTransfer::TransferState) */
    enum class State { Idle, WaitingForStart, SendingBlock, SendingEOT, Done, Error };
    void setState(State newState);     ///< 设置XMODEM内部状态
    void sendBlock();                  ///< 发送当前数据块(不足0x1A填充，m_conn为空不写入)
    void sendEOT();                    ///< 发送EOT字节，等待ACK
    QByteArray buildBlock(int blockNum, const QByteArray& blockData); ///< 构建数据包: 头+序号+反码+数据+校验
    quint16 xmodemCrc(const QByteArray& data); ///< CRC16，委托CRC::crc16Xmodem
    void updateTransferStats();        ///< 更新平均速率和ETA，发射transferStats信号
    int blockSize() const { return (m_mode == OneK) ? 1024 : 128; } ///< 当前模式块大小
    // ---- 状态处理(processReceivedData状态分发) ----
    void handleStateWaitingForStart(char ch);  ///< 收到NAK/C后启动数据发送
    void handleStateSendingBlock(char ch, int& readIdx);  ///< 收到ACK/NAK/CAN后更新进度或重发
    void handleStateSendingEOT(char ch, int& readIdx);    ///< 收到ACK/NAK/CAN后完成或重发
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
    // ---- 统计计数器 ----
    quint64 m_totalBlocksSent = 0;       ///< 累计发送块总数
    quint64 m_totalRetries = 0;          ///< 累计重试次数
    quint64 m_totalModeSwitches = 0;     ///< 累计模式降级次数
    quint64 m_xmodemErrorCount = 0;      ///< 累计错误次数
    quint64 m_totalCrcErrors = 0;        ///< 累计CRC校验被拒次数(接收方NAK触发)
    quint64 m_totalTimeouts = 0;         ///< 累计超时事件次数
    quint64 m_totalNakReceived = 0;      ///< 累计接收NAK次数
    quint64 m_totalCanReceived = 0;      ///< 累计接收CAN次数
    quint64 m_totalAcksReceived = 0;     ///< 累计接收ACK次数
};

#endif // XMODEMTRANSFER_H
