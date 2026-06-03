/**
 * @file UsbLibraryLoader.cpp
 * @brief USB库动态加载器实现 — 运行时加载libusb
 *
 * 使用QLibrary动态加载libusb，解析所有函数指针。
 * 当libusb不可用时，所有调用返回安全默认值。
 */

#include "connection/usb/UsbLibraryLoader.h"

#include <QCoreApplication>
#include <QDir>
#include <QMutexLocker>

/* ---- 单例 ---- */

UsbLibraryLoader& UsbLibraryLoader::instance()
{
    static UsbLibraryLoader loader;
    return loader;
}

UsbLibraryLoader::UsbLibraryLoader(QObject* parent)
    : QObject(parent)
{
}

UsbLibraryLoader::~UsbLibraryLoader()
{
    unload();
}

/* ---- 加载/卸载 ---- */

bool UsbLibraryLoader::load(const QString& libraryPath)
{
    QMutexLocker locker(&m_mutex);

    if (m_loaded) { return true; }

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
            if (resolveFunctions()) {
                m_lastError.clear();
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

bool UsbLibraryLoader::isLoaded() const
{
    return m_loaded;
}

QString UsbLibraryLoader::lastError() const
{
    return m_lastError;
}

QString UsbLibraryLoader::versionString() const
{
    if (!m_loaded) { return tr("libusb未加载"); }
    return QStringLiteral("libusb-1.0 (loaded)");
}

/* ---- 函数指针包装器 ---- */

int UsbLibraryLoader::init(UsbContext** ctx)
{
    if (!m_fnInit) { setError(tr("libusb_init未解析")); return -1; }
    return m_fnInit(ctx);
}

void UsbLibraryLoader::exit(UsbContext* ctx)
{
    if (!m_fnExit) { return; }
    m_fnExit(ctx);
}

UsbDeviceHandle* UsbLibraryLoader::openDeviceWithVidPid(
    UsbContext* ctx, quint16 vid, quint16 pid)
{
    if (!m_fnOpen) { setError(tr("libusb_open_device_with_vid_pid未解析")); return nullptr; }
    return m_fnOpen(ctx, vid, pid);
}

void UsbLibraryLoader::close(UsbDeviceHandle* handle)
{
    if (!m_fnClose) { return; }
    m_fnClose(handle);
}

int UsbLibraryLoader::claimInterface(UsbDeviceHandle* handle, int interfaceNum)
{
    if (!m_fnClaim) { setError(tr("libusb_claim_interface未解析")); return -1; }
    return m_fnClaim(handle, interfaceNum);
}

int UsbLibraryLoader::releaseInterface(UsbDeviceHandle* handle, int interfaceNum)
{
    if (!m_fnRelease) { setError(tr("libusb_release_interface未解析")); return -1; }
    return m_fnRelease(handle, interfaceNum);
}

int UsbLibraryLoader::bulkTransfer(UsbDeviceHandle* handle,
                                    unsigned char endpoint,
                                    unsigned char* data, int length,
                                    int* transferred, unsigned int timeout)
{
    if (!m_fnBulk) { setError(tr("libusb_bulk_transfer未解析")); return -1; }
    return m_fnBulk(handle, endpoint, data, length, transferred, timeout);
}

int UsbLibraryLoader::interruptTransfer(UsbDeviceHandle* handle,
                                         unsigned char endpoint,
                                         unsigned char* data, int length,
                                         int* transferred, unsigned int timeout)
{
    if (!m_fnInterrupt) { setError(tr("libusb_interrupt_transfer未解析")); return -1; }
    return m_fnInterrupt(handle, endpoint, data, length, transferred, timeout);
}

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

int UsbLibraryLoader::getDeviceDescriptor(UsbDeviceHandle* handle,
                                           UsbDeviceDescriptor* desc)
{
    /* 完整实现: 先通过handle获取libusb_device，再读取描述符 */
    if (!m_loaded || !handle || !desc) { return -1; }

    void* device = getDevice(handle);
    if (!device) { return -1; }

    return getDeviceDescriptorFromDevice(device, desc);
}

void* UsbLibraryLoader::getDevice(UsbDeviceHandle* handle)
{
    if (!m_fnGetDevice) {
        setError(tr("libusb_get_device未解析"));
        return nullptr;
    }
    return m_fnGetDevice(handle);
}

int UsbLibraryLoader::getDeviceDescriptorFromDevice(void* device,
                                                     UsbDeviceDescriptor* desc)
{
    if (!m_fnGetDeviceDesc) {
        setError(tr("libusb_get_device_descriptor未解析"));
        return -1;
    }
    return m_fnGetDeviceDesc(device, desc);
}

int UsbLibraryLoader::getStringDescriptorAscii(UsbDeviceHandle* handle,
                                                quint8 descIndex,
                                                char* buffer, int bufferSize)
{
    if (!m_fnGetString) { return -1; }
    return m_fnGetString(handle, descIndex, buffer, bufferSize);
}

int UsbLibraryLoader::kernelDriverActive(UsbDeviceHandle* handle,
                                          int interfaceNum)
{
    /* Linux专用 — Windows上始终返回0 */
    return 0;
}

int UsbLibraryLoader::detachKernelDriver(UsbDeviceHandle* handle,
                                          int interfaceNum)
{
    /* Linux专用 — Windows上无操作 */
    return 0;
}

/* ---- 内部方法 ---- */

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

    /* 核心函数必须全部解析成功 */
    if (!m_fnInit || !m_fnExit || !m_fnOpen || !m_fnClose) {
        return false;
    }

    return true;
}

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

bool UsbLibraryLoader::setError(const QString& error)
{
    m_lastError = error;
    return false;
}
