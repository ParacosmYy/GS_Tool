/**
 * @file JLinkSdkLoader.cpp
 * @brief J-Link SDK 动态库加载器实现
 *
 * 使用 QLibrary 在运行时加载 J-Link SDK 动态库（JLinkARM.dll），
 * 解析全部 SDK 函数符号并提供类型安全的调用封装。
 */

#include "rtt/JLinkSdkLoader.h"

#include <QMutexLocker>

// ---- 静态成员初始化 ----
QMutex JLinkSdkLoader::s_mutex;

/** @brief 获取单例实例(线程安全双重检查锁定) @return 单例指针 */
JLinkSdkLoader* JLinkSdkLoader::instance()
{
    static JLinkSdkLoader* s_instance = nullptr;

    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new JLinkSdkLoader();
    }
    return s_instance;
}

/** @brief 私有构造函数(单例模式)，初始化所有SDK函数指针为nullptr @param parent 父对象 */
JLinkSdkLoader::JLinkSdkLoader(QObject* parent)
    : QObject(parent)
    , m_library(nullptr)
    , m_loaded(false)
    , m_fnOpen(nullptr)
    , m_fnClose(nullptr)
    , m_fnConnect(nullptr)
    , m_fnDisconnect(nullptr)
    , m_fnExecCommand(nullptr)
    , m_fnGetDLLVersion(nullptr)
    , m_fnTIFSelect(nullptr)
    , m_fnSetSpeed(nullptr)
    , m_fnRttControl(nullptr)
    , m_fnRttRead(nullptr)
    , m_fnRttWrite(nullptr)
{
}

/** @brief 析构函数，析构时自动卸载已加载的SDK动态库 */
JLinkSdkLoader::~JLinkSdkLoader()
{
    unload();
}

/** @brief 加载J-Link SDK动态库，解析全部SDK符号，JLINK_Open为必需符号 @param path DLL文件路径，为空则使用系统默认搜索路径 @return true=加载并解析成功，false=加载失败或符号缺失 */
bool JLinkSdkLoader::load(const QString& path)
{
    QMutexLocker locker(&s_mutex);

    ++m_stats.totalLoadAttempts;

    if (m_loaded) {
        unload();
    }

    m_library = new QLibrary(path.isEmpty()
                                 ? QStringLiteral("JLinkARM")
                                 : path,
                             this);

    if (!m_library->load()) {
        const QString error = m_library->errorString();
        ++m_stats.totalLoadFailures;
        emit sdkLoadFailed(tr("SDK 加载失败: %1").arg(error));
        delete m_library;
        m_library = nullptr;
        return false;
    }

    // 解析所有 SDK 函数符号，JLINK_Open 为必需
    if (!resolveFunctions()) {
        ++m_stats.totalLoadFailures;
        const QString libPath = m_library->fileName();
        emit sdkLoadFailed(tr("SDK 符号解析失败，库文件无效: %1").arg(libPath));
        m_library->unload();
        delete m_library;
        m_library = nullptr;
        return false;
    }

    m_loaded = true;
    ++m_stats.totalLoadSuccesses;
    emit sdkLoaded();
    return true;
}

/** @brief 卸载J-Link SDK，释放QLibrary实例并重置所有函数指针 */
void JLinkSdkLoader::unload()
{
    if (m_library) {
        if (m_library->isLoaded()) {
            m_library->unload();
        }
        delete m_library;
        m_library = nullptr;
    }

    m_loaded = false;
    if (m_stats.totalLoadSuccesses > 0) {
        ++m_stats.totalUnloads;
    }
    m_fnOpen = nullptr;
    m_fnClose = nullptr;
    m_fnConnect = nullptr;
    m_fnDisconnect = nullptr;
    m_fnExecCommand = nullptr;
    m_fnGetDLLVersion = nullptr;
    m_fnTIFSelect = nullptr;
    m_fnSetSpeed = nullptr;
    m_fnRttControl = nullptr;
    m_fnRttRead = nullptr;
    m_fnRttWrite = nullptr;
}

/** @brief 连接到目标设备(JLINK_Open→JLINK_Connect) @param deviceId 设备标识(如"Cortex-M4") @return true=连接成功，false=SDK未加载或连接失败 */
bool JLinkSdkLoader::connectToDevice(const QString& deviceId)
{
    QMutexLocker locker(&s_mutex);

    if (!m_loaded || !m_fnOpen || !m_fnConnect) {
        return false;
    }

    ++m_stats.totalConnectAttempts;

    // 先打开 J-Link 句柄
    const int openResult = m_fnOpen();
    if (openResult != 0) {
        return false;
    }

    // 连接到指定设备
    const QByteArray devId = deviceId.toUtf8();
    const int connectResult = m_fnConnect(devId.constData());
    return (connectResult == 0);
}

/** @brief 断开与目标设备的连接(JLINK_Disconnect→JLINK_Close) */
void JLinkSdkLoader::disconnect()
{
    QMutexLocker locker(&s_mutex);

    if (!m_loaded) {
        return;
    }

    if (m_fnDisconnect) {
        m_fnDisconnect();
    }
    if (m_fnClose) {
        m_fnClose();
    }
}

// RTT操作/接口配置方法已移至 JLinkSdkLoaderRtt.cpp
