/**
 * @file OtaHistoryModel.cpp
 * @brief OTA升级历史记录表格模型实现
 *
 * 提供OTA升级历史的增删查和持久化功能。
 * 数据存储在 SettingsManager 中，以 JSON 数组格式序列化。
 * 模型最多保留 kMaxRecords=200 条记录，超出时移除最旧的记录。
 *
 * 列定义: 时间 | 文件名 | 协议 | 大小 | 耗时 | 结果
 * 特殊渲染: 结果列使用语义色（成功=绿/失败=红），失败行 Tooltip 显示错误详情
 */
#include "ota/history/OtaHistoryModel.h"
#include "utils/settings/SettingsManager.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

/** @brief 构造模型并从 SettingsManager 加载历史记录 @param parent 父对象 */
OtaHistoryModel::OtaHistoryModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    loadFromSettings();
}

// QAbstractItemModel接口(rowCount/columnCount/data/headerData)见 OtaHistoryModelIndex.cpp

/**
 * @brief 添加一条OTA记录到模型头部
 *
 * 新记录插入到第0行（最新的在前）。如果超出 kMaxRecords 上限，
 * 先移除最后一行（最旧的），避免嵌套 beginInsert/beginRemove 导致视图混乱。
 * 添加后自动持久化到 SettingsManager。
 *
 * @param record 要添加的OTA记录
 */
void OtaHistoryModel::addRecord(const OtaRecord& record)
{
    if (m_records.size() >= kMaxRecords) {
        int last = m_records.size() - 1;
        beginRemoveRows(QModelIndex(), last, last);
        m_records.removeLast();
        endRemoveRows();
        ++m_totalEntriesRemoved;  ///< 统计: 淘汰旧记录时递增
    }

    beginInsertRows(QModelIndex(), 0, 0);
    m_records.prepend(record);
    endInsertRows();
    ++m_totalEntriesAdded;  ///< 统计: 记录添加次数递增
    ++m_totalHistoryEntries;  ///< 统计: 历史条目总数递增
    saveToSettings();
}

/** @brief 清空所有历史记录并持久化 */
void OtaHistoryModel::clearHistory()
{
    if (!m_records.isEmpty()) {
        m_totalEntriesRemoved += static_cast<quint64>(m_records.size());  ///< 统计: 清空时累加移除数
    }
    beginResetModel();
    m_records.clear();
    endResetModel();
    saveToSettings();
}

/**
 * @brief 获取指定行的记录（只读引用）
 * @param row 行号（0~count-1），越界时返回静态空记录
 * @return 记录的常引用
 */
const OtaRecord& OtaHistoryModel::record(int row) const
{
    if (row < 0 || row >= m_records.size()) {
        static const OtaRecord empty;
        return empty;
    }
    return m_records.at(row);
}

/** @brief 返回记录总数 */
int OtaHistoryModel::count() const
{
    return m_records.size();
}

/** @brief 将所有记录序列化为 JSON 数组并写入 SettingsManager */
void OtaHistoryModel::saveToSettings()
{
    QJsonArray arr;
    for (const OtaRecord& rec : m_records) {
        QJsonObject obj;
        obj["fileName"] = rec.fileName;
        obj["protocol"] = rec.protocol;
        obj["fileSize"] = static_cast<qint64>(rec.fileSize);
        obj["startTime"] = rec.startTime.toString(Qt::ISODate);
        obj["durationMs"] = static_cast<qint64>(rec.durationMs);
        obj["success"] = rec.success;
        obj["errorMessage"] = rec.errorMessage;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    auto& settings = SettingsManager::instance();
    settings.set("ota_history/records", doc.toJson(QJsonDocument::Compact));
}

/** @brief 从 SettingsManager 读取 JSON 数组并反序列化为记录列表 */
void OtaHistoryModel::loadFromSettings()
{
    beginResetModel();
    m_records.clear();

    auto& settings = SettingsManager::instance();
    QByteArray json = settings.get("ota_history/records").toByteArray();
    if (json.isEmpty()) {
        endResetModel();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isArray()) {
        endResetModel();
        return;
    }

    QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        OtaRecord rec;
        rec.fileName = obj["fileName"].toString();
        rec.protocol = obj["protocol"].toString();
        rec.fileSize = obj["fileSize"].toInteger();
        rec.startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
        rec.durationMs = obj["durationMs"].toInteger();
        rec.success = obj["success"].toBool();
        rec.errorMessage = obj["errorMessage"].toString();
        m_records.append(rec);
    }

    endResetModel();
}

/**
 * @brief 获取成功传输次数
 * @return success == true 的记录数
 */
int OtaHistoryModel::successCount() const
{
    int count = 0;
    for (const auto& r : m_records) {
        if (r.success) ++count;
    }
    return count;
}

/**
 * @brief 获取失败传输次数
 * @return success == false 的记录数
 */
int OtaHistoryModel::failureCount() const
{
    return m_records.size() - successCount();
}

/**
 * @brief 获取成功率
 * @return 0.0~1.0，无记录时返回 0.0
 */
double OtaHistoryModel::successRate() const
{
    if (m_records.isEmpty()) return 0.0;
    return static_cast<double>(successCount()) / static_cast<double>(m_records.size());
}

/**
 * @brief 获取累计传输总字节数（仅成功的）
 * @return 成功传输的总字节数
 */
qint64 OtaHistoryModel::totalBytesTransferred() const
{
    qint64 total = 0;
    for (const auto& r : m_records) {
        if (r.success) total += r.fileSize;
    }
    return total;
}

/**
 * @brief 获取平均传输耗时（仅成功的）
 * @return 平均耗时（毫秒），无成功记录时返回 0
 */
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

/**
 * @brief 生成统计摘要文本
 * @return 格式化的多行OTA统计信息
 */
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

// ── 统计计数器实现 ──

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

/** @brief 重置历史记录统计计数器(不影响记录数据本身) */
void OtaHistoryModel::resetHistoryStatistics()
{
    m_totalEntriesAdded = 0;
    m_totalEntriesRemoved = 0;
    m_totalHistoryEntries = 0;
}

/** @brief 获取历史记录条目总数（当前记录数） @return 当前记录数 */
quint64 OtaHistoryModel::totalHistoryEntries() const
{
    return m_totalHistoryEntries;
}
