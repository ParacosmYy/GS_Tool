#ifndef SERIAL_STATION_CONTROLLER_H
#define SERIAL_STATION_CONTROLLER_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>

#include "apps/serial_station/core/SerialManager.h"
#include "apps/serial_station/protocols/SerialProtocolRegistry.h"

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
    SerialManager& serialManager();

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
     * @brief 系统日志需要展示到 UI。
     * @param text 日志文本
     */
    void serialSystemLogged(const QString& text);

    /**
     * @brief 一次发送成功。
     */
    void serialTxCounted();

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
    void serialCommandPrepared(const QString& command,
                               const QString& mode,
                               const QByteArray& frame);

    /**
     * @brief 命令发送成功。
     * @param command 命令文本
     * @param mode 发送模式
     * @param bytesWritten 写入字节数
     */
    void serialCommandSent(const QString& command,
                           const QString& mode,
                           qint64 bytesWritten);

    /**
     * @brief 命令发送失败。
     * @param command 命令文本
     * @param mode 发送模式
     * @param message 失败原因
     */
    void serialCommandFailed(const QString& command,
                             const QString& mode,
                             const QString& message);

private:
    QString normalizeSendMode(const QString& mode) const;
    QByteArray buildCommandFrame(const QString& command, const QString& mode) const;
    void emitSendFailure(const QString& command,
                         const QString& mode,
                         const QString& message);

    SerialProtocolRegistry m_protocols;
    SerialManager m_serialManager;
};

} // namespace serial_station

#endif // SERIAL_STATION_CONTROLLER_H
