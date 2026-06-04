/**
 * @file SerialConnectionStats.cpp
 * @brief 串口配置、统计与辅助方法 — 参数配置getter/setter、信号线控制和操作统计
 *
 * 从 SerialConnection.cpp 拆分而来，包含:
 *   - 串口参数配置: setPortName/portName/setBaudRate/baudRate/
 *     setDataBits/dataBits/setParity/parity/setStopBits/stopBits/
 *     setFlowControl/flowControl
 *   - 信号线控制: setDtr/setRts/isDtr/isRts
 *   - 统一配置入口: configure()
 *   - 可用端口查询: availablePorts()
 *   - 读取回调: onReadyRead()
 *   - 统计重置: resetStats()
 */

#include "connection/serial_port/SerialConnection.h"
#include <QDebug>

/** @brief 设置端口名 @param portName 串口端口名称(如COM3、/dev/ttyUSB0) */
void SerialConnection::setPortName(const QString& portName)
{
    m_portName = portName;
}

/** @brief 获取当前端口名 @return 串口端口名称字符串 */
QString SerialConnection::portName() const
{
    return m_portName;
}

/** @brief 设置波特率，失败时输出警告日志 @param baud 波特率值(如9600、115200) */
void SerialConnection::setBaudRate(qint32 baud)
{
    if (!m_serial.setBaudRate(baud)) {
        qWarning() << tr("设置波特率失败, baud=%1").arg(baud);
    }
}

/** @brief 获取当前波特率 @return 波特率数值 */
qint32 SerialConnection::baudRate() const
{
    return m_serial.baudRate();
}

/** @brief 设置数据位，失败时输出警告日志 @param bits 数据位枚举值(Data5~Data8) */
void SerialConnection::setDataBits(QSerialPort::DataBits bits)
{
    if (!m_serial.setDataBits(bits)) {
        qWarning() << tr("设置数据位失败, bits=%1").arg(bits);
    }
}

/** @brief 获取当前数据位 @return 数据位枚举值 */
QSerialPort::DataBits SerialConnection::dataBits() const
{
    return m_serial.dataBits();
}

/** @brief 设置校验模式，失败时输出警告日志 @param parity 校验枚举值(NoParity/EvenParity/OddParity等) */
void SerialConnection::setParity(QSerialPort::Parity parity)
{
    if (!m_serial.setParity(parity)) {
        qWarning() << tr("设置校验模式失败, parity=%1").arg(parity);
    }
}

/** @brief 获取当前校验模式 @return 校验枚举值 */
QSerialPort::Parity SerialConnection::parity() const
{
    return m_serial.parity();
}

/** @brief 设置停止位，失败时输出警告日志 @param bits 停止位枚举值(OneStop/OneAndHalfStop/TwoStop) */
void SerialConnection::setStopBits(QSerialPort::StopBits bits)
{
    if (!m_serial.setStopBits(bits)) {
        qWarning() << tr("设置停止位失败, bits=%1").arg(bits);
    }
}

/** @brief 获取当前停止位 @return 停止位枚举值 */
QSerialPort::StopBits SerialConnection::stopBits() const
{
    return m_serial.stopBits();
}

/** @brief 设置流控模式，失败时输出警告日志 @param control 流控枚举值(NoFlowControl/HardwareControl/SoftwareControl) */
void SerialConnection::setFlowControl(QSerialPort::FlowControl control)
{
    if (!m_serial.setFlowControl(control)) {
        qWarning() << tr("设置流控模式失败, control=%1").arg(control);
    }
}

/** @brief 获取当前流控模式 @return 流控枚举值 */
QSerialPort::FlowControl SerialConnection::flowControl() const
{
    return m_serial.flowControl();
}

/** @brief 设置DTR信号电平 @param enabled true=高电平，false=低电平 */
void SerialConnection::setDtr(bool enabled)
{
    ++m_totalPinChanges;
    m_serial.setDataTerminalReady(enabled);
}

/** @brief 设置RTS信号电平 @param enabled true=高电平，false=低电平 */
void SerialConnection::setRts(bool enabled)
{
    ++m_totalPinChanges;
    m_serial.setRequestToSend(enabled);
}

/** @brief 查询DTR信号当前状态 @return true=DTR高电平，false=DTR低电平 */
bool SerialConnection::isDtr() const
{
    // QSerialPort::isDataTerminalReady() 非 const，需要 const_cast
    return const_cast<QSerialPort&>(m_serial).isDataTerminalReady();
}

/** @brief 查询RTS信号当前状态 @return true=RTS高电平，false=RTS低电平 */
bool SerialConnection::isRts() const
{
    // QSerialPort::isRequestToSend() 非 const，需要 const_cast
    return const_cast<QSerialPort&>(m_serial).isRequestToSend();
}

/** @brief 通过参数映射配置串口(工厂模式下的统一配置入口)，未识别的key安全忽略，数值型参数有范围检查 @param params 参数映射表，支持portName/baudRate/dataBits/parity/stopBits/flowControl/dtr/rts */
void SerialConnection::configure(const QVariantMap& params)
{
    ++m_totalConfigChanges;
    if (params.contains("portName"))
        setPortName(params["portName"].toString());
    if (params.contains("baudRate"))
        setBaudRate(params["baudRate"].toInt());
    if (params.contains("dataBits")) {
        int db = params["dataBits"].toInt();
        QSerialPort::DataBits bits[] = {
            QSerialPort::Data5, QSerialPort::Data6,
            QSerialPort::Data7, QSerialPort::Data8
        };
        if (db >= 5 && db <= 8) setDataBits(bits[db - 5]);
    }
    if (params.contains("parity")) {
        QSerialPort::Parity p[] = {
            QSerialPort::NoParity, QSerialPort::EvenParity,
            QSerialPort::OddParity, QSerialPort::MarkParity,
            QSerialPort::SpaceParity
        };
        int idx = params["parity"].toInt();
        if (idx >= 0 && idx <= 4) setParity(p[idx]);
    }
    if (params.contains("stopBits")) {
        QSerialPort::StopBits s[] = {
            QSerialPort::OneStop, QSerialPort::OneAndHalfStop,
            QSerialPort::TwoStop
        };
        int idx = params["stopBits"].toInt();
        if (idx >= 0 && idx <= 2) setStopBits(s[idx]);
    }
    if (params.contains("flowControl")) {
        QSerialPort::FlowControl f[] = {
            QSerialPort::NoFlowControl, QSerialPort::HardwareControl,
            QSerialPort::SoftwareControl
        };
        int idx = params["flowControl"].toInt();
        if (idx >= 0 && idx <= 2) setFlowControl(f[idx]);
    }
    if (params.contains("dtr"))
        setDtr(params["dtr"].toBool());
    if (params.contains("rts"))
        setRts(params["rts"].toBool());
}

/** @brief 获取系统中所有可用的串口列表 @return QSerialPortInfo列表 */
QList<QSerialPortInfo> SerialConnection::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

/** @brief QSerialPort::readyRead信号处理，读取全部缓冲区数据并转发dataReceived信号，空数据不触发信号 */
void SerialConnection::onReadyRead()
{
    QByteArray data = m_serial.readAll();
    if (!data.isEmpty()) {
        m_totalBytesRead += static_cast<quint64>(data.size());  // 累计读取字节统计
        emit dataReceived(data);
    }
}

/** @brief 重置所有操作统计计数器(totalOpens/totalCloses/totalBytesWritten/totalBytesRead/errorCount/configChanges/pinChanges归零)，不影响SerialErrorCounters */
void SerialConnection::resetStats()
{
    m_totalOpens = 0;
    m_totalCloses = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_totalWrites = 0;
    m_errorCount = 0;
    m_totalConfigChanges = 0;
    m_totalPinChanges = 0;
}
