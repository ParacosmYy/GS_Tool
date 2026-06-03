/**
 * @file UsbConnection.cpp
 * @brief USB连接实现 — 通过UsbLibraryLoader动态调用libusb
 *
 * 使用UsbLibraryLoader单例在运行时动态加载libusb共享库，
 * 替代编译时链接。所有libusb调用通过函数指针转发。
 * 当libusb不可用时，操作返回安全默认值并发出错误信号。
 */
#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

UsbConnection::UsbConnection(QObject* parent)
    : IConnection(parent)
{
}

UsbConnection::~UsbConnection() {
    close();
}

ConnectionType UsbConnection::type() const {
    return ConnectionType::Usb;
}

QString UsbConnection::name() const {
    return tr("USB:%1:%2").arg(m_vid, 4, 16, QChar('0'))
                       .arg(m_pid, 4, 16, QChar('0'));
}

ConnectionState UsbConnection::state() const {
    return m_state;
}

bool UsbConnection::open() {
    if (m_vid == 0 || m_pid == 0) {
        emit errorOccurred(tr("未设置USB设备VID/PID"));
        ++m_errorCount;
        return false;
    }

    auto& loader = UsbLibraryLoader::instance();

    /* 加载libusb共享库 */
    if (!loader.isLoaded()) {
        if (!loader.load()) {
            emit errorOccurred(tr("无法加载libusb: %1").arg(loader.lastError()));
            ++m_errorCount;
            return false;
        }
    }

    /* 初始化libusb上下文 */
    if (loader.init(&m_usbContext) != 0) {
        emit errorOccurred(tr("libusb初始化失败"));
        ++m_errorCount;
        return false;
    }

    /* 打开指定VID/PID的设备 */
    m_devHandle = loader.openDeviceWithVidPid(m_usbContext, m_vid, m_pid);
    if (!m_devHandle) {
        emit errorOccurred(tr("未找到USB设备 %1:%2")
                           .arg(m_vid, 4, 16, QChar('0'))
                           .arg(m_pid, 4, 16, QChar('0')));
        ++m_errorCount;
        loader.exit(m_usbContext);
        m_usbContext = nullptr;
        return false;
    }

    /* 声明接口 */
    if (loader.claimInterface(m_devHandle, m_interface) != 0) {
        emit errorOccurred(tr("无法声明USB接口 %1").arg(m_interface));
        ++m_errorCount;
        loader.close(m_devHandle);
        m_devHandle = nullptr;
        loader.exit(m_usbContext);
        m_usbContext = nullptr;
        return false;
    }

    m_interfaceClaimed = true;
    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

void UsbConnection::close() {
    if (m_state == ConnectionState::Connected) {
        auto& loader = UsbLibraryLoader::instance();

        /* 释放接口 */
        if (m_interfaceClaimed && m_devHandle) {
            loader.releaseInterface(m_devHandle, m_interface);
            m_interfaceClaimed = false;
        }

        /* 关闭设备 */
        if (m_devHandle) {
            loader.close(m_devHandle);
            m_devHandle = nullptr;
        }

        /* 释放上下文 */
        if (m_usbContext) {
            loader.exit(m_usbContext);
            m_usbContext = nullptr;
        }

        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

qint64 UsbConnection::write(const QByteArray& data) {
    if (m_state != ConnectionState::Connected) { return -1; }
    if (!m_devHandle) { return -1; }

    auto& loader = UsbLibraryLoader::instance();
    int transferred = 0;

    /* 使用默认OUT端点(0x01)进行bulk传输 */
    int result = loader.bulkTransfer(
        m_devHandle,
        0x01,  /* 默认bulk OUT端点 */
        reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
        data.size(),
        &transferred,
        m_timeout);

    if (result != 0) {
        emit errorOccurred(tr("USB写入失败: 错误码 %1").arg(result));
        ++m_errorCount;
        return -1;
    }

    ++m_totalTransfers;
    m_totalBytesSent += static_cast<quint64>(transferred);
    return transferred;
}

void UsbConnection::configure(const QVariantMap& params) {
    if (params.contains("vid")) {
        m_vid = static_cast<quint16>(params["vid"].toUInt());
    }
    if (params.contains("pid")) {
        m_pid = static_cast<quint16>(params["pid"].toUInt());
    }
    if (params.contains("interface")) {
        m_interface = params["interface"].toInt();
    }
    if (params.contains("timeout")) {
        m_timeout = static_cast<unsigned int>(params["timeout"].toUInt());
    }
}

bool UsbConnection::setDevice(quint16 vid, quint16 pid) {
    m_vid = vid;
    m_pid = pid;

    /* 检查libusb是否可用 */
    auto& loader = UsbLibraryLoader::instance();
    if (!loader.isLoaded()) {
        if (!loader.load()) {
            return false;
        }
    }

    /* 初始化临时上下文以探测设备 */
    UsbContext* probeCtx = nullptr;
    if (loader.init(&probeCtx) != 0) { return false; }

    auto* handle = loader.openDeviceWithVidPid(probeCtx, vid, pid);
    bool found = (handle != nullptr);

    if (handle) {
        loader.close(handle);
    }
    loader.exit(probeCtx);

    return found;
}

bool UsbConnection::claimInterface(int interface) {
    m_interface = interface;

    if (m_state != ConnectionState::Connected || !m_devHandle) {
        return false;
    }

    auto& loader = UsbLibraryLoader::instance();
    int result = loader.claimInterface(m_devHandle, interface);
    if (result == 0) {
        m_interfaceClaimed = true;
        return true;
    }

    emit errorOccurred(tr("声明USB接口 %1 失败: 错误码 %2")
                       .arg(interface).arg(result));
    ++m_errorCount;
    return false;
}

void UsbConnection::releaseInterface(int interface) {
    if (!m_interfaceClaimed || !m_devHandle) { return; }

    auto& loader = UsbLibraryLoader::instance();
    loader.releaseInterface(m_devHandle, interface);
    m_interfaceClaimed = false;
}

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
        m_totalBytesSent += static_cast<quint64>(transferred);
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
        m_totalBytesReceived += static_cast<quint64>(transferred);
        return buffer.left(transferred);
    }
}

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
        m_totalBytesSent += static_cast<quint64>(transferred);
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
        m_totalBytesReceived += static_cast<quint64>(transferred);
        return buffer.left(transferred);
    }
}

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
        int result = loader.controlTransfer(
            m_devHandle, requestType, request, value, index,
            reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
            static_cast<quint16>(data.size()),
            m_timeout);

        if (result < 0) {
            emit errorOccurred(tr("USB Control传输失败: 错误码 %1").arg(result));
            ++m_errorCount;
            return QByteArray();
        }

        ++m_totalTransfers;
        m_totalBytesSent += static_cast<quint64>(result);
        return data.left(result);
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
        m_totalBytesReceived += static_cast<quint64>(result);
        return buffer.left(result);
    }
}

/**
 * @brief 重置所有统计计数器
 */
void UsbConnection::resetStats()
{
    m_totalTransfers = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}
