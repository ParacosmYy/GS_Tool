/**
 * @file JLinkSdkLoaderQuery.cpp
 * @brief J-Link SDK 查询与统计接口实现
 *
 * 从 JLinkSdkLoader.cpp 拆分而来，包含 SDK 函数符号解析、
 * 状态查询、版本信息获取以及运行统计计数器接口。
 */

#include "rtt/JLinkSdkLoader.h"

// ---- SDK 函数符号解析 ----

/** @brief 从已加载DLL解析所有SDK函数符号，JLINK_Open为必需，其余为可选
 *  @return true=JLINK_Open必需符号解析成功，false=库未加载或JLINK_Open缺失 */
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

// ---- 状态与版本查询 ----

/** @brief 查询SDK是否已加载 @return true=已加载，false=未加载 */
bool JLinkSdkLoader::isLoaded() const
{
    return m_loaded;
}

/** @brief 获取SDK版本号字符串，格式"V<major>.<minor><patch>"(如V7.88b)，未加载返回空
 *  @return 版本号字符串，未加载时返回空，已加载但版本函数不可用时返回"未知" */
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

// ---- 统计接口 ----

/** @brief 获取SDK加载尝试总次数 @return 累计加载尝试次数 */
quint64 JLinkSdkLoader::totalLoadAttempts() const
{
    return m_totalLoadAttempts;
}

/** @brief 获取SDK加载成功总次数 @return 累计加载成功次数 */
quint64 JLinkSdkLoader::totalLoadSuccesses() const
{
    return m_totalLoadSuccesses;
}

/** @brief 获取设备连接尝试总次数 @return 累计连接尝试次数 */
quint64 JLinkSdkLoader::totalConnectAttempts() const
{
    return m_totalConnectAttempts;
}

/** @brief 获取RTT启动总次数 @return 累计RTT启动次数 */
quint64 JLinkSdkLoader::totalRttStarts() const
{
    return m_totalRttStarts;
}

/** @brief 重置SDK统计计数器为初始值(全部归零) */
void JLinkSdkLoader::resetSdkStatistics()
{
    m_totalLoadAttempts = 0;
    m_totalLoadSuccesses = 0;
    m_totalConnectAttempts = 0;
    m_totalRttStarts = 0;
}
