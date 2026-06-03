/**
 * @file UsbLibraryLoader.h
 * @brief USB库动态加载器 — 运行时加载libusb DLL/SO
 *
 * 使用QLibrary在运行时动态加载libusb共享库，
 * 解析并缓存所有需要的函数指针。
 * 优势: 编译时不依赖libusb，仅在运行时需要时加载。
 *
 * 使用:
 *   auto& loader = UsbLibraryLoader::instance();
 *   if (loader.load()) {
 *       loader.init();
 *       auto* handle = loader.open_device_with_vid_pid(ctx, vid, pid);
 *   }
 */

#ifndef USBLIBRARYLOADER_H
#define USBLIBRARYLOADER_H

#include <QObject>
#include <QLibrary>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QMutex>

/**
 * @brief libusb上下文不透明指针类型
 */
using UsbContext = void;

/**
 * @brief libusb设备句柄不透明指针类型
 */
using UsbDeviceHandle = void;

/**
 * @brief libusb设备描述符结构体(简化版)
 */
struct UsbDeviceDescriptor {
    quint16 bcdUSB;           ///< USB规范版本
    quint8  bDeviceClass;     ///< 设备类
    quint8  bDeviceSubClass;  ///< 设备子类
    quint8  bDeviceProtocol;  ///< 设备协议
    quint16 idVendor;         ///< 厂商ID
    quint16 idProduct;        ///< 产品ID
    quint16 bcdDevice;        ///< 设备版本
    quint8  iManufacturer;    ///< 制造商字符串索引
    quint8  iProduct;         ///< 产品字符串索引
    quint8  iSerialNumber;    ///< 序列号索引
    quint8  bNumConfigurations; ///< 配置数量
};

/**
 * @brief USB库动态加载器
 *
 * 单例模式。在运行时动态加载libusb共享库并解析函数指针。
 * 支持Windows(libusb-1.0.dll)和Linux(libusb-1.0.so)。
 *
 * 当libusb不可用时，所有函数返回安全默认值(0/nullptr/false)，
 * 并设置错误信息，不会导致程序崩溃。
 */
class UsbLibraryLoader : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例 */
    static UsbLibraryLoader& instance();

    /** @brief 加载libusb共享库
     *  @param libraryPath 手动指定路径(空则自动搜索)
     *  @return true=加载成功
     */
    bool load(const QString& libraryPath = QString());

    /** @brief 卸载libusb共享库 */
    void unload();

    /** @brief 是否已加载 */
    bool isLoaded() const;

    /** @brief 获取最后错误信息 */
    QString lastError() const;

    /** @brief 获取libusb版本字符串 */
    QString versionString() const;

    // ---- libusb函数指针包装器 ----

    /** @brief libusb_init */
    int init(UsbContext** ctx = nullptr);

    /** @brief libusb_exit */
    void exit(UsbContext* ctx = nullptr);

    /** @brief libusb_open_device_with_vid_pid */
    UsbDeviceHandle* openDeviceWithVidPid(UsbContext* ctx, quint16 vid, quint16 pid);

    /** @brief libusb_close */
    void close(UsbDeviceHandle* handle);

    /** @brief libusb_claim_interface */
    int claimInterface(UsbDeviceHandle* handle, int interfaceNum);

    /** @brief libusb_release_interface */
    int releaseInterface(UsbDeviceHandle* handle, int interfaceNum);

    /** @brief libusb_bulk_transfer */
    int bulkTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                     unsigned char* data, int length, int* transferred,
                     unsigned int timeout);

    /** @brief libusb_interrupt_transfer */
    int interruptTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                          unsigned char* data, int length, int* transferred,
                          unsigned int timeout);

    /** @brief libusb_control_transfer */
    int controlTransfer(UsbDeviceHandle* handle, quint8 requestType,
                        quint8 request, quint16 value, quint16 index,
                        unsigned char* data, quint16 length,
                        unsigned int timeout);

    /** @brief libusb_get_device_descriptor */
    int getDeviceDescriptor(UsbDeviceHandle* handle,
                            UsbDeviceDescriptor* desc);

    /** @brief libusb_get_string_descriptor_ascii */
    int getStringDescriptorAscii(UsbDeviceHandle* handle,
                                 quint8 descIndex,
                                 char* buffer, int bufferSize);

    /** @brief libusb_kernel_driver_active */
    int kernelDriverActive(UsbDeviceHandle* handle, int interfaceNum);

    /** @brief libusb_detach_kernel_driver */
    int detachKernelDriver(UsbDeviceHandle* handle, int interfaceNum);

signals:
    /** @brief 库加载状态变更信号 */
    void loadStateChanged(bool loaded);

private:
    explicit UsbLibraryLoader(QObject* parent = nullptr);
    ~UsbLibraryLoader() override;
    UsbLibraryLoader(const UsbLibraryLoader&) = delete;
    UsbLibraryLoader& operator=(const UsbLibraryLoader&) = delete;

    /** @brief 解析所有函数指针 */
    bool resolveFunctions();

    /** @brief 搜索libusb共享库路径 */
    QStringList searchPaths() const;

    /** @brief 安全设置错误并返回false */
    bool setError(const QString& error);

    QLibrary* m_library = nullptr;       ///< 动态库句柄
    bool m_loaded = false;                ///< 是否已加载
    QString m_lastError;                  ///< 最后错误信息
    mutable QMutex m_mutex;               ///< 线程安全互斥

    // ---- libusb函数指针(解析后缓存) ----
    using FnInit = int(*)(UsbContext**);
    using FnExit = void(*)(UsbContext*);
    using FnOpen = UsbDeviceHandle*(*)(UsbContext*, quint16, quint16);
    using FnClose = void(*)(UsbDeviceHandle*);
    using FnClaim = int(*)(UsbDeviceHandle*, int);
    using FnRelease = int(*)(UsbDeviceHandle*, int);
    using FnBulk = int(*)(UsbDeviceHandle*, unsigned char, unsigned char*, int, int*, unsigned int);
    using FnInterrupt = int(*)(UsbDeviceHandle*, unsigned char, unsigned char*, int, int*, unsigned int);
    using FnControl = int(*)(UsbDeviceHandle*, quint8, quint8, quint16, quint16, unsigned char*, quint16, unsigned int);
    using FnGetString = int(*)(UsbDeviceHandle*, quint8, char*, int);

    FnInit       m_fnInit = nullptr;
    FnExit       m_fnExit = nullptr;
    FnOpen       m_fnOpen = nullptr;
    FnClose      m_fnClose = nullptr;
    FnClaim      m_fnClaim = nullptr;
    FnRelease    m_fnRelease = nullptr;
    FnBulk       m_fnBulk = nullptr;
    FnInterrupt  m_fnInterrupt = nullptr;
    FnControl    m_fnControl = nullptr;
    FnGetString  m_fnGetString = nullptr;
};

#endif // USBLIBRARYLOADER_H
