/**
 * @file YModemTransfer.h
 * @brief YMODEM协议传输器 - PC端Sender。基于XMODEM-CRC+Block0文件信息+批量传输
 *
 * 状态: WaitingStart->SendingBlock0->SendingData->SendingEOT->Done
 * 协作: BaseTransfer/IConnection/CRC/OtaManager
 */
#ifndef YMODEMTRANSFER_H
#define YMODEMTRANSFER_H

#include <QElapsedTimer>

#include "ota/protocols/base/BaseTransfer.h"
#include "utils/crypto/CRC.h"

/** @brief YMODEM协议传输器 - PC端Sender。独立状态机管理协议详细状态，与BaseTransfer双状态机协同 */
class YModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    /** @brief 构造YModemTransfer，默认超时5s, 最大重试10次 @param parent 父对象 */
    explicit YModemTransfer(QObject* parent = nullptr);
    /** @brief 设置单个文件路径(须在start()前) @param path 固件文件路径 */
    void setFilePath(const QString& path);
    /** @brief 设置多文件路径(批量模式) @param paths 文件路径列表 */
    void setFilePaths(const QStringList& paths);
    /** @brief 获取当前传输速率 @return 速率(字节/秒) */
    double transferRate() const;
    /** @brief 获取预计剩余时间 @return 秒数，无法估算返回-1 */
    double etaSeconds() const;

    // ── 统计计数器 ──
    /** @brief 获取已发送数据块总数(Block0+数据块) @return 块总数 */
    quint64 totalBlocksSent() const;
    /** @brief 获取传输重试总次数 @return 重试次数 */
    quint64 totalRetries() const;
    /** @brief 获取传输错误总次数(CAN/写入失败) @return 错误次数 */
    quint64 totalErrorCount() const;
    /** @brief 获取累计超时事件次数 @return 超时次数 */
    quint64 totalTimeouts() const { return m_totalTimeouts; }
    /** @brief 获取累计接收方取消次数(CAN) @return 取消次数 */
    quint64 totalCancels() const { return m_totalCancels; }
    /** @brief 重置统计计数器 */
    void resetYmodemStatistics();

signals:
    /** @brief 速率/ETA更新(每次ACK后) @param rateBytesPerSec 速率(字节/秒) @param etaSec 预计剩余时间(秒) @param fileName 当前文件名 */
    void transferStats(double rateBytesPerSec, double etaSec, const QString& fileName);

    /** @brief 单文件完成(批量模式) @param fileName 完成的文件名 @param index 文件在列表中的索引 */
    void fileTransferComplete(const QString& fileName, int index);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;             ///< 初始化: 校验文件->计算总量->加载首文件->等C/NAK
    void sendCancelBytes() override;         ///< 发送2个CAN(0x18)取消
    void processReceivedData() override;     ///< 接收状态机核心(逐字节解析ACK/NAK/CAN/C)
    void handleTimeout() override;           ///< 超时处理(重试超10次发CAN取消)

private:
    // ---- YMODEM协议控制字节 ----
    static constexpr int kBlockSize = 128;  ///< YMODEM固定128字节块

    /** @brief YMODEM内部协议状态 */
    enum class State { Idle, WaitingStart, SendingBlock0, SendingData, SendingEOT, WaitBlock0Ack, WaitFinalC, SendingFinalBlock0, Done, Error };

    void sendBlock0();                      ///< 发送Block0文件信息帧
    void sendBlock();                       ///< 发送当前数据块(128B)
    void sendEOT();                         ///< 发送EOT字节
    void sendFinalBlock0();                 ///< 发送空Block0结束会话
    QByteArray buildBlock(int blockNum, const QByteArray& blockData); ///< 构建数据包: SOH+blockNum+~blockNum+data(128B)+CRC16
    QByteArray buildBlock0(const QString& fileName, qint64 fileSize, qint64 modTime); ///< 构建Block0载荷(128B)
    void updateTransferStats();             ///< 更新速率/ETA并发射transferStats
    bool loadNextFile();                    ///< 加载下个文件(失败emit transferError)
    bool writeChecked(const QByteArray& data); ///< 安全写入(检测连接断开)

    // ---- 状态处理方法(processReceivedData状态分发) ----
    void handleStateWaitingStart(char ch);          ///< WaitingStart: 收到C/NAK后发送Block0
    void handleStateSendingBlock0(char ch, int& readIdx); ///< SendingBlock0: ACK/C/NAK/CAN后转移
    void handleStateSendingData(char ch, int& readIdx); ///< SendingData: ACK/NAK/CAN后更新进度
    void handleStateSendingEOT(char ch, int& readIdx); ///< SendingEOT: ACK/NAK后进入下一文件
    void handleStateWaitBlock0Ack(char ch, int& readIdx); ///< WaitBlock0Ack: C/NAK后加载下一文件
    void handleStateWaitFinalC(char ch);            ///< WaitFinalC: C/NAK后发送空Block0
    void handleStateSendingFinalBlock0(char ch, int& readIdx); ///< SendingFinalBlock0: ACK/NAK/CAN后完成

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

    // ── 统计计数器 ──
    quint64 m_totalBlocksSent = 0;    ///< 已发送数据块总数(Block0+数据块)
    quint64 m_totalRetries = 0;       ///< 传输重试总次数(超时/NAK触发)
    quint64 m_totalErrorCount = 0;    ///< 传输错误总次数(CAN/写入失败)
    quint64 m_totalTimeouts = 0;      ///< 累计超时事件次数
    quint64 m_totalCancels = 0;       ///< 累计接收方取消次数(CAN)
};

#endif // YMODEMTRANSFER_H
