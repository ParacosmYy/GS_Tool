/**
 * @file YModemTransfer.h
 * @brief YMODEM协议传输器 - PC端Sender实现
 *
 * 基于XMODEM-CRC，增加Block 0文件信息和批量传输。
 * 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑。
 *
 * 增强特性:
 *   - Block 0文件信息帧(文件名+大小+修改日期)
 *   - 批量传输支持(多个文件连续传输)
 *   - 完善接收方应答处理(ACK+NAK+CAN+CRC)
 *   - 传输速率计算和ETA
 *
 * 协作关系:
 *   - BaseTransfer: 提供传输框架、超时重试、连接管理
 *   - IConnection: 数据收发通道
 *   - CRC: 提供CRC16-CCITT计算
 */
#ifndef YMODEMTRANSFER_H
#define YMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

#include <QElapsedTimer>

class YModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    explicit YModemTransfer(QObject* parent = nullptr);

    /** @brief 设置单个文件路径 */
    void setFilePath(const QString& path);

    /** @brief 设置多个文件路径(批量传输) */
    void setFilePaths(const QStringList& paths);

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
     * @param etaSec 预计剩余时间(秒)
     * @param fileName 当前传输的文件名
     */
    void transferStats(double rateBytesPerSec, double etaSec,
                       const QString& fileName);

    /**
     * @brief 单个文件传输完成信号(批量传输时使用)
     * @param fileName 完成的文件名
     * @param index 文件在列表中的索引
     */
    void fileTransferComplete(const QString& fileName, int index);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout() override;

private:
    // ---- YMODEM协议控制字节 ----
    static constexpr char SOH = 0x01;
    static constexpr char EOT = 0x04;
    static constexpr char ACK = 0x06;
    static constexpr char NAK = 0x15;
    static constexpr char CAN = 0x18;
    static constexpr char CRC_CHAR = 'C';

    static constexpr int kBlockSize = 128;  ///< YMODEM固定128字节块

    /** @brief YMODEM内部状态 */
    enum class State {
        Idle,
        WaitingStart,      ///< 等待接收方发送C或NAK
        SendingBlock0,     ///< 发送Block 0文件信息等待ACK
        SendingData,       ///< 发送数据块等待ACK
        SendingEOT,        ///< 发送EOT等待ACK
        WaitBlock0Ack,     ///< 等待下一个文件的C/NAK触发
        WaitFinalC,        ///< 等待最终结束的C/NAK
        SendingFinalBlock0,///< 发送空Block 0结束会话
        Done,
        Error
    };

    // ---- 状态管理 ----
    void setState(State s);

    // ---- 数据发送 ----
    void sendBlock0();
    void sendBlock();
    void sendEOT();
    void sendFinalBlock0();

    /**
     * @brief 构建YMODEM数据块(含头部、序号、CRC16)
     * @param blockNum 块序号(Block 0 = 0, 数据块从1开始)
     * @param blockData 块数据(128字节)
     * @return 完整数据包(SOH+序号+数据+CRC16)
     */
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);

    /**
     * @brief 构建Block 0文件信息帧
     * @param fileName 文件名(不含路径)
     * @param fileSize 文件大小(字节)
     * @param modTime 文件修改时间(Unix时间戳)
     * @return 128字节的Block 0数据
     */
    QByteArray buildBlock0(const QString& fileName, qint64 fileSize,
                           qint64 modTime);

    // ---- 速率统计 ----
    void updateTransferStats();

    // ---- 文件管理 ----
    bool loadNextFile();

    // ---- 成员变量 ----
    QStringList m_filePaths;        ///< 待传输文件列表
    QByteArray m_currentData;       ///< 当前文件数据
    QString m_currentFileName;      ///< 当前文件名(不含路径)

    State m_ymodemState = State::Idle; ///< 协议内部状态
    int m_blockNumber = 0;            ///< 当前块序号
    qint64 m_bytesSent = 0;           ///< 当前文件已发送字节数
    qint64 m_totalBytes = 0;          ///< 所有文件总字节数
    qint64 m_totalBytesSent = 0;      ///< 已发送的总字节数
    int m_fileIndex = 0;              ///< 当前传输的文件索引
    int m_blockRetryCount = 0;        ///< 当前块重试次数

    // ---- 速率计算 ----
    QElapsedTimer m_transferTimer;    ///< 传输耗时计时器
    double m_currentRate = 0.0;       ///< 当前传输速率(字节/秒)
};

#endif // YMODEMTRANSFER_H
