/**
 * @file ProtocolSchemaStats.cpp
 * @brief 协议帧结构统计计数器接口实现
 *
 * 从 ProtocolSchemaFields.cpp 拆分而来，包含:
 *   - 所有统计计数器getter(totalLoads/totalSaves/totalSchemas/...)
 *   - resetSchemaStatistics()
 *   - resetStats()别名
 *
 * 字段getter、JSON序列化、校验类型枚举转换见 ProtocolSchemaFields.cpp。
 */

#include "protocol/schema/ProtocolSchema.h"

// ============================================================================
// 统计计数器接口
// ============================================================================

/** @brief 获取协议定义加载总次数(loadFromJson/loadFromJsonData调用) @return 加载总次数 */
quint64 ProtocolSchema::totalLoads() const
{
    return m_totalLoads;
}

/** @brief 获取协议定义序列化保存总次数(toJson调用) @return 保存总次数 */
quint64 ProtocolSchema::totalSaves() const
{
    return m_totalSaves;
}

/** @brief 获取已加载的协议定义总数 @return 累计加载次数 */
quint64 ProtocolSchema::totalSchemas() const
{
    return m_totalSchemas;
}

/** @brief 获取所有已加载协议的字段总数 @return 累计字段数 */
quint64 ProtocolSchema::totalFieldCount() const
{
    return m_totalFieldCount;
}

/** @brief 获取历史最大协议定义大小(字节) @return 最大JSON字节数 */
quint64 ProtocolSchema::maxSchemaSize() const
{
    return m_maxSchemaSize;
}

/** @brief 获取协议定义校验执行总次数 @return 校验总次数 */
quint64 ProtocolSchema::totalValidations() const
{
    return m_totalValidations;
}

/** @brief 获取协议定义校验失败次数 @return 校验错误计数 */
quint64 ProtocolSchema::validationErrors() const
{
    return m_validationErrors;
}

/** @brief 获取协议定义加载失败次数 @return 加载失败计数 */
quint64 ProtocolSchema::totalSchemaErrors() const
{
    return m_totalSchemaErrors;
}

/** @brief 获取当前活跃的协议定义数量 @return 活跃协议数 */
quint64 ProtocolSchema::totalActiveSchemas() const
{
    return m_totalActiveSchemas;
}

/** @brief 获取通过编程接口(addField/setFraming)构造协议的总次数 @return 构造次数 */
quint64 ProtocolSchema::totalBuilds() const
{
    return m_totalBuilds;
}

/** @brief 重置所有Schema统计计数器(加载数/保存数/字段数/最大大小/校验/错误/活跃数/构造数) */
void ProtocolSchema::resetSchemaStatistics()
{
    m_totalLoads = 0;
    m_totalSaves = 0;
    m_totalSchemas = 0;
    m_totalFieldCount = 0;
    m_maxSchemaSize = 0;
    m_totalValidations = 0;
    m_validationErrors = 0;
    m_totalSchemaErrors = 0;
    m_totalActiveSchemas = 0;
    m_totalBuilds = 0;
}

/** @brief 重置所有统计计数器（别名，调用resetSchemaStatistics） */
void ProtocolSchema::resetStats()
{
    resetSchemaStatistics();
}
