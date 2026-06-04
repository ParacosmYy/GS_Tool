/**
 * @file UsbDeviceDetector.cpp
 * @brief USB设备检测器实现
 *
 * 双通道设备扫描:
 * 1. libusb方式 — 通过UsbLibraryLoader动态加载libusb，读取设备描述符
 *    和字符串描述符(制造商/产品/序列号)，信息最完整
 * 2. WMIC方式 — 通过Windows WMIC命令枚举USB设备，不依赖libusb，
 *    可获取设备名和制造商，但无法获取序列号
 * 优先使用libusb，不可用时自动回退到WMIC。
 */
#include "connection/usb/UsbDeviceDetector.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief 构造USB设备检测器，初始化轮询定时器并检测libusb可用性 @param parent 父QObject指针 */
UsbDeviceDetector::UsbDeviceDetector(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout,
            this, &UsbDeviceDetector::onPollTimeout);

    /* 检测libusb是否可用(不强制加载) */
    auto& loader = UsbLibraryLoader::instance();
    m_libusbAvailable = loader.isLoaded();
    if (!m_libusbAvailable) {
        m_libusbAvailable = loader.load();
    }
}

/**
 * @brief 扫描当前所有USB设备
 * 优先使用libusb扫描(信息更丰富)，不可用时回退到WMIC
 * @return 设备信息列表
 */
QVariantList UsbDeviceDetector::scanDevices() {
    QVariantList devices;

    /* 优先尝试libusb扫描 */
    if (m_libusbAvailable) {
        devices = scanDevicesViaLibusb();
        if (!devices.isEmpty()) {
            m_devices = devices;
            m_totalDevicesDetected += static_cast<quint64>(devices.size());
            emit scanCompleted(devices, QStringLiteral("libusb"));
            return devices;
        }
    }

    /* libusb不可用或未发现设备时回退到WMIC */
    devices = scanDevicesViaWmic();
    m_devices = devices;
    m_totalDevicesDetected += static_cast<quint64>(devices.size());
    emit scanCompleted(devices, QStringLiteral("wmic"));
    return devices;
}

/**
 * @brief 使用libusb扫描USB设备并读取完整描述符信息
 * 初始化临时libusb上下文，遍历设备列表，读取设备描述符和字符串描述符
 * @return 设备信息列表
 */
QVariantList UsbDeviceDetector::scanDevicesViaLibusb() {
    QVariantList devices;
    ++m_totalLibusbScans;

    auto& loader = UsbLibraryLoader::instance();
    if (!loader.isLoaded()) {
        return devices;
    }

    /* 初始化临时上下文 */
    UsbContext* ctx = nullptr;
    if (loader.init(&ctx) != 0) {
        return devices;
    }

    /* 通过已知VID/PID范围尝试探测常见设备 */
    /* 使用getDeviceDescriptor逐个尝试打开已知设备，收集信息 */
    /* 由于libusb未提供直接枚举所有设备的包装器，
     * 这里采用辅助策略: 先用WMIC获取VID/PID列表，
     * 再用libusb打开每个设备获取详细描述符 */

    /* 如果WMIC可用，先用它获取原始VID/PID列表 */
    QVariantList wmicList = scanDevicesViaWmic();

    for (const QVariant& var : wmicList) {
        QVariantMap wmicDev = var.toMap();
        quint16 vid = static_cast<quint16>(wmicDev["vid"].toUInt());
        quint16 pid = static_cast<quint16>(wmicDev["pid"].toUInt());

        /* 尝试用libusb打开设备获取详细信息 */
        auto* handle = loader.openDeviceWithVidPid(ctx, vid, pid);
        if (!handle) {
            /* libusb无法打开(权限/驱动占用)，保留WMIC信息 */
            devices.append(wmicDev);
            continue;
        }

        /* 读取设备描述符 */
        UsbDeviceDescriptor desc;

        if (loader.getDeviceDescriptor(handle, &desc) != 0) {
            /* 描述符读取失败，使用WMIC数据 */
            loader.close(handle);
            devices.append(wmicDev);
            continue;
        }

        /* 读取字符串描述符 */
        char strBuf[256] = {0};
        QString manufacturer;
        if (desc.iManufacturer > 0 &&
            loader.getStringDescriptorAscii(handle, desc.iManufacturer,
                                           strBuf, sizeof(strBuf)) > 0) {
            manufacturer = QString::fromLocal8Bit(strBuf);
        }

        QString product;
        if (desc.iProduct > 0 &&
            loader.getStringDescriptorAscii(handle, desc.iProduct,
                                           strBuf, sizeof(strBuf)) > 0) {
            product = QString::fromLocal8Bit(strBuf);
        }

        QString serial;
        if (desc.iSerialNumber > 0 &&
            loader.getStringDescriptorAscii(handle, desc.iSerialNumber,
                                           strBuf, sizeof(strBuf)) > 0) {
            serial = QString::fromLocal8Bit(strBuf);
        }

        QVariantMap devMap = descriptorToMap(vid, pid, desc,
                                             manufacturer, product, serial);
        devices.append(devMap);
        loader.close(handle);
    }

    loader.exit(ctx);
    return devices;
}


// ---- scanDevicesViaWmic() 已拆分至 UsbDeviceDetectorWmic.cpp ----
// ---- fetchStringDescriptors/descriptorToMap已拆分至 UsbDeviceDetectorDescriptor.cpp ----

/** @brief 启动设备变化监控轮询 @param intervalMs 轮询间隔(毫秒) */
void UsbDeviceDetector::startMonitoring(int intervalMs) {
    m_devices = scanDevices();
    m_pollTimer->start(intervalMs);
}

/** @brief 停止设备变化监控轮询 */
void UsbDeviceDetector::stopMonitoring() {
    m_pollTimer->stop();
}

/** @brief 轮询定时器超时回调，执行一次扫描并检测设备变化 */
void UsbDeviceDetector::onPollTimeout() {
    ++m_totalDetectionCycles;
    QVariantList newDevices = scanDevices();
    detectChanges(newDevices);
}

// ---- 统计查询方法已拆分至 UsbDeviceDetectorStats.cpp ----
