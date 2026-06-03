/**
 * @file OtaManager.h
 * @brief OTA升级管理器 - 协调传输协议、文件验证和连接管理
 *
 * OtaManager是OTA升级业务层的核心管理类，职责:
 *   1. 管理三种传输协议实例(XModem/YModem/ZModem)
 *   2. 文件路径验证(存在性/可读性/大小限制)
 *   3. 固件文件类型检测(BIN/HEX自动识别)
 *   4. HEX文件自动转BIN后传输(IntelHexParser集成)
 *   5. OTA状态机管理(Idle→Selecting→Transferring→Verifying→Complete/Error)
 *   6. 转发传输协议的progress/transferStats/完成/错误信号
 *
 * 协作关系:
 *   - BaseTransfer/XModemTransfer/YModemTransfer: 传输协议实例
 *   - IConnection: 数据收发通道
 *   - OtaWidget: UI面板，连接信号显示状态
 *   - IntelHexParser: HEX文件解析转BIN
 *
 * 设计模式: 策略模式(Strategy) - 通过协议名选择不同传输协议
 */
#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <QObject>
#include <QTemporaryFile>
#include "ota/protocols/base/BaseTransfer.h"
#include "ota/protocols/xmodem/XModemTransfer.h"
#include "ota/protocols/ymodem/YModemTransfer.h"
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include "connection/interface/IConnection.h"

/**
 * @brief OTA升级管理器
 *
 * 通过BaseTransfer*统一管理三个协议实例的信号连接，
 * 提供文件验证、HEX转BIN、状态管理等业务逻辑。
 */
class OtaManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief OTA状态枚举 - 描述整个OTA升级流程的当前阶段
     *
     * 状态流转:
     *   Idle ──→ Selecting ──→ Transferring ──→ Verifying ──→ Complete
     *     │                        │                              ↑
     *     └────────────────────────┴──── Error ←─────────────────┘
     */
    enum class OtaState {
        Idle,           ///< 空闲，未开始任何操作
        Selecting,      ///< 文件选择和验证阶段
        Transferring,   ///< 数据传输进行中
        Verifying,      ///< 传输完成后的校验阶段
        Complete,       ///< 传输完成且校验通过
        Error           ///< 传输失败或文件验证失败
    };
    Q_ENUM(OtaState)

    /**
     * @brief 固件文件类型枚举
     */
    enum class FirmwareType {
        Binary,         ///< 二进制文件(.bin)
        IntelHex,       ///< Intel HEX文件(.hex)
        Unknown         ///< 无法识别的文件类型
    };

    /** @brief 固件文件最大允许大小(64MB) */
    static constexpr qint64 kMaxFirmwareSize = 64 * 1024 * 1024;

    explicit OtaManager(QObject* parent = nullptr);
    ~OtaManager() override;  // 清理HEX转换产生的临时BIN文件

    /** @brief 设置数据连接(串口/TCP/UDP) */
    void setConnection(IConnection* conn);

    /**
     * @brief 开始OTA传输
     * @param filePath 固件文件路径(BIN或HEX)
     * @param protocol 传输协议("xmodem-crc"/"xmodem-checksum"/"xmodem-1k"/"ymodem"/"zmodem")
     * @return true=成功启动，false=验证失败或连接未就绪
     *
     * 自动检测文件类型，HEX文件自动转BIN后传输。
     */
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");

    /** @brief 取消正在进行的传输 */
    void cancelTransfer();

    /** @brief 是否正在传输 */
    bool isTransferring() const;

    /** @brief 获取当前OTA状态 */
    OtaState otaState() const;

    /** @brief 获取历史传输总次数 */
    int transferCount() const;

    /** @brief 获取上次传输是否成功 */
    bool lastTransferSuccess() const;

    /** @brief 获取上次传输的文件名 */
    QString lastFileName() const;

    /** @brief 获取当前协议名称 */
    QString currentProtocolName() const;

    /** @brief 获取传输尝试总次数 */
    quint64 totalTransfers() const;

    /** @brief 获取成功完成的传输次数 */
    quint64 successfulTransfers() const;

    /** @brief 获取失败的传输次数 */
    quint64 failedTransfers() const;

    /** @brief 获取所有会话累计传输的字节数 */
    quint64 totalBytesTransferred() const;

    /**
     * @brief 重置传输统计计数器
     *
     * 将 totalTransfers/successfulTransfers/failedTransfers/totalBytesTransferred 全部清零。
     * 不影响 m_transferCount 和 m_lastTransferSuccess 等历史记录。
     */
    void resetTransferStatistics();

    /**
     * @brief 验证固件文件路径
     * @param filePath 文件路径
     * @param errorMsg 错误信息输出(验证失败时填充)
     * @return true=文件有效，false=文件无效
     *
     * 检查项: 文件存在、可读、大小在限制范围内
     */
    bool validateFilePath(const QString& filePath, QString& errorMsg) const;

    /**
     * @brief 检测固件文件类型
     * @param filePath 文件路径
     * @return 文件类型枚举
     *
     * 通过扩展名判断: .bin→Binary, .hex→IntelHex, 其他→Unknown
     */
    FirmwareType detectFirmwareType(const QString& filePath) const;

signals:
    /** @brief 传输进度更新 @param percent 百分比 @param bytesSent 已发送字节 @param totalBytes 总字节 */
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);

    /** @brief 传输完成 */
    void transferComplete();

    /** @brief 传输错误 @param reason 错误原因 */
    void transferError(const QString& reason);

    /** @brief 传输速率和ETA更新 @param rateBytesPerSec 速率 @param etaSec 预计剩余秒数 */
    void transferStats(double rateBytesPerSec, double etaSec);

    /** @brief OTA状态变化 @param state 新状态 */
    void otaStateChanged(OtaManager::OtaState state);

    /** @brief 协议模式自动降级通知(如CRC→Checksum) @param message 降级描述消息 */
    void modeDegraded(const QString& message);

private:
    /** @brief 统一绑定BaseTransfer的信号到OtaManager的转发 */
    void connectTransferSignals(BaseTransfer* transfer);

    /** @brief 绑定XModemTransfer的transferStats信号 */
    void connectXModemStats();

    /** @brief 绑定YModemTransfer的transferStats信号 */
    void connectYModemStats();

    /** @brief 设置OTA状态并发射状态变化信号 */
    void setOtaState(OtaState state);

    /**
     * @brief 将HEX文件转换为BIN数据并创建临时文件
     * @param hexPath HEX文件路径
     * @param outBinPath 转换后的临时BIN文件路径
     * @return true=转换成功，false=解析失败
     */
    bool convertHexToBin(const QString& hexPath, QString& outBinPath);

    /**
     * @brief 获取协议的可读名称（用于错误消息）
     * @param protocol 协议标识("xmodem-crc"/"ymodem"/"zmodem"等)
     * @return 人类可读的协议名称（如 "XMODEM-CRC"）
     */
    QString protocolDisplayName(const QString& protocol) const;

    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
    ZModemTransfer* m_zmodem = nullptr;

    OtaState m_otaState = OtaState::Idle;   ///< 当前OTA状态
    QTemporaryFile* m_tempBinFile = nullptr; ///< HEX转换临时BIN文件(复用而非累积)
    QString m_tempBinPath;                   ///< HEX转BIN的临时文件路径
    QString m_currentFileName;               ///< 当前传输的文件名（用于错误信息上下文）
    QString m_currentProtocol;               ///< 当前传输协议名称（用于错误信息上下文）
    int m_transferCount = 0;                  ///< 历史传输总次数
    bool m_lastTransferSuccess = false;       ///< 上次传输是否成功

    quint64 m_totalTransfers = 0;             ///< 传输尝试总次数(包含成功和失败)
    quint64 m_successfulTransfers = 0;        ///< 成功完成的传输次数
    quint64 m_failedTransfers = 0;            ///< 失败的传输次数
    quint64 m_totalBytesTransferred = 0;      ///< 所有会话累计传输的字节数
    qint64 m_currentFileSize = 0;             ///< 当前传输文件的字节大小(用于成功后累加到totalBytesTransferred)
};

#endif // OTAMANAGER_H
