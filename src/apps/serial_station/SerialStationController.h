#ifndef SERIAL_STATION_CONTROLLER_H
#define SERIAL_STATION_CONTROLLER_H

#include <QtCore/QObject>

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

private:
    SerialProtocolRegistry m_protocols;
    SerialManager m_serialManager;
};

} // namespace serial_station

#endif // SERIAL_STATION_CONTROLLER_H
