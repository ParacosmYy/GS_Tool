/**
 * @file UsbConnectionInterface.cpp
 * @brief USB连接接口管理方法实现 — 接口声明/释放/内核驱动分离
 *
 * 从 UsbConnection.cpp 拆分而来，包含USB接口的声明、释放和
 * 内核驱动自动分离方法。
 */

#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

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
