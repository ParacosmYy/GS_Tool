/**
 * @file UsbLibraryLoader.cpp
 * @brief USB库动态加载器实现 — 运行时加载libusb
 *
 * 使用QLibrary动态加载libusb，解析所有函数指针。
 * 当libusb不可用时，所有调用返回安全默认值。
 */

#include "connection/usb/UsbLibraryLoader.h"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QElapsedTimer>

/* ---- 单例 ---- */

/** @brief 获取UsbLibraryLoader的单例引用 @return 全局唯一实例的引用 */
UsbLibraryLoader& UsbLibraryLoader::instance()
{
    static UsbLibraryLoader loader;
    return loader;
}

/** @brief 构造UsbLibraryLoader @param parent 父QObject指针 */
UsbLibraryLoader::UsbLibraryLoader(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构时卸载已加载的USB库 */
UsbLibraryLoader::~UsbLibraryLoader()
{
    unload();
}

/* ---- 加载/卸载 ---- */

/** @brief 动态加载libusb库并解析函数指针 @param libraryPath 库文件路径，为空时自动搜索 @return 加载成功返回true */
bool UsbLibraryLoader::load(const QString& libraryPath)
{
    QMutexLocker locker(&m_mutex);
    ++m_totalLoadAttempts;

    if (m_loaded) { return true; }

    QElapsedTimer loadTimer;
    loadTimer.start();

    /* 确定搜索路径列表 */
    QStringList paths;
    if (!libraryPath.isEmpty()) {
        paths.append(libraryPath);
    } else {
        paths = searchPaths();
    }

    /* 尝试每个路径 */
    for (const QString& path : paths) {
        m_library = new QLibrary(path, this);
        if (m_library->load()) {
            m_loaded = true;
            ++m_totalSuccessfulLoads;
            if (resolveFunctions()) {
                m_lastError.clear();
                m_totalLoadTimeMs += static_cast<qint64>(loadTimer.elapsed());
                emit loadStateChanged(true);
                return true;
            }
            /* 函数解析失败 — 版本不兼容 */
            m_library->unload();
            delete m_library;
            m_library = nullptr;
            m_loaded = false;
            return setError(tr("libusb函数解析失败: %1").arg(path));
        }
        delete m_library;
        m_library = nullptr;
    }

    return setError(tr("未找到libusb共享库"));
}

/** @brief 卸载已加载的USB库，清空所有函数指针 */
void UsbLibraryLoader::unload()
{
    QMutexLocker locker(&m_mutex);
    if (m_library) {
        m_library->unload();
        delete m_library;
        m_library = nullptr;
    }
    m_loaded = false;
    m_fnInit = nullptr;
    m_fnExit = nullptr;
    m_fnOpen = nullptr;
    m_fnClose = nullptr;
    m_fnClaim = nullptr;
    m_fnRelease = nullptr;
    m_fnBulk = nullptr;
    m_fnInterrupt = nullptr;
    m_fnControl = nullptr;
    m_fnGetString = nullptr;
    m_fnGetDevice = nullptr;
    m_fnGetDeviceDesc = nullptr;
    emit loadStateChanged(false);
}

/* ---- 函数指针包装器 ---- */

/** @brief 初始化libusb上下文 @param ctx 输出的libusb上下文指针 @return 0表示成功 */
int UsbLibraryLoader::init(UsbContext** ctx)
{
    if (!m_fnInit) { setError(tr("libusb_init未解析")); return -1; }
    return m_fnInit(ctx);
}

/** @brief 释放libusb上下文 @param ctx 要释放的libusb上下文指针 */
void UsbLibraryLoader::exit(UsbContext* ctx)
{
    if (!m_fnExit) { return; }
    m_fnExit(ctx);
}

/** @brief 通过VID/PID打开USB设备 @param ctx libusb上下文 @param vid 厂商ID @param pid 产品ID @return 设备句柄指针 */
UsbDeviceHandle* UsbLibraryLoader::openDeviceWithVidPid(
    UsbContext* ctx, quint16 vid, quint16 pid)
{
    if (!m_fnOpen) { setError(tr("libusb_open_device_with_vid_pid未解析")); return nullptr; }
    return m_fnOpen(ctx, vid, pid);
}

/** @brief 关闭USB设备句柄 @param handle 要关闭的设备句柄 */
void UsbLibraryLoader::close(UsbDeviceHandle* handle)
{
    if (!m_fnClose) { return; }
    m_fnClose(handle);
}

/** @brief 声明USB接口 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0表示成功 */
int UsbLibraryLoader::claimInterface(UsbDeviceHandle* handle, int interfaceNum)
{
    if (!m_fnClaim) { setError(tr("libusb_claim_interface未解析")); return -1; }
    return m_fnClaim(handle, interfaceNum);
}

/** @brief 释放USB接口 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0表示成功 */
int UsbLibraryLoader::releaseInterface(UsbDeviceHandle* handle, int interfaceNum)
{
    if (!m_fnRelease) { setError(tr("libusb_release_interface未解析")); return -1; }
    return m_fnRelease(handle, interfaceNum);
}

/** @brief 执行USB批量传输 @param handle 设备句柄 @param endpoint 端点地址 @param data 数据缓冲区 @param length 缓冲区长度 @param transferred 输出实际传输字节数 @param timeout 超时(ms) @return 0表示成功 */
int UsbLibraryLoader::bulkTransfer(UsbDeviceHandle* handle,
                                    unsigned char endpoint,
                                    unsigned char* data, int length,
                                    int* transferred, unsigned int timeout)
{
    if (!m_fnBulk) { setError(tr("libusb_bulk_transfer未解析")); return -1; }
    return m_fnBulk(handle, endpoint, data, length, transferred, timeout);
}

/** @brief 执行USB中断传输 @param handle 设备句柄 @param endpoint 端点地址 @param data 数据缓冲区 @param length 缓冲区长度 @param transferred 输出实际传输字节数 @param timeout 超时(ms) @return 0表示成功 */
int UsbLibraryLoader::interruptTransfer(UsbDeviceHandle* handle,
                                         unsigned char endpoint,
                                         unsigned char* data, int length,
                                         int* transferred, unsigned int timeout)
{
    if (!m_fnInterrupt) { setError(tr("libusb_interrupt_transfer未解析")); return -1; }
    return m_fnInterrupt(handle, endpoint, data, length, transferred, timeout);
}

/** @brief 执行USB控制传输 @param handle 设备句柄 @param requestType 请求类型 @param request 请求代码 @param value 值字段 @param index 索引字段 @param data 数据缓冲区 @param length 数据长度 @param timeout 超时(ms) @return 实际传输字节数 */
int UsbLibraryLoader::controlTransfer(UsbDeviceHandle* handle,
                                       quint8 requestType, quint8 request,
                                       quint16 value, quint16 index,
                                       unsigned char* data, quint16 length,
                                       unsigned int timeout)
{
    if (!m_fnControl) { setError(tr("libusb_control_transfer未解析")); return -1; }
    return m_fnControl(handle, requestType, request, value, index,
                       data, length, timeout);
}

/* ---- 内部方法 ---- */

/** @brief 设置错误信息并递增错误计数 @param error 错误描述 @return 始终返回false */
bool UsbLibraryLoader::setError(const QString& error)
{
    m_lastError = error;
    ++m_totalErrors;
    return false;
}

/** @brief 获取平均加载耗时(ms) @return 平均加载时间 */
double UsbLibraryLoader::avgLoadTimeMs() const
{
    if (m_totalSuccessfulLoads == 0) return 0.0;
    return static_cast<double>(m_totalLoadTimeMs) / static_cast<double>(m_totalSuccessfulLoads);
}

/** @brief 重置加载器统计计数器 */
void UsbLibraryLoader::resetLoaderStatistics()
{
    m_totalLoadAttempts = 0;
    m_totalSuccessfulLoads = 0;
    m_totalErrors = 0;
    m_totalFunctionResolutions = 0;
    m_totalLoadTimeMs = 0;
}
