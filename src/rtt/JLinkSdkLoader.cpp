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

/**
 * @brief 获取单例实例
 *
 * 使用 QMutexLocker 保证线程安全的双重检查锁定模式。
 * 实例在首次调用时创建，由 QCoreApplication 退出时自动销毁。
 *
 * @return 单例指针
 */
JLinkSdkLoader* JLinkSdkLoader::instance()
{
    static JLinkSdkLoader* s_instance = nullptr;

    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new JLinkSdkLoader();
    }
    return s_instance;
}

/**
 * @brief 私有构造函数（单例模式）
 * @param parent 父对象
 */
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

/**
 * @brief 析构函数
 *
 * 析构时自动卸载已加载的 SDK 动态库。
 */
JLinkSdkLoader::~JLinkSdkLoader()
{
    unload();
}

/**
 * @brief 加载 J-Link SDK 动态库
 *
 * 创建 QLibrary 实例并加载指定路径的动态库文件。
 * 加载后调用 resolveFunctions() 解析全部 SDK 符号。
 * JLINK_Open 为必需符号，缺失则视为无效库。
 *
 * @param path DLL 文件路径，为空则使用系统默认搜索路径
 * @return true 加载并解析成功，false 加载失败或符号缺失
 */
bool JLinkSdkLoader::load(const QString& path)
{
    QMutexLocker locker(&s_mutex);

    if (m_loaded) {
        unload();
    }

    m_library = new QLibrary(path.isEmpty()
                                 ? QStringLiteral("JLinkARM")
                                 : path,
                             this);

    if (!m_library->load()) {
        const QString error = m_library->errorString();
        emit sdkLoadFailed(tr("SDK 加载失败: %1").arg(error));
        delete m_library;
        m_library = nullptr;
        return false;
    }

    // 解析所有 SDK 函数符号，JLINK_Open 为必需
    if (!resolveFunctions()) {
        const QString libPath = m_library->fileName();
        emit sdkLoadFailed(tr("SDK 符号解析失败，库文件无效: %1").arg(libPath));
        m_library->unload();
        delete m_library;
        m_library = nullptr;
        return false;
    }

    m_loaded = true;
    emit sdkLoaded();
    return true;
}

/**
 * @brief 解析所有 SDK 函数符号
 *
 * 从已加载的 QLibrary 中逐一 resolve 所有函数指针。
 * JLINK_Open 为必需符号，解析失败返回 false。
 * 其余符号为可选，缺失时指针保持 nullptr（调用时跳过）。
 *
 * @return true 必需符号全部解析成功，false JLINK_Open 缺失
 */
bool JLinkSdkLoader::resolveFunctions()
{
    if (!m_library || !m_library->isLoaded()) {
        return false;
    }

    // 必需符号 — JLINK_Open
    m_fnOpen = reinterpret_cast<FnJLink_Open>(m_library->resolve("JLINK_Open"));
    if (!m_fnOpen) {
        return false;
    }

    // 可选符号 — 其余 SDK 函数
    m_fnClose = reinterpret_cast<FnJLink_Close>(m_library->resolve("JLINK_Close"));
    m_fnConnect = reinterpret_cast<FnJLink_Connect>(m_library->resolve("JLINK_Connect"));
    m_fnDisconnect = reinterpret_cast<FnJLink_Disconnect>(m_library->resolve("JLINK_Disconnect"));
    m_fnExecCommand = reinterpret_cast<FnJLink_ExecCommand>(m_library->resolve("JLINK_ExecCommand"));
    m_fnGetDLLVersion = reinterpret_cast<FnJLink_GetDLLVersion>(m_library->resolve("JLINK_GetDLLVersion"));
    m_fnTIFSelect = reinterpret_cast<FnJLink_TIF_Select>(m_library->resolve("JLINK_TIF_Select"));
    m_fnSetSpeed = reinterpret_cast<FnJLink_SetSpeed>(m_library->resolve("JLINK_SetSpeed"));
    m_fnRttControl = reinterpret_cast<FnJLINK_RTTERMINAL_Control>(
        m_library->resolve("JLINK_RTTERMINAL_Control"));
    m_fnRttRead = reinterpret_cast<FnJLINK_RTTERMINAL_Read>(
        m_library->resolve("JLINK_RTTERMINAL_Read"));
    m_fnRttWrite = reinterpret_cast<FnJLINK_RTTERMINAL_Write>(
        m_library->resolve("JLINK_RTTERMINAL_Write"));

    return true;
}

/**
 * @brief 卸载 J-Link SDK
 *
 * 卸载动态库并释放 QLibrary 实例。
 * 重置所有 SDK 函数指针和加载状态。
 */
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

/**
 * @brief 查询 SDK 是否已加载
 * @return true 已加载，false 未加载
 */
bool JLinkSdkLoader::isLoaded() const
{
    return m_loaded;
}

/**
 * @brief 获取 SDK 版本字符串
 *
 * 调用 JLINK_GetDLLVersion() 获取整数版本号。
 * 版本编码: major = version / 10000,
 *           minor = (version / 100) % 100,
 *           patch = version % 100 → 映射为字母 a-z。
 * 格式: "V<major>.<minor><patch>"（如 "V7.88b"）。
 *
 * @return 版本号字符串，未加载时返回空字符串
 */
QString JLinkSdkLoader::sdkVersion() const
{
    if (!m_loaded || !m_fnGetDLLVersion) {
        if (!m_loaded) {
            return QString();
        }
        return tr("未知");
    }

    const int version = m_fnGetDLLVersion();
    if (version <= 0) {
        return tr("未知");
    }

    const int major = version / 10000;
    const int minor = (version / 100) % 100;
    const int patchNum = version % 100;
    const QChar patchLetter(static_cast<char>('a' + (patchNum % 26)));

    return QStringLiteral("V%1.%2%3")
        .arg(major)
        .arg(minor, 2, 10, QLatin1Char('0'))
        .arg(patchLetter);
}

/**
 * @brief 连接到目标设备
 *
 * 依次调用 JLINK_Open → JLINK_Connect 连接 J-Link 调试器。
 *
 * @param deviceId 设备标识（如 "Cortex-M4"）
 * @return true 连接成功，false SDK 未加载或连接失败
 */
bool JLinkSdkLoader::connectToDevice(const QString& deviceId)
{
    QMutexLocker locker(&s_mutex);

    if (!m_loaded || !m_fnOpen || !m_fnConnect) {
        return false;
    }

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

/**
 * @brief 断开与目标设备的连接
 *
 * 依次调用 JLINK_Disconnect → JLINK_Close。
 */
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

/**
 * @brief 启动 RTT 通信
 *
 * 调用 JLINK_RTTERMINAL_Control(cmd=0, buf=nullptr) 启动 RTT 会话。
 * cmd=0 对应 SEGGER RTT 的 START 命令。
 *
 * @return 0 成功，非零为 SDK 错误码，-1 表示 SDK 未加载
 */
int JLinkSdkLoader::rttStart()
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttControl) {
        return -1;
    }

    // cmd=0: RTT_START
    return m_fnRttControl(0, nullptr);
}

/**
 * @brief 停止 RTT 通信
 *
 * 调用 JLINK_RTTERMINAL_Control(cmd=1, buf=nullptr) 停止 RTT 会话。
 * cmd=1 对应 SEGGER RTT 的 STOP 命令。
 *
 * @return 0 成功，非零为 SDK 错误码，-1 表示 SDK 未加载
 */
int JLinkSdkLoader::rttStop()
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttControl) {
        return -1;
    }

    // cmd=1: RTT_STOP
    return m_fnRttControl(1, nullptr);
}

/**
 * @brief 从 RTT 通道读取数据
 *
 * @param channel RTT 通道号（0-15）
 * @param buf 接收缓冲区
 * @param size 缓冲区大小
 * @return 实际读取字节数，-1 表示失败或 SDK 未加载
 */
int JLinkSdkLoader::rttRead(int channel, char* buf, int size)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttRead || !buf || size <= 0) {
        return -1;
    }

    return m_fnRttRead(channel, buf, size);
}

/**
 * @brief 向 RTT 通道写入数据
 *
 * @param channel RTT 通道号（0-15）
 * @param buf 待写入数据
 * @param numBytes 待写入字节数
 * @return 实际写入字节数，-1 表示失败或 SDK 未加载
 */
int JLinkSdkLoader::rttWrite(int channel, const char* buf, int numBytes)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttWrite || !buf || numBytes <= 0) {
        return -1;
    }

    return m_fnRttWrite(channel, buf, numBytes);
}

/**
 * @brief 选择调试接口类型
 *
 * @param ifType 接口类型：0=JTAG, 1=SWD
 * @return true 设置成功，false 失败或 SDK 未加载
 */
bool JLinkSdkLoader::selectInterface(int ifType)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnTIFSelect) {
        return false;
    }

    const int result = m_fnTIFSelect(ifType);
    return (result == 0);
}

/**
 * @brief 设置 J-Link 连接速度
 * @param kHz 速度值（单位 kHz），如 4000 表示 4MHz
 */
void JLinkSdkLoader::setSpeed(int kHz)
{
    QMutexLocker locker(&s_mutex);

    if (m_fnSetSpeed) {
        m_fnSetSpeed(kHz);
    }
}
