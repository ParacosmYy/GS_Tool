/** @file UsbLibraryLoader.h @brief USB库动态加载器(单例) — 运行时加载libusb DLL/SO，解析函数指针 */
#ifndef USBLIBRARYLOADER_H
#define USBLIBRARYLOADER_H

#include <QObject>
#include <QLibrary>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QMutex>

using UsbContext = void;       ///< libusb上下文不透明指针
using UsbDeviceHandle = void;  ///< libusb设备句柄不透明指针

/// @brief libusb版本信息结构体(对应libusb_version)
struct UsbVersion {
    const quint16 major;    ///< 主版本号
    const quint16 minor;    ///< 次版本号
    const quint16 micro;    ///< 修订号
    const quint16 nano;     ///< 构建号
    const char* rc;         ///< 候选版本字符串
    const char* describe;   ///< 完整版本描述字符串
};

/// @brief libusb设备描述符(简化版)
struct UsbDeviceDescriptor {
    quint16 bcdUSB;            ///< USB规范版本
    quint8  bDeviceClass;      ///< 设备类
    quint8  bDeviceSubClass;   ///< 设备子类
    quint8  bDeviceProtocol;   ///< 设备协议
    quint16 idVendor;          ///< 厂商ID
    quint16 idProduct;         ///< 产品ID
    quint16 bcdDevice;         ///< 设备版本
    quint8  iManufacturer;     ///< 制造商字符串索引
    quint8  iProduct;          ///< 产品字符串索引
    quint8  iSerialNumber;     ///< 序列号索引
    quint8  bNumConfigurations;///< 配置数量
};

/** @brief USB库动态加载器(单例)，libusb不可用时安全降级 */
class UsbLibraryLoader : public QObject {
    Q_OBJECT

public:
    static UsbLibraryLoader& instance();            ///< @brief 获取单例实例
    bool load(const QString& libraryPath = QString()); ///< @brief 加载libusb动态库，空则系统搜索
    void unload();                                    ///< @brief 卸载libusb动态库
    bool isLoaded() const;                            ///< @brief 查询libusb是否已加载
    QString lastError() const;                        ///< @brief 获取最后错误信息
    QString versionString() const;                    ///< @brief 获取libusb版本字符串(如"1.0.26")

    // libusb函数包装器
    int init(UsbContext** ctx = nullptr);             ///< @brief 初始化libusb上下文，返回0=成功
    void exit(UsbContext* ctx = nullptr);             ///< @brief 释放libusb上下文
    UsbDeviceHandle* openDeviceWithVidPid(UsbContext* ctx, quint16 vid, quint16 pid); ///< @brief 按VID/PID打开设备
    void close(UsbDeviceHandle* handle);              ///< @brief 关闭USB设备
    int claimInterface(UsbDeviceHandle* handle, int interfaceNum);   ///< @brief 声明接口，返回0=成功
    int releaseInterface(UsbDeviceHandle* handle, int interfaceNum); ///< @brief 释放接口，返回0=成功
    /// @brief Bulk传输 @return 0=成功
    int bulkTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                     unsigned char* data, int length, int* transferred, unsigned int timeout);
    /// @brief Interrupt传输 @return 0=成功
    int interruptTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                          unsigned char* data, int length, int* transferred, unsigned int timeout);
    /// @brief Control传输 @return 0=成功
    int controlTransfer(UsbDeviceHandle* handle, quint8 requestType,
                        quint8 request, quint16 value, quint16 index,
                        unsigned char* data, quint16 length, unsigned int timeout);
    void* getDevice(UsbDeviceHandle* handle);         ///< @brief 从句柄获取底层设备对象
    int getDeviceDescriptorFromDevice(void* device, UsbDeviceDescriptor* desc); ///< @brief 从设备读取描述符
    int getDeviceDescriptor(UsbDeviceHandle* handle, UsbDeviceDescriptor* desc); ///< @brief 从句柄读取描述符
    int getStringDescriptorAscii(UsbDeviceHandle* handle, quint8 descIndex,
                                 char* buffer, int bufferSize); ///< @brief 读取字符串描述符
    int kernelDriverActive(UsbDeviceHandle* handle, int interfaceNum);  ///< @brief 检查内核驱动是否活跃
    int detachKernelDriver(UsbDeviceHandle* handle, int interfaceNum);  ///< @brief 分离内核驱动

signals:
    void loadStateChanged(bool loaded);              ///< @brief 库加载状态变更信号

private:
    explicit UsbLibraryLoader(QObject* parent = nullptr); ///< @brief 构造函数(私有, 单例模式)
    ~UsbLibraryLoader() override;                    ///< @brief 析构函数，自动卸载动态库
    Q_DISABLE_COPY(UsbLibraryLoader)
    bool resolveFunctions();                         ///< @brief 解析所有libusb函数指针
    QStringList searchPaths() const;                 ///< @brief 搜索系统中libusb路径
    bool setError(const QString& error);             ///< @brief 设置错误信息并返回false

    QLibrary* m_library = nullptr;   ///< 动态库句柄
    bool m_loaded = false;           ///< 是否已加载
    QString m_lastError;             ///< 最后错误信息
    mutable QMutex m_mutex;          ///< 线程安全互斥锁

    // 函数指针类型定义
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
    using FnGetDevice = void*(*)(UsbDeviceHandle*);
    using FnGetDeviceDesc = int(*)(void*, UsbDeviceDescriptor*);
    using FnGetVersion = const UsbVersion*(*)();           ///< libusb_get_version
    using FnKernelDriverActive = int(*)(UsbDeviceHandle*, int);  ///< libusb_kernel_driver_active
    using FnDetachKernelDriver = int(*)(UsbDeviceHandle*, int);  ///< libusb_detach_kernel_driver

    // 函数指针实例(双列紧凑排列)
    FnInit m_fnInit = nullptr;            FnExit m_fnExit = nullptr;
    FnOpen m_fnOpen = nullptr;            FnClose m_fnClose = nullptr;
    FnClaim m_fnClaim = nullptr;          FnRelease m_fnRelease = nullptr;
    FnBulk m_fnBulk = nullptr;            FnInterrupt m_fnInterrupt = nullptr;
    FnControl m_fnControl = nullptr;      FnGetString m_fnGetString = nullptr;
    FnGetDevice m_fnGetDevice = nullptr;  FnGetDeviceDesc m_fnGetDeviceDesc = nullptr;
    FnGetVersion m_fnGetVersion = nullptr; ///< libusb_get_version函数指针
    FnKernelDriverActive m_fnKernelDriverActive = nullptr;
    FnDetachKernelDriver m_fnDetachKernelDriver = nullptr;

    quint64 m_totalLoadAttempts = 0;      ///< 累计加载尝试
    quint64 m_totalSuccessfulLoads = 0;   ///< 累计成功加载
    quint64 m_totalErrors = 0;            ///< 累计错误
    quint64 m_totalFunctionResolutions = 0; ///< 累计函数符号解析次数
    qint64  m_totalLoadTimeMs = 0;        ///< 累计加载总耗时(ms)
public:
    quint64 totalLoadAttempts() const { return m_totalLoadAttempts; }         ///< @brief 累计加载尝试次数
    quint64 totalSuccessfulLoads() const { return m_totalSuccessfulLoads; }   ///< @brief 累计成功加载次数
    quint64 totalErrors() const { return m_totalErrors; }                     ///< @brief 累计错误次数
    quint64 totalFunctionResolutions() const { return m_totalFunctionResolutions; } ///< @brief 累计函数符号解析次数
    double avgLoadTimeMs() const;         ///< @brief 平均加载耗时(ms)
    void resetLoaderStatistics();         ///< @brief 重置加载器统计计数器
};

#endif // USBLIBRARYLOADER_H
