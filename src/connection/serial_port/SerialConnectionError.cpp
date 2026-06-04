/**
 * @file SerialConnectionError.cpp
 * @brief 串口错误处理 — 错误分类与恢复策略
 *
 * 从 SerialConnection.cpp 拆分而来，包含:
 *   - queryPlatformErrors(): Win32 ClearCommError() 底层错误分类
 *   - onError(): 错误恢复策略(致命断开/可恢复计数)
 *
 * 错误翻译、辅助方法见：@see SerialConnectionUtility.cpp
 */

#include "connection/serial_port/SerialConnection.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

/** @brief 通过平台API获取底层串口通信错误统计，Win32使用ClearCommError()读取帧/校验/溢出错误 */
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
            ++m_totalFramingErrors;  // 累计帧错误统计
            qWarning() << "Serial framing error detected on" << m_portName;
        }
        if (errors & CE_RXPARITY) {
            m_errorCounters.parityErrors++;
            ++m_totalParityErrors;  // 累计校验错误统计
            qWarning() << "Serial parity error detected on" << m_portName;
        }
        if (errors & CE_OVERRUN || errors & CE_RXOVER) {
            m_errorCounters.overrunErrors++;
            ++m_totalOverrunErrors;  // 累计溢出错误统计
            qWarning() << "Serial overrun error detected on" << m_portName;
        }
    }
#else
    // Linux: 未来可通过 TIOCGICOUNT ioctl 获取串口错误计数
    // 目前为空实现
#endif
}

/** @brief QSerialPort::errorOccurred信号处理，按致命/可恢复分类处理错误 @param error QSerialPort错误码 */
void SerialConnection::onError(QSerialPort::SerialPortError error)
{
    // 忽略无错误的情况 (Qt在某些操作后会触发NoError)
    if (error == QSerialPort::NoError) {
        return;
    }

    // 统计: 每次实际错误都计入总错误次数
    m_errorCount++;
    ++m_totalErrorsTracked;  // 累计已跟踪错误统计

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

// translateError/onBytesWritten/sendBreak/resetErrorCounters/pinoutSignals/resetErrorClassificationStats
// 已移至 SerialConnectionUtility.cpp
