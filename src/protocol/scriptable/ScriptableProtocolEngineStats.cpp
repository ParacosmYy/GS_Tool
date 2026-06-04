/**
 * @file ScriptableProtocolEngineStats.cpp
 * @brief JavaScript 可脚本化协议引擎 -- 统计接口与重置方法
 *
 * 从 ScriptableProtocolEngine.cpp 中拆分出的统计相关逻辑，职责:
 *   1. stats()      -- 返回运行统计的 const 引用
 *   2. resetStatistics() -- 归零所有统计计数器
 *
 * 注意: resetStatistics() 不影响已加载的脚本和引擎状态，
 * 仅清零累计统计。活跃脚本计数会在重置后重新计算。
 */

#include "protocol/scriptable/ScriptableProtocolEngine.h"

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取运行统计 @return Stats 的 const 引用 */
const ScriptableProtocolEngine::Stats &ScriptableProtocolEngine::stats() const
{
    return m_stats;
}

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零所有 Stats 字段。不影响当前已加载的脚本列表和引擎状态。
 * 活跃脚本计数会在重置后根据 m_scripts 重新计算。
 */
void ScriptableProtocolEngine::resetStatistics()
{
    m_stats.totalEncodeCalls = 0;
    m_stats.totalDecodeCalls = 0;
    m_stats.totalEncodeErrors = 0;
    m_stats.totalDecodeErrors = 0;
    m_stats.totalEncodeBytes = 0;
    m_stats.totalDecodeBytes = 0;
    m_stats.totalScriptsLoaded = 0;
    m_stats.totalScriptsRemoved = 0;
    m_stats.totalImports = 0;
    m_stats.totalExports = 0;

    /* 重新计算活跃脚本数(不依赖累计值) */
    int activeCount = 0;
    for (auto it = m_scripts.constBegin(); it != m_scripts.constEnd(); ++it) {
        if (it->enabled) {
            ++activeCount;
        }
    }
    m_stats.activeScriptCount = activeCount;
}
