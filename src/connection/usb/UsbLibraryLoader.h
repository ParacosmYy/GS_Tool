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

/** @brief libusb设备描述符(简化版) */
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
    quint8  bNumConfigurations; ///< 配置数量
};

/** @brief USB库动态加载器(单例)，libusb不可用时安全降级 */
class UsbLibraryLoader : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例 @return 单例引用 */
    static UsbLibraryLoader& instance();
    /** @brief 加载libusb动态库 @param libraryPath DLL/SO路径，空则系统搜索 @return true=加载成功 */
    bool load(const QString& libraryPath = QString());
    /** @brief 卸载libusb动态库 */
    void unload();
    /** @brief 查询libusb是否已加载 @return true=已加载 */
    bool isLoaded() const;
    /** @brief 获取最后错误信息 @return 错误描述字符串 */
    QString lastError() const;
    /** @brief 获取libusb版本字符串 @return 版本号(如"1.0.26") */
    QString versionString() const;

    // libusb函数包装器
    /** @brief 初始化libusb上下文 @param ctx 输出上下文指针 @return 0=成功 */
    int init(UsbContext** ctx = nullptr);
    /** @brief 释放libusb上下文 @param ctx 上下文指针 */
    void exit(UsbContext* ctx = nullptr);
    /** @brief 按VID/PID打开USB设备 @param ctx libusb上下文 @param vid 厂商ID @param pid 产品ID @return 设备句柄，失败返回nullptr */
    UsbDeviceHandle* openDeviceWithVidPid(UsbContext* ctx, quint16 vid, quint16 pid);
    /** @brief 关闭USB设备 @param handle 设备句柄 */
    void close(UsbDeviceHandle* handle);
    /** @brief 声明USB接口 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0=成功 */
    int claimInterface(UsbDeviceHandle* handle, int interfaceNum);
    /** @brief 释放USB接口 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0=成功 */
    int releaseInterface(UsbDeviceHandle* handle, int interfaceNum);
    /** @brief Bulk传输 @param handle 设备句柄 @param endpoint 端点地址 @param data 数据缓冲区 @param length 缓冲区长度 @param transferred 实际传输字节数输出 @param timeout 超时(ms) @return 0=成功 */
    int bulkTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                     unsigned char* data, int length, int* transferred, unsigned int timeout);
    /** @brief Interrupt传输 @param handle 设备句柄 @param endpoint 端点地址 @param data 数据缓冲区 @param length 缓冲区长度 @param transferred 实际传输字节数输出 @param timeout 超时(ms) @return 0=成功 */
    int interruptTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                          unsigned char* data, int length, int* transferred, unsigned int timeout);
    /** @brief Control传输 @param handle 设备句柄 @param requestType 请求类型 @param request 请求码 @param value 值字段 @param index 索引字段 @param data 数据缓冲区 @param length 数据长度 @param timeout 超时(ms) @return 0=成功 */
    int controlTransfer(UsbDeviceHandle* handle, quint8 requestType,
                        quint8 request, quint16 value, quint16 index,
                        unsigned char* data, quint16 length, unsigned int timeout);
    /** @brief 从设备句柄获取底层设备对象 @param handle 设备句柄 @return 设备指针 */
    void* getDevice(UsbDeviceHandle* handle);
    /** @brief 从设备对象读取设备描述符 @param device 设备指针 @param desc 输出描述符 @return 0=成功 */
    int getDeviceDescriptorFromDevice(void* device, UsbDeviceDescriptor* desc);
    /** @brief 从设备句柄读取设备描述符 @param handle 设备句柄 @param desc 输出描述符 @return 0=成功 */
    int getDeviceDescriptor(UsbDeviceHandle* handle, UsbDeviceDescriptor* desc);
    /** @brief 读取字符串描述符 @param handle 设备句柄 @param descIndex 描述符索引 @param buffer 输出缓冲区 @param bufferSize 缓冲区大小 @return 实际读取字节数 */
    int getStringDescriptorAscii(UsbDeviceHandle* handle, quint8 descIndex,
                                 char* buffer, int bufferSize);
    /** @brief 检查内核驱动是否活跃 @param handle 设备句柄 @param interfaceNum 接口编号 @return 1=活跃, 0=不活跃 */
    int kernelDriverActive(UsbDeviceHandle* handle, int interfaceNum);
    /** @brief 分离内核驱动 @param handle 设备句柄 @param interfaceNum 接口编号 @return 0=成功 */
    int detachKernelDriver(UsbDeviceHandle* handle, int interfaceNum);

signals:
    /** @brief 库加载状态变更信号 @param loaded true=已加载, false=已卸载 */
    void loadStateChanged(bool loaded);

private:
    /** @brief 构造函数(私有, 单例模式) @param parent 父对象 */
    explicit UsbLibraryLoader(QObject* parent = nullptr);
    /** @brief 析构函数，自动卸载动态库 */
    ~UsbLibraryLoader() override;
    Q_DISABLE_COPY(UsbLibraryLoader)
    /** @brief 从已加载DLL解析所有libusb函数指针 @return true=全部必需符号解析成功 */
    bool resolveFunctions();
    /** @brief 搜索系统中libusb可能存在的路径 @return 候选路径列表 */
    QStringList searchPaths() const;
    /** @brief 设置错误信息并返回false @param error 错误描述 @return 始终返回false */
    bool setError(const QString& error);

    QLibrary* m_library = nullptr;   ///< 动态库句柄
    bool m_loaded = false;           ///< 是否已加载
    QString m_lastError;             ///< 最后错误信息
    mutable QMutex m_mutex;          ///< 线程安全互斥锁

    // 函数指针类型
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

    FnInit m_fnInit = nullptr;       FnExit m_fnExit = nullptr;
    FnOpen m_fnOpen = nullptr;       FnClose m_fnClose = nullptr;
    FnClaim m_fnClaim = nullptr;     FnRelease m_fnRelease = nullptr;
    FnBulk m_fnBulk = nullptr;       FnInterrupt m_fnInterrupt = nullptr;
    FnControl m_fnControl = nullptr; FnGetString m_fnGetString = nullptr;
    FnGetDevice m_fnGetDevice = nullptr; FnGetDeviceDesc m_fnGetDeviceDesc = nullptr;

    quint64 m_totalLoadAttempts = 0;    ///< 累计加载尝试
    quint64 m_totalSuccessfulLoads = 0; ///< 累计成功加载
    quint64 m_totalErrors = 0;          ///< 累计错误
public:
    /** @brief 获取累计加载尝试次数 */
    quint64 totalLoadAttempts() const { return m_totalLoadAttempts; }
    /** @brief 获取累计成功加载次数 */
    quint64 totalSuccessfulLoads() const { return m_totalSuccessfulLoads; }
    /** @brief 获取累计错误次数 */
    quint64 totalErrors() const { return m_totalErrors; }
    /** @brief 重置加载器统计计数器 */
    void resetLoaderStatistics() { m_totalLoadAttempts = 0; m_totalSuccessfulLoads = 0; m_totalErrors = 0; }
};

#endif // USBLIBRARYLOADER_H
