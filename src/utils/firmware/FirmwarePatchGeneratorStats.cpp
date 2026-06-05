/**
 * @file FirmwarePatchGeneratorStats.cpp
 * @brief 固件补丁生成器 — 统计计数器访问器与重置
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/firmware/FirmwarePatchGenerator.h"

/**
 * @brief 获取累计生成补丁次数
 * @return generateIhexPatch()/generateBinPatch() 的累计调用数
 */
quint64 FirmwarePatchGenerator::totalPatchesGenerated() const
{
    return m_totalPatchesGenerated;
}

/**
 * @brief 获取累计应用补丁次数
 * @return applyPatch() 的累计成功调用数
 */
quint64 FirmwarePatchGenerator::totalPatchesApplied() const
{
    return m_totalPatchesApplied;
}

/**
 * @brief 获取累计补丁总字节数
 * @return 所有生成补丁的累计字节数
 */
quint64 FirmwarePatchGenerator::totalPatchBytes() const
{
    return m_totalPatchBytes;
}

/**
 * @brief 获取累计应用错误次数
 * @return CRC 校验失败等累计错误数
 */
quint64 FirmwarePatchGenerator::totalApplyErrors() const
{
    return m_totalApplyErrors;
}

/**
 * @brief 重置所有累计统计计数器为初始值
 */
void FirmwarePatchGenerator::resetStatistics()
{
    m_totalPatchesGenerated = 0;
    m_totalPatchesApplied   = 0;
    m_totalPatchBytes       = 0;
    m_totalApplyErrors      = 0;
}
