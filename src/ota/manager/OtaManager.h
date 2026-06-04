/** @file OtaManager.h @brief OTA升级管理器 - 协调传输协议(XModem/YModem/ZModem)、文件验证、校验和验证和HEX转BIN */
#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <QObject>
#include <QTemporaryFile>
#include <QElapsedTimer>
#include <QVector>
#include "ota/protocols/base/BaseTransfer.h"
#include "ota/protocols/xmodem/XModemTransfer.h"
#include "ota/protocols/ymodem/YModemTransfer.h"
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include "connection/interface/IConnection.h"

/** @brief OTA升级管理器(策略模式)，管理三种传输协议实例和状态机 */
class OtaManager : public QObject {
    Q_OBJECT

public:
    enum class OtaState { Idle, Selecting, Transferring, Verifying, Complete, Error }; ///< OTA状态
    Q_ENUM(OtaState)
    enum class FirmwareType { Binary, IntelHex, Unknown }; ///< 固件文件类型
    enum class VerifyResult { Ok, FileNotFound, ReadError, ChecksumEmpty, Mismatch }; ///< 校验结果

    static constexpr qint64 kMaxFirmwareSize = 64 * 1024 * 1024; ///< 最大固件64MB

    struct Stats {
        quint64 totalTransfers = 0; quint64 successfulTransfers = 0; quint64 failedTransfers = 0;
        quint64 totalBytesTransferred = 0; quint64 totalCrcChecks = 0; quint64 totalCancellations = 0;
        quint64 totalHexConversions = 0; quint64 totalProtocolSwitches = 0;
    };

    explicit OtaManager(QObject* parent = nullptr);
    ~OtaManager() override;

    void setConnection(IConnection* conn);               ///< 设置数据连接
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc"); ///< 开始OTA传输
    void cancelTransfer();                                ///< 取消当前传输
    bool isTransferring() const;                          ///< 是否正在传输
    OtaState otaState() const;                            ///< 获取当前OTA状态
    int transferCount() const;                            ///< 获取历史传输次数
    bool lastTransferSuccess() const;                     ///< 上次传输是否成功
    QString lastFileName() const;                         ///< 获取上次文件名
    QString currentProtocolName() const;                  ///< 获取当前协议名

    const Stats& stats() const { return m_stats; }       ///< 获取统计只读引用
    void resetStats() { m_stats = Stats{}; m_speedHistory.clear(); }

    // ── 向后兼容便捷Getter ──
    quint64 totalTransfers() const { return m_stats.totalTransfers; }
    quint64 successfulTransfers() const { return m_stats.successfulTransfers; }
    quint64 failedTransfers() const { return m_stats.failedTransfers; }
    quint64 totalBytesTransferred() const { return m_stats.totalBytesTransferred; }
    quint64 totalCrcChecks() const { return m_stats.totalCrcChecks; }
    quint64 totalCancellations() const { return m_stats.totalCancellations; }
    quint64 totalHexConversions() const { return m_stats.totalHexConversions; }
    quint64 totalProtocolSwitches() const { return m_stats.totalProtocolSwitches; }
    void resetTransferStatistics() { resetStats(); }
    double averageSpeed() const;                          ///< 获取历史平均传输速率

    bool validateFilePath(const QString& filePath, QString& errorMsg) const;
    FirmwareType detectFirmwareType(const QString& filePath) const;
    VerifyResult verifyChecksum(const QString& filePath, const QString& expectedChecksum, QString& outError);

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes); ///< 传输进度更新
    void transferComplete();                         ///< 传输完成
    void transferError(const QString& reason);       ///< 传输错误
    void transferStats(double rateBytesPerSec, double etaSec); ///< 速率和ETA更新
    void otaStateChanged(OtaManager::OtaState state); ///< OTA状态变化
    void modeDegraded(const QString& message);        ///< 协议降级通知

private:
    void connectTransferSignals(BaseTransfer* transfer);
    void connectXModemStats();
    void connectYModemStats();
    void setOtaState(OtaState state);
    bool convertHexToBin(const QString& hexPath, QString& outBinPath);
    QString protocolDisplayName(const QString& protocol) const;
    QString computeFileCrc32(const QString& filePath);

    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
    ZModemTransfer* m_zmodem = nullptr;
    OtaState m_otaState = OtaState::Idle;
    QTemporaryFile* m_tempBinFile = nullptr;
    QString m_tempBinPath, m_currentFileName, m_currentProtocol;
    int m_transferCount = 0;
    bool m_lastTransferSuccess = false;
    qint64 m_currentFileSize = 0;
    Stats m_stats;
    QElapsedTimer m_transferTimer;
    QVector<double> m_speedHistory;
    static constexpr int kMaxSpeedHistory = 100;
};

#endif // OTAMANAGER_H
