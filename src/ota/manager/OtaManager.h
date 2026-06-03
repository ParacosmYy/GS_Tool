/** @file OtaManager.h @brief OTA升级管理器 - 协调传输协议(XModem/YModem/ZModem)、文件验证和HEX转BIN */
#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <QObject>
#include <QTemporaryFile>
#include "ota/protocols/base/BaseTransfer.h"
#include "ota/protocols/xmodem/XModemTransfer.h"
#include "ota/protocols/ymodem/YModemTransfer.h"
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include "connection/interface/IConnection.h"

/** @brief OTA升级管理器(策略模式)，管理三种传输协议实例和状态机 */
class OtaManager : public QObject {
    Q_OBJECT

public:
    /** @brief OTA状态: Idle→Selecting→Transferring→Verifying→Complete/Error */
    enum class OtaState { Idle, Selecting, Transferring, Verifying, Complete, Error };
    Q_ENUM(OtaState)

    /** @brief 固件文件类型 */
    enum class FirmwareType { Binary, IntelHex, Unknown };

    static constexpr qint64 kMaxFirmwareSize = 64 * 1024 * 1024; ///< 最大固件64MB

    explicit OtaManager(QObject* parent = nullptr);
    ~OtaManager() override;

    void setConnection(IConnection* conn); ///< 设置数据连接
    /** @brief 开始OTA传输，自动检测BIN/HEX并转BIN @param protocol "xmodem-crc"等 */
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");
    void cancelTransfer();   ///< 取消传输
    bool isTransferring() const; ///< 是否正在传输
    OtaState otaState() const;   ///< 当前状态

    int transferCount() const;       ///< 历史传输次数
    bool lastTransferSuccess() const; ///< 上次是否成功
    QString lastFileName() const;     ///< 上次文件名
    QString currentProtocolName() const; ///< 当前协议名

    // 统计
    quint64 totalTransfers() const;         ///< 传输尝试总次数
    quint64 successfulTransfers() const;    ///< 成功次数
    quint64 failedTransfers() const;        ///< 失败次数
    quint64 totalBytesTransferred() const;  ///< 累计传输字节
    void resetTransferStatistics();         ///< 重置统计(不影响历史记录)

    /** @brief 验证固件文件(存在/可读/大小限制) */
    bool validateFilePath(const QString& filePath, QString& errorMsg) const;
    /** @brief 检测文件类型(.bin→Binary, .hex→IntelHex) */
    FirmwareType detectFirmwareType(const QString& filePath) const;

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes); ///< 传输进度
    void transferComplete();                          ///< 传输完成
    void transferError(const QString& reason);        ///< 传输错误
    void transferStats(double rateBytesPerSec, double etaSec); ///< 速率和ETA
    void otaStateChanged(OtaManager::OtaState state); ///< 状态变化
    void modeDegraded(const QString& message);         ///< 协议降级通知

private:
    void connectTransferSignals(BaseTransfer* transfer); ///< 绑定BaseTransfer信号
    void connectXModemStats(); ///< 绑定XModem统计信号
    void connectYModemStats(); ///< 绑定YModem统计信号
    void setOtaState(OtaState state); ///< 设置状态并发射信号
    /** @brief HEX→BIN转换，创建临时文件 */
    bool convertHexToBin(const QString& hexPath, QString& outBinPath);
    /** @brief 协议可读名称(用于错误消息) */
    QString protocolDisplayName(const QString& protocol) const;

    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
    ZModemTransfer* m_zmodem = nullptr;

    OtaState m_otaState = OtaState::Idle;
    QTemporaryFile* m_tempBinFile = nullptr; ///< HEX转BIN临时文件
    QString m_tempBinPath;
    QString m_currentFileName;
    QString m_currentProtocol;
    int m_transferCount = 0;
    bool m_lastTransferSuccess = false;

    quint64 m_totalTransfers = 0;
    quint64 m_successfulTransfers = 0;
    quint64 m_failedTransfers = 0;
    quint64 m_totalBytesTransferred = 0;
    qint64 m_currentFileSize = 0;
};

#endif // OTAMANAGER_H
