#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>

#include "apps/serial_station/core/SerialPort.h"
#include "apps/serial_station/core/SerialSession.h"

namespace serial_station {

/**
 * @brief 串口核心管理器。
 *
 * 负责会话状态和 SerialPort 薄封装协调，不解析具体协议。
 */
class SerialManager : public QObject {
    Q_OBJECT

public:
    explicit SerialManager(QObject* parent = nullptr);

    void configure(const SerialPortConfig& config);
    /**
     * @brief 记录被拒绝的配置并进入错误状态。
     * @param config 被拒绝的 UART 配置
     * @param message 拒绝原因
     */
    void rejectConfiguration(const SerialPortConfig& config, const QString& message);

    SerialSession session() const;

    bool open();
    void close();
    qint64 send(const QByteArray& bytes);

signals:
    void bytesReceived(const QByteArray& bytes);
    void stateChanged(SerialSessionState state);
    void errorOccurred(const QString& message);

private slots:
    void handlePortError(const QString& message);

private:
    SerialPort m_port;
    SerialSession m_session;
};

} // namespace serial_station

#endif // SERIAL_MANAGER_H
