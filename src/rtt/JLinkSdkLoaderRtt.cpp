/**
 * @file JLinkSdkLoaderRtt.cpp
 * @brief J-Link SDK 加载器 — RTT 通信与接口配置实现
 *
 * 从 JLinkSdkLoader.cpp 拆分而来，包含 RTT 启停/读写操作
 * 和调试接口选择、连接速度设置等运行时配置方法。
 */

#include "rtt/JLinkSdkLoader.h"

#include <QMutexLocker>

/** @brief 启动RTT通信(cmd=0对应RTT_START) @return 0=成功，非零=SDK错误码，-1=SDK未加载 */
int JLinkSdkLoader::rttStart()
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttControl) {
        return -1;
    }

    // cmd=0: RTT_START
    ++m_stats.totalRttStarts;
    return m_fnRttControl(0, nullptr);
}

/** @brief 停止RTT通信(cmd=1对应RTT_STOP) @return 0=成功，非零=SDK错误码，-1=SDK未加载 */
int JLinkSdkLoader::rttStop()
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttControl) {
        return -1;
    }

    // cmd=1: RTT_STOP
    ++m_stats.totalRttStops;
    return m_fnRttControl(1, nullptr);
}

/** @brief 从RTT通道读取数据 @param channel RTT通道号(0-15) @param buf 接收缓冲区 @param size 缓冲区大小 @return 实际读取字节数，-1=失败或SDK未加载 */
int JLinkSdkLoader::rttRead(int channel, char* buf, int size)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttRead || !buf || size <= 0) {
        return -1;
    }

    ++m_stats.totalRttReads;
    return m_fnRttRead(channel, buf, size);
}

/** @brief 向RTT通道写入数据 @param channel RTT通道号(0-15) @param buf 待写入数据 @param numBytes 待写入字节数 @return 实际写入字节数，-1=失败或SDK未加载 */
int JLinkSdkLoader::rttWrite(int channel, const char* buf, int numBytes)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnRttWrite || !buf || numBytes <= 0) {
        return -1;
    }

    ++m_stats.totalRttWrites;
    return m_fnRttWrite(channel, buf, numBytes);
}

/** @brief 选择调试接口类型 @param ifType 接口类型: 0=JTAG, 1=SWD @return true=设置成功，false=失败或SDK未加载 */
bool JLinkSdkLoader::selectInterface(int ifType)
{
    QMutexLocker locker(&s_mutex);

    if (!m_fnTIFSelect) {
        return false;
    }

    const int result = m_fnTIFSelect(ifType);
    return (result == 0);
}

/** @brief 设置J-Link连接速度 @param kHz 速度值(单位kHz)，如4000表示4MHz */
void JLinkSdkLoader::setSpeed(int kHz)
{
    QMutexLocker locker(&s_mutex);

    if (m_fnSetSpeed) {
        m_fnSetSpeed(kHz);
    }
}
