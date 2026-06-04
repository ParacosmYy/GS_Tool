/**
 * @file SerialConnectionUtility.cpp
 * @brief 串口连接 - 错误翻译与辅助方法实现
 *
 * 从 SerialConnectionError.cpp 拆分而来，包含错误码翻译、
 * 写入回调、Break信号发送和引脚信号状态查询方法。
 */

#include "connection/serial_port/SerialConnection.h"

#include <QTimer>

/** @brief 将QSerialPort错误码翻译为详细的中文错误描述，含诊断信息和操作建议 @param error QSerialPort错误码 @return 人类可读的中文错误描述 */
QString SerialConnection::translateError(QSerialPort::SerialPortError error)
{
    // 先获取系统级的错误描述作为补充信息
    QString systemError = m_serial.errorString();

    switch (error) {
    case QSerialPort::DeviceNotFoundError:
        return tr("端口 %1 不存在，设备可能已断开连接，请刷新端口列表后重试")
            .arg(m_portName);

    case QSerialPort::PermissionError:
        return tr("端口 %1 权限不足或被其他程序占用。\n"
                  "请尝试: 1) 关闭其他串口工具 2) 以管理员身份运行本程序")
            .arg(m_portName);

    case QSerialPort::OpenError:
        return tr("无法打开端口 %1，端口可能已被其他程序占用。\n"
                  "系统错误: %2")
            .arg(m_portName, systemError);

    case QSerialPort::NotOpenError:
        return tr("端口 %1 尚未打开，请先建立连接").arg(m_portName);

    case QSerialPort::TimeoutError:
        return tr("端口 %1 通信超时，请检查设备是否正常响应").arg(m_portName);

    case QSerialPort::ResourceError:
        return tr("端口 %1 连接意外断开，设备可能已被拔出或驱动异常。\n"
                  "请检查设备连接后重新打开串口")
            .arg(m_portName);

    case QSerialPort::UnsupportedOperationError:
        return tr("端口 %1 不支持当前操作: %2").arg(m_portName, systemError);

    case QSerialPort::UnknownError:
        return tr("端口 %1 发生未知错误: %2").arg(m_portName, systemError);

    case QSerialPort::NoError:
        return tr("无错误");

    default:
        return tr("端口 %1 发生未识别的错误(代码:%2): %3")
            .arg(m_portName).arg(static_cast<int>(error)).arg(systemError);
    }
}

/** @brief QSerialPort::bytesWritten信号处理，转发为IConnection::bytesWritten信号 @param bytes 实际写入的字节数 */
void SerialConnection::onBytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}

/** @brief 发送Break信号，拉低TX线指定时间后恢复，用于触发bootloader升级模式 @param duration Break持续时间(毫秒)，默认100ms */
void SerialConnection::sendBreak(int duration)
{
    if (m_serial.isOpen()) {
        m_serial.setBreakEnabled(true);
        QTimer::singleShot(duration, this, [this]() {
            m_serial.setBreakEnabled(false);
        });
    }
}

/** @brief 重置会话级错误计数器(帧/校验/溢出/未知)，通常在串口重新打开时调用 */
void SerialConnection::resetErrorCounters()
{
    m_errorCounters = SerialErrorCounters{};
}

/** @brief 读取串口引脚信号状态(CTS/DSR/DCD/RI/RTS/DTR) @return PinoutSignals结构体 */
PinoutSignals SerialConnection::pinoutSignals() const
{
    PinoutSignals result;
    if (!m_serial.isOpen()) {
        return result;
    }
    QSerialPort::PinoutSignals qtSignals = const_cast<QSerialPort&>(m_serial).pinoutSignals();
    result.cts = qtSignals & QSerialPort::ClearToSendSignal;
    result.dsr = qtSignals & QSerialPort::DataSetReadySignal;
    result.dcd = qtSignals & QSerialPort::DataCarrierDetectSignal;
    result.ri  = qtSignals & QSerialPort::RingIndicatorSignal;
    result.dtr = qtSignals & QSerialPort::DataTerminalReadySignal;
    result.rts = qtSignals & QSerialPort::RequestToSendSignal;
    return result;
}

/** @brief 重置错误分类统计计数器(totalErrorsTracked/totalFramingErrors/totalParityErrors/totalOverrunErrors) */
void SerialConnection::resetErrorClassificationStats()
{
    m_totalErrorsTracked = 0;
    m_totalFramingErrors = 0;
    m_totalParityErrors = 0;
    m_totalOverrunErrors = 0;
}
