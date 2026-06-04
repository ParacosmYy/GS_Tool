/**
 * @file DashboardSerializerStats.cpp
 * @brief 仪表盘布局序列化器 — 统计查询方法与重置
 *
 * 从 DashboardSerializer.cpp 拆分而来，包含所有序列化器统计计数器的
 * 查询方法和统一重置操作:
 *   - totalSaves()/totalLoads()/totalValidations()/totalDeletes()/totalErrors()
 *   - totalProfileSaves()/totalProfileLoads()
 *   - totalExports()/totalImports()
 *   - totalBytesSerialized()/totalBytesDeserialized()
 *   - serializationErrors()/deserializationErrors()
 *   - totalSerializations()/totalDeserializations()
 *   - totalSerializationErrors()/totalBytesWritten()/totalBytesRead()
 *   - maxProfileVersionSaved()/minProfileVersionLoaded()
 *   - resetSerializerStatistics()
 */

#include "dashboard/DashboardSerializer.h"

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计保存操作次数 @return 保存计数 */
quint64 DashboardSerializer::totalSaves() const        { return m_totalSaves; }

/** @brief 获取累计加载操作次数 @return 加载计数 */
quint64 DashboardSerializer::totalLoads() const        { return m_totalLoads; }

/** @brief 获取累计验证操作次数 @return 验证计数 */
quint64 DashboardSerializer::totalValidations() const  { return m_totalValidations; }

/** @brief 获取累计删除操作次数 @return 删除计数 */
quint64 DashboardSerializer::totalDeletes() const      { return m_totalDeletes; }

/** @brief 获取累计错误次数 @return 错误计数 */
quint64 DashboardSerializer::totalErrors() const       { return m_totalErrors; }

/** @brief 获取累计QSettings配置文件保存次数 @return 配置保存计数 */
quint64 DashboardSerializer::totalProfileSaves() const { return m_totalProfileSaves; }

/** @brief 获取累计QSettings配置文件加载次数 @return 配置加载计数 */
quint64 DashboardSerializer::totalProfileLoads() const { return m_totalProfileLoads; }

/** @brief 获取累计JSON导出次数 @return 导出计数 */
quint64 DashboardSerializer::totalExports() const      { return m_totalExports; }

/** @brief 获取累计JSON导入次数 @return 导入计数 */
quint64 DashboardSerializer::totalImports() const      { return m_totalImports; }

/** @brief 获取已保存配置文件的最高版本号 @return 最高版本号 */
int DashboardSerializer::maxProfileVersionSaved() const { return m_maxProfileVersionSaved; }

/** @brief 获取已加载配置文件的最低版本号 @return 最低版本号 */
int DashboardSerializer::minProfileVersionLoaded() const { return m_minProfileVersionLoaded; }

/** @brief 获取累计序列化输出字节数 @return 字节总数 */
quint64 DashboardSerializer::totalBytesSerialized() const { return m_totalBytesSerialized; }

/** @brief 获取累计反序列化输入字节数 @return 字节总数 */
quint64 DashboardSerializer::totalBytesDeserialized() const { return m_totalBytesDeserialized; }

/** @brief 获取累计序列化错误次数 @return 错误次数 */
quint64 DashboardSerializer::serializationErrors() const { return m_serializationErrors; }

/** @brief 获取累计反序列化错误次数 @return 错误次数 */
quint64 DashboardSerializer::deserializationErrors() const { return m_deserializationErrors; }

/** @brief 获取累计序列化操作次数 @return 序列化总数 */
quint64 DashboardSerializer::totalSerializations() const { return m_totalSerializations; }

/** @brief 获取累计反序列化操作次数 @return 反序列化总数 */
quint64 DashboardSerializer::totalDeserializations() const { return m_totalDeserializations; }

/** @brief 获取累计序列化错误次数(保存失败) @return 错误次数 */
quint64 DashboardSerializer::totalSerializationErrors() const { return m_totalSerializationErrors; }

/** @brief 获取累计写入文件字节数 @return 字节总数 */
quint64 DashboardSerializer::totalBytesWritten() const { return m_totalBytesWritten; }

/** @brief 获取累计读取文件字节数 @return 字节总数 */
quint64 DashboardSerializer::totalBytesRead() const { return m_totalBytesRead; }

/** @brief 重置所有序列化器统计计数器(保存/加载/验证/删除/错误/导出/导入/字节/版本) */
void DashboardSerializer::resetSerializerStatistics()
{
    m_totalSaves = 0; m_totalLoads = 0; m_totalValidations = 0;
    m_totalDeletes = 0; m_totalErrors = 0; m_totalProfileSaves = 0;
    m_totalProfileLoads = 0; m_totalExports = 0; m_totalImports = 0;
    m_maxProfileVersionSaved = 0; m_minProfileVersionLoaded = 0;
    m_hasLoadedVersion = false;
    m_totalBytesSerialized = 0; m_totalBytesDeserialized = 0;
    m_serializationErrors = 0; m_deserializationErrors = 0;
    m_totalSerializations = 0; m_totalDeserializations = 0;
    m_totalSerializationErrors = 0; m_totalBytesWritten = 0; m_totalBytesRead = 0;
}
