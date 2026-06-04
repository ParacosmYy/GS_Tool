/**
 * @file UsbConnectionDescriptors.cpp
 * @brief USB描述符读取方法实现 — 设备描述符/字符串描述符/描述符摘要
 *
 * 本文件从UsbConnection.cpp拆分而来，包含USB设备描述符和字符串描述符的
 * 读取方法，以及将描述符信息汇总为可读映射的接口。
 *
 * @see UsbConnection.cpp — 核心连接生命周期管理(打开/关闭/配置)
 * @see UsbConnectionTransfer.cpp — 传输方法实现(Bulk/Interrupt/Control)
 */
#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/**
 * @brief 读取USB设备描述符
 * 通过UsbLibraryLoader读取18字节标准设备描述符
 * @return 设备描述符结构体，未连接或读取失败时bNumConfigurations为0
 */
UsbDeviceDescriptor UsbConnection::readDeviceDescriptor() {
    UsbDeviceDescriptor desc{};
    desc.bNumConfigurations = 0; /* 标记为无效 */

    if (!m_devHandle) {
        ++m_errorCount;
        return desc;
    }

    auto& loader = UsbLibraryLoader::instance();
    if (loader.getDeviceDescriptor(m_devHandle, &desc) != 0) {
        emit errorOccurred(tr("读取USB设备描述符失败"));
        ++m_errorCount;
        return desc;
    }

    return desc;
}

/**
 * @brief 读取USB字符串描述符
 * 通过libusb_get_string_descriptor_ascii读取人类可读的字符串
 * @param descIndex 字符串描述符索引(来自设备描述符的iManufacturer/iProduct/iSerialNumber)
 * @return 解码后的字符串，失败返回空
 */
QString UsbConnection::readStringDescriptor(quint8 descIndex) {
    if (!m_devHandle) {
        ++m_errorCount;
        return QString();
    }

    if (descIndex == 0) {
        return QString(); /* 索引0无字符串 */
    }

    auto& loader = UsbLibraryLoader::instance();
    char buffer[256] = {0};
    int result = loader.getStringDescriptorAscii(m_devHandle, descIndex,
                                                  buffer, sizeof(buffer));
    if (result <= 0) {
        return QString();
    }

    return QString::fromLocal8Bit(buffer, result);
}

/**
 * @brief 获取设备描述符的摘要信息
 * 读取描述符和字符串描述符，返回完整的设备信息映射
 * @return 描述符摘要映射 {bcdUSB, deviceClass, vid, pid, manufacturer, product, serial...}
 */
QVariantMap UsbConnection::deviceDescriptorSummary() {
    QVariantMap summary;

    UsbDeviceDescriptor desc = readDeviceDescriptor();
    if (desc.bNumConfigurations == 0) {
        /* 读取失败 */
        return summary;
    }

    summary["bcdUSB"] = desc.bcdUSB;
    summary["deviceClass"] = desc.bDeviceClass;
    summary["deviceSubClass"] = desc.bDeviceSubClass;
    summary["deviceProtocol"] = desc.bDeviceProtocol;
    summary["vid"] = desc.idVendor;
    summary["pid"] = desc.idProduct;
    summary["bcdDevice"] = desc.bcdDevice;
    summary["numConfigurations"] = desc.bNumConfigurations;

    /* 读取字符串描述符(制造商/产品名/序列号) */
    summary["manufacturer"] = readStringDescriptor(desc.iManufacturer);
    summary["product"] = readStringDescriptor(desc.iProduct);
    summary["serial"] = readStringDescriptor(desc.iSerialNumber);

    return summary;
}
