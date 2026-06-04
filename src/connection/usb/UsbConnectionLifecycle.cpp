/**
 * @file UsbConnectionLifecycle.cpp
 * @brief USB连接生命周期方法 — open/close/setDevice探测
 *
 * 从 UsbConnection.cpp 拆分而来，集中管理USB设备的打开、关闭和
 * 设备探测逻辑，与核心属性查询(write/configure)解耦。
 *
 * @see UsbConnection.cpp — 属性查询/配置/write
 * @see UsbConnectionTransfer.cpp — 传输方法
 * @see UsbConnectionDescriptors.cpp — 描述符读取
 */

#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief 打开USB连接，加载libusb、分离内核驱动、声明接口 @return 成功返回true */
bool UsbConnection::open() {
    ++m_totalOpenAttempts;  // 累计open()调用次数

    // 如果当前已连接，先关闭视为设备重置
    if (m_state == ConnectionState::Connected) {
        ++m_totalDeviceResets;  // 累计USB设备重置次数
        close();
    }

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
