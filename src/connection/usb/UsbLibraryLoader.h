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
    static UsbLibraryLoader& instance(); ///< 获取单例
    bool load(const QString& libraryPath = QString()); ///< 加载libusb
    void unload();      ///< 卸载libusb
    bool isLoaded() const; ///< 是否已加载
    QString lastError() const;  ///< 最后错误
    QString versionString() const; ///< libusb版本

    // libusb函数包装器
    int init(UsbContext** ctx = nullptr);
    void exit(UsbContext* ctx = nullptr);
    UsbDeviceHandle* openDeviceWithVidPid(UsbContext* ctx, quint16 vid, quint16 pid);
    void close(UsbDeviceHandle* handle);
    int claimInterface(UsbDeviceHandle* handle, int interfaceNum);
    int releaseInterface(UsbDeviceHandle* handle, int interfaceNum);
    int bulkTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                     unsigned char* data, int length, int* transferred, unsigned int timeout);
    int interruptTransfer(UsbDeviceHandle* handle, unsigned char endpoint,
                          unsigned char* data, int length, int* transferred, unsigned int timeout);
    int controlTransfer(UsbDeviceHandle* handle, quint8 requestType,
                        quint8 request, quint16 value, quint16 index,
                        unsigned char* data, quint16 length, unsigned int timeout);
    void* getDevice(UsbDeviceHandle* handle);
    int getDeviceDescriptorFromDevice(void* device, UsbDeviceDescriptor* desc);
    int getDeviceDescriptor(UsbDeviceHandle* handle, UsbDeviceDescriptor* desc);
    int getStringDescriptorAscii(UsbDeviceHandle* handle, quint8 descIndex,
                                 char* buffer, int bufferSize);
    int kernelDriverActive(UsbDeviceHandle* handle, int interfaceNum);
    int detachKernelDriver(UsbDeviceHandle* handle, int interfaceNum);

signals:
    void loadStateChanged(bool loaded); ///< 库加载状态变更

private:
    explicit UsbLibraryLoader(QObject* parent = nullptr);
    ~UsbLibraryLoader() override;
    Q_DISABLE_COPY(UsbLibraryLoader)
    bool resolveFunctions();    ///< 解析所有函数指针
    QStringList searchPaths() const; ///< 搜索libusb路径
    bool setError(const QString& error); ///< 设置错误并返回false

    QLibrary* m_library = nullptr;
    bool m_loaded = false;
    QString m_lastError;
    mutable QMutex m_mutex;

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
