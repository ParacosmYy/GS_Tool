/**
 * @file JLinkSdkLoader.h
 * @brief J-Link SDK 动态库加载器 — 运行时加载 J-Link SDK DLL
 *
 * 使用 QLibrary 动态加载 J-Link SDK，避免编译期硬依赖。
 * 提供 SDK 函数指针的解析、缓存和类型安全调用封装。
 * 采用单例模式，全局共享同一份 SDK 加载实例。
 *
 * 协作关系:
 *   - JLinkRttConnection: 通过本加载器调用 SDK 函数
 *   - RttConfigPanel: 显示 SDK 加载状态和版本信息
 */
#ifndef JLINKSDKLOADER_H
#define JLINKSDKLOADER_H

#include <QObject>
#include <QLibrary>
#include <QMutex>

/**
 * @brief J-Link SDK 动态库加载器（单例）
 *
 * 负责在运行时加载 JLinkARM.dll（或对应平台库），
 * 解析 SDK 函数符号并缓存函数指针。
 * 线程安全：instance() 和所有公开方法均可跨线程调用。
 */
class JLinkSdkLoader : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     *
     * 线程安全，使用 QMutexLocker 保护首次创建。
     *
     * @return 单例指针，调用者不持有所有权
     */
    static JLinkSdkLoader* instance();

    /** @brief 析构函数，自动卸载 SDK */
    ~JLinkSdkLoader() override;

    /**
     * @brief 加载 J-Link SDK 动态库
     *
     * 使用 QLibrary 加载指定路径的 DLL，解析所有必需 SDK 符号。
     * JLINK_Open 符号必须解析成功，否则视为无效库。
     * 加载成功后发出 sdkLoaded 信号。
     *
     * @param path DLL 文件路径，为空则使用系统搜索路径
     * @return true 加载成功，false 加载失败
     */
    bool load(const QString& path = QString());

    /**
     * @brief 卸载 J-Link SDK
     *
     * 释放动态库句柄，重置加载状态和所有函数指针。
     */
    void unload();

    /**
     * @brief 查询 SDK 是否已加载
     * @return true 已加载，false 未加载
     */
    bool isLoaded() const;

    /**
     * @brief 获取 SDK 版本字符串
     *
     * 调用 JLINK_GetDLLVersion() 获取版本号并格式化为
     * "V<major>.<minor><patch>" 格式（如 "V7.88b"）。
     *
     * @return 版本号字符串，未加载时返回空字符串
     */
    QString sdkVersion() const;

    /**
     * @brief 连接到目标设备
     * @param deviceId 设备标识字符串（如 "Cortex-M4"）
     * @return true 连接成功，false 连接失败或 SDK 未加载
     */
    bool connectToDevice(const QString& deviceId);

    /**
     * @brief 断开与目标设备的连接
     */
    void disconnect();

    /**
     * @brief 启动 RTT 通信
     *
     * 调用 JLINK_RTTERMINAL_Control 启动 RTT 会话。
     * 启动前需先调用 connectToDevice() 建立连接。
     *
     * @return 0 成功，非零为 SDK 错误码
     */
    int rttStart();

    /**
     * @brief 停止 RTT 通信
     *
     * 调用 JLINK_RTTERMINAL_Control 停止 RTT 会话。
     *
     * @return 0 成功，非零为 SDK 错误码
     */
    int rttStop();

    /**
     * @brief 从 RTT 通道读取数据
     *
     * @param channel RTT 通道号（0-15）
     * @param buf 接收缓冲区
     * @param size 缓冲区大小
     * @return 实际读取字节数，-1 表示失败
     */
    int rttRead(int channel, char* buf, int size);

    /**
     * @brief 向 RTT 通道写入数据
     *
     * @param channel RTT 通道号（0-15）
     * @param buf 待写入数据
     * @param numBytes 待写入字节数
     * @return 实际写入字节数，-1 表示失败
     */
    int rttWrite(int channel, const char* buf, int numBytes);

    /**
     * @brief 选择调试接口类型
     *
     * @param ifType 接口类型：0=JTAG, 1=SWD
     * @return true 设置成功，false 失败或 SDK 未加载
     */
    bool selectInterface(int ifType);

    /**
     * @brief 设置 J-Link 连接速度
     * @param kHz 速度值（单位 kHz），如 4000 表示 4MHz
     */
    void setSpeed(int kHz);

    // ---- 统计接口 ----

    /** @brief 获取累计SDK加载尝试次数 */
    quint64 totalLoadAttempts() const;

    /** @brief 获取累计成功加载次数 */
    quint64 totalLoadSuccesses() const;

    /** @brief 获取累计设备连接尝试次数 */
    quint64 totalConnectAttempts() const;

    /** @brief 获取累计RTT启动次数 */
    quint64 totalRttStarts() const;

    /** @brief 重置所有统计计数器归零 */
    void resetSdkStatistics();

signals:
    /** @brief SDK 加载成功信号 */
    void sdkLoaded();

    /**
     * @brief SDK 加载失败信号
     * @param error 错误描述
     */
    void sdkLoadFailed(const QString& error);

private:
    /** @brief 私有构造函数（单例模式） */
    explicit JLinkSdkLoader(QObject* parent = nullptr);
    Q_DISABLE_COPY(JLinkSdkLoader)

    /**
     * @brief 从已加载的 DLL 中解析所有 SDK 函数符号
     *
     * 逐一解析 JLINK_Open、JLINK_Close 等函数指针。
     * 必需符号（JLINK_Open）解析失败时返回 false。
     *
     * @return true 所有必需符号解析成功，false 至少一个必需符号缺失
     */
    bool resolveFunctions();

    // ---- SDK 函数指针类型定义 ----
    using FnJLink_Open = int(*)();
    using FnJLink_Close = void(*)();
    using FnJLink_Connect = int(*)(const char*);
    using FnJLink_Disconnect = void(*)();
    using FnJLink_ExecCommand = int(*)(const char* cmd, void* a, int b);
    using FnJLink_GetDLLVersion = int(*)();
    using FnJLink_TIF_Select = int(*)(int);
    using FnJLink_SetSpeed = void(*)(int);
    using FnJLINK_RTTERMINAL_Control = int(*)(int cmd, void* buf);
    using FnJLINK_RTTERMINAL_Read = int(*)(int bufIndex, char* buf, int bufSize);
    using FnJLINK_RTTERMINAL_Write = int(*)(int bufIndex, const char* buf, int numBytes);

    QLibrary* m_library = nullptr;  ///< 动态库句柄
    bool m_loaded = false;          ///< 加载状态标志

    // ---- 已解析的 SDK 函数指针 ----
    FnJLink_Open m_fnOpen = nullptr;                       ///< JLINK_Open
    FnJLink_Close m_fnClose = nullptr;                     ///< JLINK_Close
    FnJLink_Connect m_fnConnect = nullptr;                 ///< JLINK_Connect
    FnJLink_Disconnect m_fnDisconnect = nullptr;           ///< JLINK_Disconnect
    FnJLink_ExecCommand m_fnExecCommand = nullptr;         ///< JLINK_ExecCommand
    FnJLink_GetDLLVersion m_fnGetDLLVersion = nullptr;     ///< JLINK_GetDLLVersion
    FnJLink_TIF_Select m_fnTIFSelect = nullptr;            ///< JLINK_TIF_Select
    FnJLink_SetSpeed m_fnSetSpeed = nullptr;               ///< JLINK_SetSpeed
    FnJLINK_RTTERMINAL_Control m_fnRttControl = nullptr;   ///< RTT Control
    FnJLINK_RTTERMINAL_Read m_fnRttRead = nullptr;         ///< RTT Read
    FnJLINK_RTTERMINAL_Write m_fnRttWrite = nullptr;       ///< RTT Write

    static QMutex s_mutex;  ///< 单例访问互斥锁

    // ---- 统计计数器 ----
    quint64 m_totalLoadAttempts = 0;    ///< 累计SDK加载尝试次数
    quint64 m_totalLoadSuccesses = 0;   ///< 累计成功加载次数
    quint64 m_totalConnectAttempts = 0; ///< 累计设备连接尝试次数
    quint64 m_totalRttStarts = 0;       ///< 累计RTT启动次数
};

#endif // JLINKSDKLOADER_H
