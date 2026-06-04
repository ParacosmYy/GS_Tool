/**
 * @file UsbDeviceDetectorDescriptor.cpp
 * @brief USB设备检测器 - 字符串描述符获取与设备信息转换
 *
 * 从 UsbDeviceDetector.cpp 拆分而来，包含:
 *   - fetchStringDescriptors: 通过libusb获取制造商/产品/序列号字符串
 *   - descriptorToMap: 设备描述符转换为QVariantMap
 */

#include "connection/usb/UsbDeviceDetector.h"
#include "connection/usb/UsbLibraryLoader.h"

/**
 * @brief 通过libusb获取指定设备的字符串描述符
 * 打开设备，读取iManufacturer/iProduct/iSerialNumber字符串
 * @param vid 厂商ID
 * @param pid 产品ID
 * @return 包含manufacturer/product/serial字段的映射
 */
QVariantMap UsbDeviceDetector::fetchStringDescriptors(quint16 vid,
                                                       quint16 pid) {
    QVariantMap result;
    result["manufacturer"] = QString();
    result["product"] = QString();
    result["serial"] = QString();

    auto& loader = UsbLibraryLoader::instance();
    if (!loader.isLoaded()) {
        if (!loader.load()) {
            return result;
        }
    }

    UsbContext* ctx = nullptr;
    if (loader.init(&ctx) != 0) {
        return result;
    }

    auto* handle = loader.openDeviceWithVidPid(ctx, vid, pid);
    if (!handle) {
        loader.exit(ctx);
        return result;
    }

    /* 先读取设备描述符获取字符串索引 */
    UsbDeviceDescriptor desc;
    if (loader.getDeviceDescriptor(handle, &desc) != 0) {
        loader.close(handle);
        loader.exit(ctx);
        return result;
    }

    char strBuf[256] = {0};

    if (desc.iManufacturer > 0 &&
        loader.getStringDescriptorAscii(handle, desc.iManufacturer,
                                       strBuf, sizeof(strBuf)) > 0) {
        result["manufacturer"] = QString::fromLocal8Bit(strBuf);
    }

    if (desc.iProduct > 0 &&
        loader.getStringDescriptorAscii(handle, desc.iProduct,
                                       strBuf, sizeof(strBuf)) > 0) {
        result["product"] = QString::fromLocal8Bit(strBuf);
    }

    if (desc.iSerialNumber > 0 &&
        loader.getStringDescriptorAscii(handle, desc.iSerialNumber,
                                       strBuf, sizeof(strBuf)) > 0) {
        result["serial"] = QString::fromLocal8Bit(strBuf);
    }

    loader.close(handle);
    loader.exit(ctx);
    return result;
}

/**
 * @brief 将设备描述符转换为QVariantMap
 * @param vid 厂商ID
 * @param pid 产品ID
 * @param desc 设备描述符结构体
 * @param manufacturer 制造商字符串
 * @param product 产品字符串
 * @param serial 序列号字符串
 * @return 包含完整设备信息的映射
 */
QVariantMap UsbDeviceDetector::descriptorToMap(
    quint16 vid, quint16 pid,
    const UsbDeviceDescriptor& desc,
    const QString& manufacturer,
    const QString& product,
    const QString& serial)
{
    QVariantMap dev;
    dev["vid"] = vid;
    dev["pid"] = pid;
    dev["vidHex"] = QString("%1").arg(vid, 4, 16, QChar('0')).toUpper();
    dev["pidHex"] = QString("%1").arg(pid, 4, 16, QChar('0')).toUpper();
    dev["bcdUSB"] = desc.bcdUSB;
    dev["deviceClass"] = desc.bDeviceClass;
    dev["deviceSubClass"] = desc.bDeviceSubClass;
    dev["deviceProtocol"] = desc.bDeviceProtocol;
    dev["bcdDevice"] = desc.bcdDevice;
    dev["numConfigurations"] = desc.bNumConfigurations;

    /* 名称优先使用libusb读取的产品字符串 */
    if (!product.isEmpty()) {
        dev["name"] = product;
    } else {
        dev["name"] = tr("USB设备 VID_%1 PID_%2")
                          .arg(dev["vidHex"].toString(),
                               dev["pidHex"].toString());
    }
    dev["manufacturer"] = manufacturer;
    dev["serial"] = serial;
    return dev;
}
