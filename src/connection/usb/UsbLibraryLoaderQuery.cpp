/**
 * @file UsbLibraryLoaderQuery.cpp
 * @brief USB库查询方法实现 — 库状态查询、设备描述符获取、函数解析
 *
 * 从UsbLibraryLoader.cpp中提取的查询/查找/枚举相关方法:
 * - 库加载状态查询 (isLoaded / lastError / versionString)
 * - USB设备描述符获取 (getDevice / getDeviceDescriptor / getDeviceDescriptorFromDevice)
 * - USB字符串描述符获取 (getStringDescriptorAscii)
 * - 内核驱动状态查询 (kernelDriverActive / detachKernelDriver)
 * - 库搜索路径生成 (searchPaths)
 * - 函数指针解析 (resolveFunctions)
 */

#include "connection/usb/UsbLibraryLoader.h"

#include <QCoreApplication>
#include <QDir>

/* ---- 状态查询 ---- */

/** @brief 检查USB库是否已成功加载 @return 已加载返回true */
bool UsbLibraryLoader::isLoaded() const
{
    return m_loaded;
}

/** @brief 获取最后一次错误信息 @return 错误描述字符串 */
QString UsbLibraryLoader::lastError() const
{
    return m_lastError;
}

/** @brief 获取已加载库的版本描述字符串 @return 版本信息(如"libusb 1.0.26.11574") */
QString UsbLibraryLoader::versionString() const
{
    if (!m_loaded) { return tr("libusb未加载"); }
    if (!m_fnGetVersion) { return tr("libusb (版本查询不可用)"); }

    /* 通过libusb_get_version读取真实版本号 */
    const UsbVersion* ver = m_fnGetVersion();
    if (!ver) { return tr("libusb (版本信息为空)"); }

    return tr("libusb %1.%2.%3.%4")
        .arg(ver->major)
        .arg(ver->minor)
        .arg(ver->micro)
        .arg(ver->nano);
}

/* ---- 设备描述符查询 ---- */

/** @brief 通过设备句柄获取设备描述符 @param handle 设备句柄 @param desc 输出的设备描述符 @return 0表示成功 */
int UsbLibraryLoader::getDeviceDescriptor(UsbDeviceHandle* handle,
                                           UsbDeviceDescriptor* desc)
{
    /* 完整实现: 先通过handle获取libusb_device，再读取描述符 */
    if (!m_loaded || !handle || !desc) { return -1; }

    void* device = getDevice(handle);
    if (!device) { return -1; }

    return getDeviceDescriptorFromDevice(device, desc);
}

/** @brief 从设备句柄获取底层libusb_device指针 @param handle 设备句柄 @return libusb_device指针 */
void* UsbLibraryLoader::getDevice(UsbDeviceHandle* handle)
{
    if (!m_fnGetDevice) {
        setError(tr("libusb_get_device未解析"));
        return nullptr;
    }
    return m_fnGetDevice(handle);
}

/** @brief 从libusb_device指针获取设备描述符 @param device 设备指针 @param desc 输出的设备描述符 @return 0表示成功 */
int UsbLibraryLoader::getDeviceDescriptorFromDevice(void* device,
                                                     UsbDeviceDescriptor* desc)
{
    if (!m_fnGetDeviceDesc) {
        setError(tr("libusb_get_device_descriptor未解析"));
        return -1;
    }
    return m_fnGetDeviceDesc(device, desc);
}

/** @brief 获取USB字符串描述符(ASCII编码) @param handle 设备句柄 @param descIndex 描述符索引 @param buffer 输出缓冲区 @param bufferSize 缓冲区大小 @return 实际写入字节数 */
int UsbLibraryLoader::getStringDescriptorAscii(UsbDeviceHandle* handle,
                                                quint8 descIndex,
                                                char* buffer, int bufferSize)
{
    if (!m_fnGetString) { return -1; }
    return m_fnGetString(handle, descIndex, buffer, bufferSize);
}

/** @brief 检查内核驱动是否占用了指定接口 @param handle 设备句柄 @param interfaceNum 接口编号 @return 1=内核驱动活跃, 0=不活跃, 负数=错误 */
int UsbLibraryLoader::kernelDriverActive(UsbDeviceHandle* handle,
                                          int interfaceNum)
{
    if (!m_fnKernelDriverActive) {
        /* 函数指针未解析 — 平台不支持或库版本过旧 */
        return 0;
    }
    return m_fnKernelDriverActive(handle, interfaceNum);
}

/** @brief 从接口上分离内核驱动 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0=成功, 负数=错误 */
int UsbLibraryLoader::detachKernelDriver(UsbDeviceHandle* handle,
                                          int interfaceNum)
{
    if (!m_fnDetachKernelDriver) {
        /* 函数指针未解析 — 平台不支持或库版本过旧 */
        return 0;
    }
    return m_fnDetachKernelDriver(handle, interfaceNum);
}

/* ---- 内部: 函数解析 ---- */

/** @brief 解析libusb库中所有函数指针 @return 核心函数全部解析成功返回true */
bool UsbLibraryLoader::resolveFunctions()
{
    if (!m_library || !m_library->isLoaded()) { return false; }

    /* 解析核心函数 */
    m_fnInit = reinterpret_cast<FnInit>(m_library->resolve("libusb_init"));
    m_fnExit = reinterpret_cast<FnExit>(m_library->resolve("libusb_exit"));
    m_fnOpen = reinterpret_cast<FnOpen>(
        m_library->resolve("libusb_open_device_with_vid_pid"));
    m_fnClose = reinterpret_cast<FnClose>(m_library->resolve("libusb_close"));
    m_fnClaim = reinterpret_cast<FnClaim>(m_library->resolve("libusb_claim_interface"));
    m_fnRelease = reinterpret_cast<FnRelease>(
        m_library->resolve("libusb_release_interface"));
    m_fnBulk = reinterpret_cast<FnBulk>(m_library->resolve("libusb_bulk_transfer"));
    m_fnInterrupt = reinterpret_cast<FnInterrupt>(
        m_library->resolve("libusb_interrupt_transfer"));
    m_fnControl = reinterpret_cast<FnControl>(
        m_library->resolve("libusb_control_transfer"));
    m_fnGetString = reinterpret_cast<FnGetString>(
        m_library->resolve("libusb_get_string_descriptor_ascii"));
    m_fnGetDevice = reinterpret_cast<FnGetDevice>(
        m_library->resolve("libusb_get_device"));
    m_fnGetDeviceDesc = reinterpret_cast<FnGetDeviceDesc>(
        m_library->resolve("libusb_get_device_descriptor"));

    /* 解析扩展函数(可选，不影响核心加载) */
    m_fnGetVersion = reinterpret_cast<FnGetVersion>(
        m_library->resolve("libusb_get_version"));
    m_fnKernelDriverActive = reinterpret_cast<FnKernelDriverActive>(
        m_library->resolve("libusb_kernel_driver_active"));
    m_fnDetachKernelDriver = reinterpret_cast<FnDetachKernelDriver>(
        m_library->resolve("libusb_detach_kernel_driver"));

    /* 累计函数符号解析次数(15个符号: 12核心 + 3扩展) */
    m_totalFunctionResolutions += 15;

    /* 核心函数必须全部解析成功 */
    if (!m_fnInit || !m_fnExit || !m_fnOpen || !m_fnClose) {
        return false;
    }

    return true;
}

/** @brief 生成libusb共享库搜索路径列表 @return 按优先级排列的候选路径 */
QStringList UsbLibraryLoader::searchPaths() const
{
    QStringList paths;

#ifdef Q_OS_WIN
    /* Windows: 搜索应用程序目录和系统目录 */
    QString appDir = QCoreApplication::applicationDirPath();

    /* 应用目录下的libusb-1.0.dll */
    paths.append(appDir + "/libusb-1.0.dll");
    paths.append(appDir + "/libusb-1.0.dllx");

    /* 应用目录子目录 */
    paths.append(appDir + "/drivers/libusb-1.0.dll");
    paths.append(appDir + "/usb/libusb-1.0.dll");

    /* 尝试不带路径，依赖系统PATH */
    paths.append("libusb-1.0.dll");
#else
    /* Linux/macOS */
    paths.append("libusb-1.0.so");
    paths.append("libusb-1.0.so.0");
    paths.append("/usr/lib/x86_64-linux-gnu/libusb-1.0.so");
    paths.append("/usr/local/lib/libusb-1.0.so");
    paths.append("/usr/lib/libusb-1.0.so");
#endif

    return paths;
}
