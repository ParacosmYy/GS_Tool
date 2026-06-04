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
    /** @brief OTA状态: Idle→Selecting→Transferring→Verifying→Complete/Error */
    enum class OtaState { Idle, Selecting, Transferring, Verifying, Complete, Error };
    Q_ENUM(OtaState)

    /** @brief 固件文件类型 */
    enum class FirmwareType { Binary, IntelHex, Unknown };

    /** @brief 校验和验证结果 */
    enum class VerifyResult {
        Ok,             ///< 校验通过
        FileNotFound,   ///< 文件不存在
        ReadError,      ///< 读取失败
        ChecksumEmpty,  ///< 校验和文件为空
        Mismatch        ///< 校验和不匹配
    };

    static constexpr qint64 kMaxFirmwareSize = 64 * 1024 * 1024; ///< 最大固件64MB

    /** @brief 构造OTA管理器 @param parent 父对象 */
    explicit OtaManager(QObject* parent = nullptr);
    /** @brief 析构，释放传输协议实例 */
    ~OtaManager() override;

    /** @brief 设置数据连接 @param conn 连接实例(外部管理生命周期) */
    void setConnection(IConnection* conn);
    /** @brief 开始OTA传输，自动检测BIN/HEX并转BIN @param filePath 固件文件路径 @param protocol 协议名(如"xmodem-crc"，默认"xmodem-crc") @return true=启动成功 */
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");
    /** @brief 取消当前传输 */
    void cancelTransfer();
    /** @brief 是否正在传输 @return true=传输中 */
    bool isTransferring() const;
    /** @brief 获取当前OTA状态 @return 状态枚举 */
    OtaState otaState() const;

    /** @brief 获取历史传输次数 @return 传输次数 */
    int transferCount() const;
    /** @brief 上次传输是否成功 @return true=成功 */
    bool lastTransferSuccess() const;
    /** @brief 获取上次文件名 @return 文件名 */
    QString lastFileName() const;
    /** @brief 获取当前协议名 @return 协议名 */
    QString currentProtocolName() const;

    // ---- 统计 ----
    /** @brief 获取传输尝试总次数 @return 总次数 */
    quint64 totalTransfers() const;
    /** @brief 获取成功传输次数 @return 成功次数 */
    quint64 successfulTransfers() const;
    /** @brief 获取失败传输次数 @return 失败次数 */
    quint64 failedTransfers() const;
    /** @brief 获取累计传输字节数 @return 字节总数 */
    quint64 totalBytesTransferred() const;
    /** @brief 获取累计CRC校验验证次数 @return 校验次数 */
    quint64 totalCrcChecks() const;
    /** @brief 获取历史平均传输速率 @return 速率(字节/秒) */
    double averageSpeed() const;
    /** @brief 重置统计(不影响历史记录) */
    void resetTransferStatistics();

    /** @brief 获取累计传输取消次数 @return 取消次数 */
    quint64 totalCancellations() const;

    /** @brief 获取累计HEX转BIN次数 @return 转换次数 */
    quint64 totalHexConversions() const;

    /** @brief 获取累计协议切换次数 @return 切换次数 */
    quint64 totalProtocolSwitches() const;

    /** @brief 验证固件文件(存在/可读/大小限制) @param filePath 文件路径 @param errorMsg 输出: 错误描述 @return true=有效 */
    bool validateFilePath(const QString& filePath, QString& errorMsg) const;
    /** @brief 检测文件类型(.bin->Binary, .hex->IntelHex) @param filePath 文件路径 @return 文件类型枚举 */
    FirmwareType detectFirmwareType(const QString& filePath) const;

    /**
     * @brief 传输后校验和验证
     * @param filePath 固件文件路径
     * @param expectedChecksum 预期的校验和(hex字符串，如CRC32/MD5前8位)
     * @param outError 错误描述输出
     * @return VerifyResult 校验结果
     *
     * 传输完成后可调用此方法对固件文件进行CRC32校验，
     * 确保本地文件与传输前一致。支持CRC32十六进制字符串比对。
     */
    VerifyResult verifyChecksum(const QString& filePath,
                                const QString& expectedChecksum,
                                QString& outError);

signals:
    /** @brief 传输进度更新 @param percent 百分比 @param bytesSent 已发送字节 @param totalBytes 总字节 */
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    /** @brief 传输完成 */
    void transferComplete();
    /** @brief 传输错误 @param reason 错误原因 */
    void transferError(const QString& reason);
    /** @brief 速率和ETA更新 @param rateBytesPerSec 速率(字节/秒) @param etaSec 预计剩余时间(秒) */
    void transferStats(double rateBytesPerSec, double etaSec);
    /** @brief OTA状态变化 @param state 新状态 */
    void otaStateChanged(OtaManager::OtaState state);
    /** @brief 协议降级通知 @param message 降级说明 */
    void modeDegraded(const QString& message);

private:
    /** @brief 绑定BaseTransfer信号到OTA管理器槽 @param transfer 传输协议实例 */
    void connectTransferSignals(BaseTransfer* transfer);
    /** @brief 绑定XModem统计信号 */
    void connectXModemStats();
    /** @brief 绑定YModem统计信号 */
    void connectYModemStats();
    /** @brief 设置OTA状态并发射otaStateChanged信号 @param state 新状态 */
    void setOtaState(OtaState state);
    /** @brief HEX转BIN转换，创建临时文件 @param hexPath HEX文件路径 @param outBinPath 输出BIN路径 @return true=转换成功 */
    bool convertHexToBin(const QString& hexPath, QString& outBinPath);
    /** @brief 获取协议可读名称(用于错误消息) @param protocol 协议标识 @return 可读名称 */
    QString protocolDisplayName(const QString& protocol) const;
    /** @brief 计算文件的CRC32校验和 @param filePath 文件路径 @return CRC32十六进制字符串，失败返回空 */
    QString computeFileCrc32(const QString& filePath);

    IConnection* m_conn = nullptr;               ///< 数据连接(不拥有)
    XModemTransfer* m_xmodem = nullptr;          ///< XModem传输实例
    YModemTransfer* m_ymodem = nullptr;          ///< YModem传输实例
    ZModemTransfer* m_zmodem = nullptr;          ///< ZModem传输实例

    OtaState m_otaState = OtaState::Idle;        ///< 当前OTA状态
    QTemporaryFile* m_tempBinFile = nullptr;     ///< HEX转BIN临时文件
    QString m_tempBinPath;                       ///< 临时BIN文件路径
    QString m_currentFileName;                   ///< 当前传输文件名
    QString m_currentProtocol;                   ///< 当前传输协议名
    int m_transferCount = 0;                     ///< 历史传输次数
    bool m_lastTransferSuccess = false;          ///< 上次传输是否成功

    quint64 m_totalTransfers = 0;                ///< 传输尝试总次数
    quint64 m_successfulTransfers = 0;           ///< 成功传输次数
    quint64 m_failedTransfers = 0;               ///< 失败传输次数
    quint64 m_totalBytesTransferred = 0;         ///< 累计传输字节数
    quint64 m_totalCrcChecks = 0;                ///< 累计CRC校验次数
    qint64 m_currentFileSize = 0;                ///< 当前文件大小(字节)

    // ---- 新增统计计数器 ----
    quint64 m_totalCancellations = 0;            ///< 累计传输取消次数
    quint64 m_totalHexConversions = 0;           ///< 累计HEX转BIN次数
    quint64 m_totalProtocolSwitches = 0;         ///< 累计协议切换次数(startTransfer时协议变化)

    // ---- 速率跟踪 ----
    QElapsedTimer m_transferTimer;          ///< 当前传输耗时计时器
    QVector<double> m_speedHistory;         ///< 历史传输速率记录(字节/秒)
    static constexpr int kMaxSpeedHistory = 100; ///< 速率历史最大保留条数
};

#endif // OTAMANAGER_H
