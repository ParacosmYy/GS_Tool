/**
 * @file OtaHistoryModelStats.cpp
 * @brief OTA升级历史模型 - 统计分析与重置接口实现
 *
 * 从 OtaHistoryModel.cpp 拆分而来，包含成功率/字节数/耗时统计、
 * 摘要生成、统计getter和resetHistoryStatistics方法。
 */

#include "ota/history/OtaHistoryModel.h"

/** @brief 获取成功传输次数 @return success == true 的记录数 */
int OtaHistoryModel::successCount() const
{
    int count = 0;
    for (const auto& r : m_records) {
        if (r.success) ++count;
    }
    return count;
}

/** @brief 获取失败传输次数 @return success == false 的记录数 */
int OtaHistoryModel::failureCount() const
{
    return m_records.size() - successCount();
}

/** @brief 获取成功率 @return 0.0~1.0，无记录时返回 0.0 */
double OtaHistoryModel::successRate() const
{
    if (m_records.isEmpty()) return 0.0;
    return static_cast<double>(successCount()) / static_cast<double>(m_records.size());
}

/** @brief 获取累计传输总字节数（仅成功的） @return 成功传输的总字节数 */
qint64 OtaHistoryModel::totalBytesTransferred() const
{
    qint64 total = 0;
    for (const auto& r : m_records) {
        if (r.success) total += r.fileSize;
    }
    return total;
}

/** @brief 获取平均传输耗时（仅成功的） @return 平均耗时（毫秒），无成功记录时返回 0 */
qint64 OtaHistoryModel::averageDurationMs() const
{
    qint64 total = 0;
    int count = 0;
    for (const auto& r : m_records) {
        if (r.success) {
            total += r.durationMs;
            ++count;
        }
    }
    return count > 0 ? total / count : 0;
}

/** @brief 生成统计摘要文本 @return 格式化的多行OTA统计信息 */
QString OtaHistoryModel::statisticsSummary() const
{
    QString summary;
    summary += tr("总传输次数: %1\n").arg(m_records.size());
    summary += tr("成功: %1  失败: %2\n").arg(successCount()).arg(failureCount());

    if (!m_records.isEmpty()) {
        summary += tr("成功率: %1%\n").arg(successRate() * 100, 0, 'f', 1);
    }

    const qint64 totalBytes = totalBytesTransferred();
    if (totalBytes > 0) {
        summary += tr("累计传输: %1\n").arg(
            QString::number(static_cast<double>(totalBytes) / (1024.0 * 1024.0), 'f', 1) + " MB");
    }

    const qint64 avgMs = averageDurationMs();
    if (avgMs > 0) {
        summary += tr("平均耗时: %1秒\n").arg(avgMs / 1000.0, 0, 'f', 1);
    }

    return summary.trimmed();
}

/** @brief 获取历史记录添加总次数 @return 累计添加次数 */
quint64 OtaHistoryModel::totalEntriesAdded() const
{
    return m_totalEntriesAdded;
}

/** @brief 获取历史记录移除总次数(含淘汰) @return 累计移除次数 */
quint64 OtaHistoryModel::totalEntriesRemoved() const
{
    return m_totalEntriesRemoved;
}

/** @brief 获取历史记录条目总数（当前记录数） @return 当前记录数 */
quint64 OtaHistoryModel::totalHistoryEntries() const
{
    return m_totalHistoryEntries;
}

/** @brief 重置历史记录统计计数器(不影响记录数据本身) */
void OtaHistoryModel::resetHistoryStatistics()
{
    m_totalEntriesAdded = 0;
    m_totalEntriesRemoved = 0;
    m_totalHistoryEntries = 0;
}
