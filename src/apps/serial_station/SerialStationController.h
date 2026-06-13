#ifndef SERIAL_STATION_CONTROLLER_H
#define SERIAL_STATION_CONTROLLER_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "apps/serial_station/core/SerialCodec.h"
#include "apps/serial_station/core/SerialDispatcher.h"
#include "apps/serial_station/core/SerialManager.h"
#include "apps/serial_station/protocols/SerialProtocolRegistry.h"
#include "apps/serial_station/services/SerialExportService.h"
#include "apps/serial_station/services/SerialLogService.h"
#include "apps/serial_station/services/SerialProfileService.h"
#include "apps/serial_station/services/SerialReplayService.h"

namespace serial_station {

/**
 * @brief Serial Station 控制器。
 *
 * 作为 UI 与 core/protocols 的唯一协调入口。
 */
class SerialStationController : public QObject {
    Q_OBJECT

public:
    explicit SerialStationController(QObject* parent = nullptr);

    SerialProtocolRegistry& protocols();
    QStringList availableProtocolNames() const;
    QString activeProtocolName() const;
    SerialManager& serialManager();
    SerialLogService& logService();
    const SerialLogService& logService() const;

    /** @brief 返回当前会话结构化日志快照。 */
    QVector<SerialLogRecord> logRecords() const;

    /** @brief 返回过滤后的结构化日志快照。 */
    QVector<SerialLogRecord> logRecords(const SerialLogFilter& filter) const;

    /** @brief 返回当前会话日志纯文本。 */
    QString logPlainText() const;

    /** @brief 返回过滤后的日志纯文本。 */
    QString logPlainText(const SerialLogFilter& filter) const;

    /** @brief 返回当前会话日志 JSON Lines。 */
    QString logJsonLines() const;

    /** @brief 返回过滤后的日志 JSON Lines。 */
    QString logJsonLines(const SerialLogFilter& filter) const;

    /** @brief 导出当前结构化日志快照。 */
    SerialExportResult exportLogRecords(const SerialExportRequest& request);

    /** @brief 根据导出格式生成建议文件名。 */
    QString suggestedExportFileName(SerialExportFormat format) const;

    /** @brief 基于当前日志生成回放预览计划。 */
    SerialReplayPlan previewReplayPlan(
        const SerialReplayOptions& options = SerialReplayOptions());

    /** @brief 保存 Serial Station 配置档案到文件。 */
    SerialProfileWriteResult saveProfileToFile(const SerialStationProfile& profile,
                                               const QString& filePath);

    /** @brief 从文件加载 Serial Station 配置档案。 */
    SerialProfileResult loadProfileFromFile(const QString& filePath);

public slots:
    /**
     * @brief 应用 UI 提交的 UART 配置并尝试打开串口。
     * @param config UART 配置
     */
    void connectSerialPort(const SerialPortConfig& config);

    /**
     * @brief 关闭当前串口会话。
     */
    void disconnectSerialPort();

    /**
     * @brief 发送 UI 提交的命令。
     * @param command 命令文本
     * @param mode 发送模式
     */
    void sendCommand(const QString& command, const QString& mode);

    /**
     * @brief 处理串口核心层收到的原始字节。
     * @param bytes 原始接收数据
     */
    void handleBytesReceived(const QByteArray& bytes);

    /**
     * @brief 清空 controller 持有的结构化日志。
     */
    void clearLogRecords();

    /**
     * @brief 切换 protocol 模式和接收 dispatcher 使用的默认协议。
     * @param protocolName 协议注册名
     */
    void setActiveProtocol(const QString& protocolName);

signals:
    /**
     * @brief 串口会话状态变化。
     * @param state 新状态
     */
    void serialStateChanged(SerialSessionState state);

    /**
     * @brief 串口错误向 UI 层传播。
     * @param message 错误描述
     */
    void serialErrorOccurred(const QString& message);

    /**
     * @brief 发送日志需要展示到 UI。
     * @param text 日志文本
     */
    void serialTxLogged(const QString& text);

    /**
     * @brief 接收日志需要展示到 UI。
     * @param text 日志文本
     */
    void serialRxLogged(const QString& text);

    /**
     * @brief 系统日志需要展示到 UI。
     * @param text 日志文本
     */
    void serialSystemLogged(const QString& text);

    /**
     * @brief 一次发送成功。
     */
    void serialTxCounted();

    /**
     * @brief 一次接收帧解析成功。
     */
    void serialRxCounted();

    /**
     * @brief 一次发送失败。
     */
    void serialErrorCounted();

    /**
     * @brief 命令已构建为发送帧。
     * @param command 命令文本
     * @param mode 发送模式
     * @param frame 已构建帧
     */
    void serialCommandPrepared(const QString& command, const QString& mode, const QByteArray& frame);

    /**
     * @brief 命令发送成功。
     * @param command 命令文本
     * @param mode 发送模式
     * @param bytesWritten 写入字节数
     */
    void serialCommandSent(const QString& command, const QString& mode, qint64 bytesWritten);

    /**
     * @brief 命令发送失败。
     * @param command 命令文本
     * @param mode 发送模式
     * @param message 失败原因
     */
    void serialCommandFailed(const QString& command, const QString& mode, const QString& message);

    /**
     * @brief 当前默认协议已变化。
     * @param protocolName 协议注册名
     */
    void activeProtocolChanged(const QString& protocolName);

private:
    QString normalizeSendMode(const QString& mode) const;
    SerialCodec::EncodeResult buildCommandFrame(const QString& command,
                                                const QString& mode) const;
    void resetReceiveDispatcher();
    void handleSerialManagerError(const QString& message);
    void processProtocolEvent(const SerialProtocolEvent& event);
    QString eventPayloadText(const SerialProtocolEvent& event) const;
    QString rawBytesSummary(const QByteArray& bytes) const;
    QString bufferedReceiveText(const SerialDispatcher::FeedSummary& summary) const;
    void logTx(const QString& text, const QByteArray& payload, const QVariantMap& fields);
    void logRx(const QString& text, const QByteArray& payload, const QVariantMap& fields);
    void logSystem(const QString& text, const QVariantMap& fields = QVariantMap());
    void logError(const QString& text, const QVariantMap& fields = QVariantMap());
    void emitSendFailure(const QString& command,
                         const QString& mode,
                         const QString& message);

    SerialProtocolRegistry m_protocols;
    SerialManager m_serialManager;
    SerialDispatcher m_dispatcher;
    SerialCodec m_codec;
    SerialLogService m_logService;
    SerialExportService m_exportService;
    SerialProfileService m_profileService;
    SerialReplayService m_replayService;
};

} // namespace serial_station

#endif // SERIAL_STATION_CONTROLLER_H
