/**
 * @file SerialConnectionError.cpp
 * @brief 串口错误处理 — 错误分类、恢复策略和错误翻译
 *
 * 从 SerialConnection.cpp 拆分而来，包含:
 *   - queryPlatformErrors(): Win32 ClearCommError() 底层错误分类
 *   - onError(): 错误恢复策略(致命断开/可恢复计数)
 *   - translateError(): QSerialPort错误码→中文描述翻译
 *   - onBytesWritten(): 写入完成信号转发
 *   - sendBreak(): Break信号发送
 *   - resetErrorCounters(): 错误计数器重置
 *   - pinoutSignals(): 引脚信号状态查询
 */

#include "connection/SerialConnection.h"
#include <QDebug>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

/**
 * @brief 通过平台API获取底层串口通信错误统计
 *
 * Windows平台使用ClearCommError()读取COMSTAT结构中的错误标志:
 *   - CE_FRAME: 帧错误(起始位/停止位不匹配)
 *   - CE_RXPARITY: 硬件奇偶校验错误
 *   - CE_OVERRUN/CE_RXOVER: 接收缓冲区溢出
 * 非Windows平台为空实现，未来可扩展Linux的TIOCGICOUNT ioctl。
 */
void SerialConnection::queryPlatformErrors()
{
#ifdef Q_OS_WIN
    if (!m_serial.isOpen()) return;

    // 获取Win32底层串口句柄
    HANDLE hComm = reinterpret_cast<HANDLE>(m_serial.handle());
    if (hComm == INVALID_HANDLE_VALUE) return;

    DWORD errors = 0;
    COMSTAT comStat = {};

    // ClearCommError()清除错误标志并返回当前通信状态
    if (ClearCommError(hComm, &errors, &comStat)) {
        if (errors & CE_FRAME) {
            m_errorCounters.framingErrors++;
            qWarning() << "Serial framing error detected on" << m_portName;
        }
        if (errors & CE_RXPARITY) {
            m_errorCounters.parityErrors++;
            qWarning() << "Serial parity error detected on" << m_portName;
        }
        if (errors & CE_OVERRUN || errors & CE_RXOVER) {
            m_errorCounters.overrunErrors++;
            qWarning() << "Serial overrun error detected on" << m_portName;
        }
    }
#else
    // Linux: 未来可通过 TIOCGICOUNT ioctl 获取串口错误计数
    // 目前为空实现
#endif
}

/**
 * @brief QSerialPort::errorOccurred 信号处理
 *
 * 错误分类与恢复策略:
 *   - 致命错误(ResourceError/PermissionError/OpenError/DeviceNotFoundError):
 *     将状态设为Error并关闭端口，需要用户手动重连
 *   - 可恢复错误(TimeoutError/ReadError/WriteError等):
 *     保持连接状态不变，通过queryPlatformErrors()查询底层错误分类，
 *     仅更新错误计数器，不触发断开
 *
 * @param error QSerialPort 的错误码
 */
void SerialConnection::onError(QSerialPort::SerialPortError error)
{
    // 忽略无错误的情况 (Qt在某些操作后会触发NoError)
    if (error == QSerialPort::NoError) {
        return;
    }

    // 翻译错误为详细的中文描述
    QString errorMsg = translateError(error);
    qWarning() << "Serial error on" << m_portName << ":" << error << errorMsg;

    // 查询平台底层错误分类(帧错误/校验错误/溢出错误)
    queryPlatformErrors();

    // 分类: 可恢复 vs 致命
    bool isFatal = false;
    switch (error) {
    case QSerialPort::ResourceError:
        // 设备物理断开 — 致命
        isFatal = true;
        break;
    case QSerialPort::PermissionError:
        // 权限丢失 — 致命
        isFatal = true;
        break;
    case QSerialPort::OpenError:
    case QSerialPort::DeviceNotFoundError:
        // 打开/设备错误 — 致命
        isFatal = true;
        break;
    case QSerialPort::TimeoutError:
        // 超时 — 可恢复，仅计数
        m_errorCounters.unknownErrors++;
        break;
    case QSerialPort::ReadError:
    case QSerialPort::WriteError:
        // 读写错误 — 可恢复(可能是瞬态帧错误)
        // queryPlatformErrors()已将底层分类到具体计数器
        if (m_errorCounters.framingErrors == 0 &&
            m_errorCounters.parityErrors == 0 &&
            m_errorCounters.overrunErrors == 0) {
            m_errorCounters.unknownErrors++;
        }
        break;
    default:
        // 其他错误 — 计数但不一定断开
        m_errorCounters.unknownErrors++;
        break;
    }

    if (isFatal) {
        m_state = ConnectionState::Error;
        if (error == QSerialPort::ResourceError && m_serial.isOpen()) {
            m_serial.close();
        }
        emit stateChanged(m_state);
    }
    // 可恢复错误: 保持连接状态不变，仅通知上层
    emit errorOccurred(errorMsg);
    emit errorCountersUpdated(m_errorCounters);
}

/**
 * @brief 将 QSerialPort 错误码翻译为详细的中文错误描述
 *
 * 针对每种错误类型给出具体的诊断信息和操作建议:
 *   - DeviceNotFoundError: 端口不存在，检查设备连接
 *   - PermissionError: 权限不足，建议管理员运行或关闭占用程序
 *   - OpenError: 端口被占用或打开失败
 *   - NotOpenError: 操作在未打开时执行
 *   - TimeoutError: 读写超时
 *   - ResourceError: 资源意外释放（设备被拔出等）
 *   - UnsupportedOperationError: 不支持的操作
 *   - UnknownError: 未知错误，附带系统错误信息
 *
 * @param error QSerialPort 错误码
 * @return 人类可读的中文错误描述
 */
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
        // 资源错误通常意味着设备被意外拔出或驱动崩溃
        return tr("端口 %1 连接意外断开，设备可能已被拔出或驱动异常。\n"
                  "请检查设备连接后重新打开串口")
            .arg(m_portName);

    case QSerialPort::UnsupportedOperationError:
        return tr("端口 %1 不支持当前操作: %2").arg(m_portName, systemError);

    case QSerialPort::UnknownError:
        return tr("端口 %1 发生未知错误: %2").arg(m_portName, systemError);

    case QSerialPort::NoError:
        // 理论上不会被调用（调用方已过滤），但保持完整性
        return tr("无错误");

    default:
        return tr("端口 %1 发生未识别的错误(代码:%2): %3")
            .arg(m_portName).arg(static_cast<int>(error)).arg(systemError);
    }
}

/**
 * @brief QSerialPort::bytesWritten 信号处理
 *
 * 将 QSerialPort 的写入完成事件转发为 IConnection::bytesWritten 信号，
 * 上层模块可通过此信号跟踪实际写入的字节数（如OTA进度追踪）。
 *
 * @param bytes 实际写入的字节数
 */
void SerialConnection::onBytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}

/**
 * @brief 发送Break信号
 *
 * 部分嵌入式设备的bootloader需要通过串口Break信号触发升级模式。
 * 实现方式: 拉低TX线指定时间后恢复，模拟标准Break条件。
 *
 * @param duration Break持续时间(毫秒)，默认100ms
 */
void SerialConnection::sendBreak(int duration)
{
    if (m_serial.isOpen()) {
        m_serial.setBreakEnabled(true);
        QTimer::singleShot(duration, this, [this]() {
            m_serial.setBreakEnabled(false);
        });
    }
}

/**
 * @brief 重置错误计数器
 *
 * 将所有错误计数归零。通常在串口重新打开时自动调用。
 */
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
