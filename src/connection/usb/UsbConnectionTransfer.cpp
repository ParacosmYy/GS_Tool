/**
 * @file UsbConnectionTransfer.cpp
 * @brief USB传输方法实现 — Bulk/Interrupt/Control传输及统计管理
 *
 * 本文件从UsbConnection.cpp拆分而来，包含三种USB传输模式的实现
 * (Bulk传输、Interrupt传输、Control传输)以及统计计数器的重置方法。
 * 传输方向由端点地址或请求类型的bit7自动判断，支持发送与接收。
 * 每种传输类型独立计数，接收操作通过dataReceived信号通知上层。
 *
 * @see UsbConnection.cpp — 核心连接生命周期管理(打开/关闭/配置)
 */
#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief 执行USB Bulk传输，根据端点方向自动判断收发 @param endpoint 端点地址(bit7决定方向) @param data 发送数据或接收缓冲区大小 @return 实际传输的数据，失败返回空QByteArray */
QByteArray UsbConnection::bulkTransfer(int endpoint,
                                        const QByteArray& data) {
    if (!m_devHandle) {
        ++m_errorCount;
        return QByteArray();
    }

    auto& loader = UsbLibraryLoader::instance();

    /* 根据端点方向决定发送或接收 */
    bool isOut = (endpoint & 0x80) == 0;
    int transferred = 0;

    if (isOut) {
        /* 发送数据 */
        int result = loader.bulkTransfer(
            m_devHandle,
            static_cast<unsigned char>(endpoint),
            reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
            data.size(),
            &transferred,
            m_timeout);

        if (result != 0) {
            emit errorOccurred(tr("USB Bulk传输失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_bulkTransferCount;
        m_totalBytesSent += static_cast<quint64>(transferred);
        emit bytesWritten(transferred);
        return data.left(transferred);
    } else {
        /* 接收数据 */
        QByteArray buffer(data.size() > 0 ? data.size() : 4096, '\0');
        int result = loader.bulkTransfer(
            m_devHandle,
            static_cast<unsigned char>(endpoint),
            reinterpret_cast<unsigned char*>(buffer.data()),
            buffer.size(),
            &transferred,
            m_timeout);

        if (result != 0) {
            emit errorOccurred(tr("USB Bulk接收失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_bulkTransferCount;
        m_totalBytesReceived += static_cast<quint64>(transferred);
        QByteArray received = buffer.left(transferred);
        emit dataReceived(received);
        return received;
    }
}

/** @brief 执行USB Interrupt传输，根据端点方向自动判断收发 @param endpoint 端点地址(bit7决定方向) @param data 发送数据或接收缓冲区大小 @return 实际传输的数据，失败返回空QByteArray */
QByteArray UsbConnection::interruptTransfer(int endpoint,
                                             const QByteArray& data) {
    if (!m_devHandle) {
        ++m_errorCount;
        return QByteArray();
    }

    auto& loader = UsbLibraryLoader::instance();
    bool isOut = (endpoint & 0x80) == 0;

    if (isOut) {
        int transferred = 0;
        int result = loader.interruptTransfer(
            m_devHandle,
            static_cast<unsigned char>(endpoint),
            reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
            data.size(),
            &transferred,
            m_timeout);

        if (result != 0) {
            emit errorOccurred(tr("USB Interrupt传输失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_interruptTransferCount;
        m_totalBytesSent += static_cast<quint64>(transferred);
        emit bytesWritten(transferred);
        return data.left(transferred);
    } else {
        QByteArray buffer(data.size() > 0 ? data.size() : 64, '\0');
        int transferred = 0;
        int result = loader.interruptTransfer(
            m_devHandle,
            static_cast<unsigned char>(endpoint),
            reinterpret_cast<unsigned char*>(buffer.data()),
            buffer.size(),
            &transferred,
            m_timeout);

        if (result != 0) {
            emit errorOccurred(tr("USB Interrupt接收失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_interruptTransferCount;
        m_totalBytesReceived += static_cast<quint64>(transferred);
        QByteArray received = buffer.left(transferred);
        emit dataReceived(received);
        return received;
    }
}

/** @brief 执行USB Control传输，根据requestType方向自动判断收发 @param requestType 请求类型字节(bit7决定方向) @param request 请求码 @param value wValue字段 @param index wIndex字段 @param data 发送数据或接收缓冲区 @return 实际传输的数据，失败返回空QByteArray */
QByteArray UsbConnection::controlTransfer(quint8 requestType,
                                           quint8 request,
                                           quint16 value,
                                           quint16 index,
                                           const QByteArray& data) {
    if (!m_devHandle) {
        ++m_errorCount;
        return QByteArray();
    }

    auto& loader = UsbLibraryLoader::instance();

    /* Control传输方向由requestType的bit7决定 */
    bool isOut = (requestType & 0x80) == 0;

    if (isOut) {
        /* 发送Control请求 */
        int dataLen = qMin(data.size(), 65535);
        int result = loader.controlTransfer(
            m_devHandle, requestType, request, value, index,
            reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
            static_cast<quint16>(dataLen),
            m_timeout);

        if (result < 0) {
            emit errorOccurred(tr("USB Control传输失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_controlTransferCount;
        int sent = qBound(0, result, data.size());
        m_totalBytesSent += static_cast<quint64>(sent);
        emit bytesWritten(sent);
        return data.left(sent);
    } else {
        /* 接收Control响应 */
        QByteArray buffer(256, '\0');
        int result = loader.controlTransfer(
            m_devHandle, requestType, request, value, index,
            reinterpret_cast<unsigned char*>(buffer.data()),
            static_cast<quint16>(buffer.size()),
            m_timeout);

        if (result < 0) {
            emit errorOccurred(tr("USB Control接收失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        ++m_controlTransferCount;
        int received = qBound(0, result, buffer.size());
        m_totalBytesReceived += static_cast<quint64>(received);
        QByteArray receivedData = buffer.left(received);
        emit dataReceived(receivedData);
        return receivedData;
    }
}

/** @brief 重置所有统计计数器，包括分类型传输计数、内核驱动分离计数、设备重置和打开次数 */
void UsbConnection::resetStats()
{
    m_totalTransfers = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_bulkTransferCount = 0;
    m_interruptTransferCount = 0;
    m_controlTransferCount = 0;
    m_kernelDetachCount = 0;
    m_totalDeviceResets = 0;
    m_totalOpenAttempts = 0;
}
