/**
 * @file YModemTransfer.h
 * @brief YMODEM协议传输器 - PC端Sender实现
 *
 * 基于XMODEM-CRC，增加Block 0文件信息和批量传输。
 * 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑。
 *
 * == Block 0 文件信息帧格式 (128字节) ==
 * 偏移0: 文件名(ASCII, null-terminated)
 * 偏移N: 文件大小(ASCII十进制, null-terminated, 如"65536")
 * 偏移M: 修改时间(ASCII八进制Unix时间戳, null-terminated)
 * 偏移K: 权限(ASCII, 如"100644", null-terminated)
 * 剩余:  0x00填充至128字节。超过128字节时截断。
 *
 * == 状态流转 ==
 * WaitingStart → SendingBlock0 → SendingData → SendingEOT
 *   → [批量: WaitBlock0Ack → SendingBlock0] → WaitFinalC
 *   → SendingFinalBlock0(空Block0) → Done
 *
 * == 边界情况 ==
 * 1. 文件>1MB: onStartInit()拒绝  2. 空文件/空列表: onStartInit()拒绝
 * 3. Block0超128B: buildBlock0()截断  4. 数据不足128B: 0x1A填充
 * 5. 块序号: Block0=0, 数据块从1递增   6. 接收方CAN: 立即中止
 * 7. 重试上限: 每阶段独立计数, 超10次发CAN取消
 *
 * 协作: BaseTransfer(框架), IConnection(通道), CRC(校验), OtaManager(业务层)
 */
#ifndef YMODEMTRANSFER_H
#define YMODEMTRANSFER_H

#include <QElapsedTimer>

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

/**
 * @brief YMODEM协议传输器 - PC端Sender
 *
 * 独立状态机管理协议详细状态，与BaseTransfer的TransferState
 * (Idle/Active/Done/Error)双状态机协同。
 * 速率通过QElapsedTimer累计发送量计算平均值，每次ACK后更新。
 */
class YModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    /** @brief 构造YMODEM传输器。默认超时5s, 最大重试10次。
     *  @param parent 父对象(Qt对象树管理生命周期)
     *  构造后调用setFilePath()→start()开始传输 */
    explicit YModemTransfer(QObject* parent = nullptr);

    /** @brief 设置单个文件路径，替换内部列表为单元素。须在start()前调用 */
    void setFilePath(const QString& path);

    /** @brief 设置多个文件路径(批量传输模式)。每个文件有独立Block0。须在start()前调用 */
    void setFilePaths(const QStringList& paths);

    /** @brief 当前传输速率(字节/秒)。未开始时返回0。基于累计发送量/耗时计算 */
    double transferRate() const;

    /** @brief 预计剩余时间(秒)。速率<=0或总量<=0时返回-1。ETA=剩余字节/当前速率 */
    double etaSeconds() const;

signals:
    /** @brief 速率和ETA更新。每次收到ACK后发射。
     *  @param rateBytesPerSec 速率 @param etaSec ETA(-1=无法估算)
     *  @param fileName 当前文件名(不含路径)。OtaManager转发时去除此参数 */
    void transferStats(double rateBytesPerSec, double etaSec,
                       const QString& fileName);

    /** @brief 单文件传输完成(批量模式)。EOT被ACK后发射。
     *  @param fileName 文件名 @param index 在m_filePaths中的索引(从0起) */
    void fileTransferComplete(const QString& fileName, int index);

protected:
    // === BaseTransfer 钩子实现 ===

    /** @brief 初始化: 校验文件(非空/<=1MB/可读)→计算总量→加载首文件→等C/NAK
     *  @return true=进入WaitingStart, false=校验失败。超时设为3倍(15s) */
    bool onStartInit() override;

    /** @brief 发送2个CAN(0x18)取消。m_conn为空时空操作 */
    void sendCancelBytes() override;

    /** @brief 接收状态机核心。逐字节解析ACK/NAK/CAN/C，驱动状态转移 */
    void processReceivedData() override;

    /** @brief 超时处理。WaitingStart等仅重启定时器; Sending*状态重发+递增重试。
     *  重试超10次发CAN取消 */
    void handleTimeout() override;

private:
    // ---- YMODEM协议控制字节 ----
    static constexpr char SOH = 0x01;       ///< 128字节块起始标记
    static constexpr char EOT = 0x04;       ///< 传输结束标记
    static constexpr char ACK = 0x06;       ///< 确认应答
    static constexpr char NAK = 0x15;       ///< 否定确认(Checksum回退)
    static constexpr char CAN = 0x18;       ///< 取消传输
    static constexpr char CRC_CHAR = 'C';   ///< CRC模式请求
    static constexpr int kBlockSize = 128;  ///< YMODEM固定128字节块

    /** @brief YMODEM内部协议状态 */
    enum class State {
        Idle,               ///< 空闲
        WaitingStart,      ///< 等待接收方C或NAK
        SendingBlock0,     ///< 已发Block0文件信息, 等ACK/C
        SendingData,       ///< 发送数据块中, 等ACK
        SendingEOT,        ///< 已发EOT, 等ACK
        WaitBlock0Ack,     ///< 批量: 等下一文件C/NAK
        WaitFinalC,        ///< 全部完成: 等最终C发空Block0
        SendingFinalBlock0,///< 已发空Block0结束会话, 等ACK
        Done, Error
    };

    void sendBlock0();                      ///< 发送Block0文件信息帧
    void sendBlock();                       ///< 发送当前数据块(128B)
    void sendEOT();                         ///< 发送EOT字节
    void sendFinalBlock0();                 ///< 发送空Block0结束会话

    /** @brief 构建数据包: SOH + blockNum + ~blockNum + data(128B) + CRC16(2B)
     *  @param blockNum 块序号(Block0=0, 数据块从1起)
     *  @param blockData 128字节数据载荷 */
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);

    /** @brief 构建Block0数据载荷(128B)
     *  @param fileName 文件名(不含路径) @param fileSize 文件大小(字节)
     *  @param modTime 修改时间(Unix时间戳)
     *  @return 128字节载荷: fileName\0size\0modTime\0"100644"\0+0x00填充 */
    QByteArray buildBlock0(const QString& fileName, qint64 fileSize,
                           qint64 modTime);

    void updateTransferStats();             ///< 更新速率/ETA并发射transferStats
    bool loadNextFile();                    ///< 加载下个文件。失败时emit transferError

    // ---- 状态处理方法(processReceivedData状态分发) ----
    /** @brief 处理WaitingStart状态: 收到C/NAK后发送Block0 */
    void handleStateWaitingStart(char ch);

    /** @brief 处理SendingBlock0状态: 收到ACK/C/NAK/CAN后转移状态 */
    void handleStateSendingBlock0(char ch, int& readIdx);

    /** @brief 处理SendingData状态: 收到ACK/NAK/CAN后更新进度或重发 */
    void handleStateSendingData(char ch, int& readIdx);

    /** @brief 处理SendingEOT状态: 收到ACK/NAK/CAN后进入下一文件或结束 */
    void handleStateSendingEOT(char ch, int& readIdx);

    /** @brief 处理WaitBlock0Ack状态: 收到C/NAK后加载下一文件 */
    void handleStateWaitBlock0Ack(char ch, int& readIdx);

    /** @brief 处理WaitFinalC状态: 收到C/NAK后发送空Block0 */
    void handleStateWaitFinalC(char ch);

    /** @brief 处理SendingFinalBlock0状态: 收到ACK/NAK/CAN后完成或重发 */
    void handleStateSendingFinalBlock0(char ch, int& readIdx);

    // ---- 成员变量 ----
    QStringList m_filePaths;        ///< 待传输文件路径列表
    QByteArray m_currentData;       ///< 当前文件全部内容
    QString m_currentFileName;      ///< 当前文件名(不含路径)
    State m_ymodemState = State::Idle; ///< 协议内部状态
    int m_blockNumber = 0;            ///< 当前块序号(Block0=0, 数据块从1起)
    qint64 m_bytesSent = 0;           ///< 当前文件已发送字节数
    qint64 m_totalBytes = 0;          ///< 所有文件总字节数
    qint64 m_totalBytesSent = 0;      ///< 已发送总字节数
    int m_fileIndex = 0;              ///< 当前文件索引
    int m_blockRetryCount = 0;        ///< 当前阶段重试次数(上限10)
    QElapsedTimer m_transferTimer;    ///< 传输计时器
    double m_currentRate = 0.0;       ///< 当前速率(字节/秒)
};

#endif // YMODEMTRANSFER_H
