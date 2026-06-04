/**
 * @file UsbConnection.cpp
 * @brief USB连接核心生命周期 — 打开/关闭/配置/探测/描述符读取
 *
 * 通过UsbLibraryLoader单例动态加载libusb，实现USB设备的连接管理。
 * 支持内核驱动自动分离(Linux)、设备描述符和字符串描述符读取。
 *
 * 传输方法(Bulk/Interrupt/Control)实现在 UsbConnectionTransfer.cpp 中。
 * 描述符读取方法实现在 UsbConnectionDescriptors.cpp 中。
 *
 * @see UsbConnectionTransfer.cpp — 传输方法实现
 * @see UsbConnectionDescriptors.cpp — 描述符读取方法实现
 */
#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief 构造函数，初始化USB连接基类 @param parent 父对象指针 */
UsbConnection::UsbConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，关闭连接并释放USB资源 */
UsbConnection::~UsbConnection() {
    close();
}

/** @brief 获取连接类型 @return 固定返回ConnectionType::Usb */
ConnectionType UsbConnection::type() const {
    return ConnectionType::Usb;
}

/** @brief 获取连接显示名称，格式为USB:VID:PID @return 十六进制格式的VID:PID字符串 */
QString UsbConnection::name() const {
    return tr("USB:%1:%2").arg(m_vid, 4, 16, QChar('0'))
                       .arg(m_pid, 4, 16, QChar('0'));
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState UsbConnection::state() const {
    return m_state;
}

/** @brief 打开USB连接，加载libusb、分离内核驱动、声明接口 @return 成功返回true */
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

    /* 自动分离内核驱动(Linux有效，Windows无操作) */
    detachKernelDriverIfNeeded(m_interface);

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

/** @brief 关闭USB连接，释放接口、恢复内核驱动、关闭设备、释放libusb上下文 */
void UsbConnection::close() {
    if (m_state == ConnectionState::Connected) {
        auto& loader = UsbLibraryLoader::instance();

        /* 释放接口 */
        if (m_interfaceClaimed && m_devHandle) {
            loader.releaseInterface(m_devHandle, m_interface);
            m_interfaceClaimed = false;
        }

        /* 内核驱动在releaseInterface后会自动重新绑定 */
        m_kernelDriverDetached = false;

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

/** @brief 通过默认bulk OUT端点写入数据 @param data 待发送的字节数据 @return 实际传输字节数，失败返回-1 */
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
    ++m_bulkTransferCount;
    m_totalBytesSent += static_cast<quint64>(transferred);
    emit bytesWritten(transferred);
    return transferred;
}

/** @brief 通过参数映射配置USB连接属性(VID/PID/接口/超时) @param params 配置参数键值对 */
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

/** @brief 设置目标USB设备的VID和PID，并探测设备是否存在 @param vid 厂商ID @param pid 产品ID @return 设备存在且libusb可用返回true */
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

/** @brief 声明指定USB接口以便独占使用，自动处理内核驱动分离 @param interface 接口号 @return 声明成功返回true */
bool UsbConnection::claimInterface(int interface) {
    m_interface = interface;

    if (m_state != ConnectionState::Connected || !m_devHandle) {
        return false;
    }

    /* 先尝试分离内核驱动 */
    detachKernelDriverIfNeeded(interface);

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

/** @brief 释放已声明的USB接口 @param interface 接口号 */
void UsbConnection::releaseInterface(int interface) {
    if (!m_interfaceClaimed || !m_devHandle) { return; }

    auto& loader = UsbLibraryLoader::instance();
    loader.releaseInterface(m_devHandle, interface);
    m_interfaceClaimed = false;
}

// ============================================================
// 描述符读取方法 — 已拆分至 UsbConnectionDescriptors.cpp
// ============================================================

/**
 * @brief 检查并分离内核驱动(Linux专用)
 * Windows上kernelDriverActive始终返回0，此函数为无操作。
 * Linux上如果内核驱动(如usbserial/cdc_acm)占用接口，
 * 需要先分离才能claimInterface成功。
 * @param interfaceNum 接口编号
 * @return 成功分离或无需分离返回true
 */
bool UsbConnection::detachKernelDriverIfNeeded(int interfaceNum) {
    auto& loader = UsbLibraryLoader::instance();
    if (!m_devHandle) { return false; }

    /* 检查内核驱动是否活跃 */
    int active = loader.kernelDriverActive(m_devHandle, interfaceNum);
    if (active == 1) {
        /* 内核驱动活跃，尝试分离 */
        int result = loader.detachKernelDriver(m_devHandle, interfaceNum);
        if (result == 0) {
            m_kernelDriverDetached = true;
            ++m_kernelDetachCount;
            return true;
        }
        /* 分离失败 */
        return false;
    }

    /* 内核驱动不活跃(Windows或Linux无驱动占用) */
    return true;
}
