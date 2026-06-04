/** @file JLinkSdkLoader.h @brief J-Link SDK 动态库加载器(单例) — 运行时加载JLinkARM.dll，解析SDK函数指针 */
#ifndef JLINKSDKLOADER_H
#define JLINKSDKLOADER_H

#include <QObject>
#include <QLibrary>
#include <QMutex>

/** @brief J-Link SDK 动态库加载器(单例)，运行时加载JLinkARM.dll并缓存函数指针，线程安全 */
class JLinkSdkLoader : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例(线程安全，QMutexLocker保护首次创建) @return 单例指针 */
    static JLinkSdkLoader* instance();
    /** @brief 析构函数，自动卸载 SDK */
    ~JLinkSdkLoader() override;
    /** @brief 加载J-Link SDK动态库，JLINK_Open符号必须解析成功 @param path DLL路径(空则系统搜索) @return true加载成功 */
    bool load(const QString& path = QString());
    /** @brief 卸载J-Link SDK，释放动态库句柄并重置所有函数指针 */
    void unload();
    /** @brief 查询SDK是否已加载 */
    bool isLoaded() const;
    /** @brief 获取SDK版本字符串(如"V7.88b")，未加载返回空 */
    QString sdkVersion() const;
    /** @brief 连接到目标设备 @param deviceId 设备标识(如"Cortex-M4") @return true连接成功 */
    bool connectToDevice(const QString& deviceId);
    /** @brief 断开与目标设备的连接 */
    void disconnect();
    /** @brief 启动RTT通信(需先connectToDevice)，调用JLINK_RTTERMINAL_Control @return 0成功，非零SDK错误码 */
    int rttStart();
    /** @brief 停止RTT通信 @return 0成功，非零SDK错误码 */
    int rttStop();
    /** @brief 从RTT通道读取数据 @param channel 通道号(0-15) @param buf 接收缓冲区 @param size 缓冲区大小 @return 实际读取字节数，-1失败 */
    int rttRead(int channel, char* buf, int size);
    /** @brief 向RTT通道写入数据 @param channel 通道号(0-15) @return 实际写入字节数，-1失败 */
    int rttWrite(int channel, const char* buf, int numBytes);
    /** @brief 选择调试接口类型 @param ifType 0=JTAG, 1=SWD @return true设置成功 */
    bool selectInterface(int ifType);
    /** @brief 设置J-Link连接速度 @param kHz 速度值(单位kHz) */
    void setSpeed(int kHz);

    // ---- 统计接口 ----

    /** @brief SDK运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalLoadAttempts = 0;     ///< 累计SDK加载尝试次数
        quint64 totalLoadSuccesses = 0;    ///< 累计成功加载次数
        quint64 totalLoadFailures = 0;     ///< 累计加载失败次数
        quint64 totalUnloads = 0;          ///< 累计SDK卸载次数
        quint64 totalConnectAttempts = 0;  ///< 累计设备连接尝试次数
        quint64 totalRttStarts = 0;        ///< 累计RTT启动次数
        quint64 totalRttStops = 0;         ///< 累计RTT停止次数
        quint64 totalRttReads = 0;         ///< 累计RTT读操作次数
        quint64 totalRttWrites = 0;        ///< 累计RTT写操作次数
        quint64 dllPathQueries = 0;        ///< 累计DLL路径查询次数
        quint64 sdkVersionQueries = 0;     ///< 累计SDK版本查询次数
    };

    quint64 totalLoadAttempts() const;     ///< @return 累计SDK加载尝试次数
    quint64 totalLoadSuccesses() const;    ///< @return 累计成功加载次数
    quint64 totalConnectAttempts() const;  ///< @return 累计设备连接尝试次数
    quint64 totalRttStarts() const;        ///< @return 累计RTT启动次数
    /** @brief 获取累计加载失败次数 @return 失败总数 */
    quint64 totalLoadFailures() const { return m_stats.totalLoadFailures; }
    /** @brief 获取累计SDK卸载次数 @return 卸载总数 */
    quint64 totalUnloads() const { return m_stats.totalUnloads; }
    /** @brief 获取累计RTT停止次数 @return 停止总数 */
    quint64 totalRttStops() const { return m_stats.totalRttStops; }
    /** @brief 获取累计RTT读操作次数 @return 读操作总数 */
    quint64 totalRttReads() const { return m_stats.totalRttReads; }
    /** @brief 获取累计RTT写操作次数 @return 写操作总数 */
    quint64 totalRttWrites() const { return m_stats.totalRttWrites; }
    /** @brief 获取累计DLL路径查询次数 @return 查询总数 */
    quint64 totalDllPathQueries() const { return m_stats.dllPathQueries; }
    /** @brief 获取累计SDK版本查询次数 @return 查询总数 */
    quint64 totalSdkVersionQueries() const { return m_stats.sdkVersionQueries; }
    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }
    void resetSdkStatistics();             ///< 重置所有统计计数器归零

signals:
    void sdkLoaded();                      ///< SDK加载成功信号
    void sdkLoadFailed(const QString& error); ///< SDK加载失败信号

private:
    /** @brief 私有构造函数(单例模式) @param parent 父对象 */
    explicit JLinkSdkLoader(QObject* parent = nullptr);
    Q_DISABLE_COPY(JLinkSdkLoader)
    /** @brief 从已加载DLL解析所有SDK函数符号，JLINK_Open必需 @return true所有必需符号解析成功 */
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
    mutable Stats m_stats;    ///< 聚合统计结构体(mutable因const方法需修改)
};

#endif // JLINKSDKLOADER_H
