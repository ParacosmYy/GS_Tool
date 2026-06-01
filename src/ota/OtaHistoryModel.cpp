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
#include "ota/OtaHistoryModel.h"
#include "utils/SettingsManager.h"
#include "core/ThemeManager.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QColor>

/** @brief 构造模型并从 SettingsManager 加载历史记录 @param parent 父对象 */
OtaHistoryModel::OtaHistoryModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    loadFromSettings();
}

/** @brief 返回记录行数 @param parent 父索引（表格模型中无效） */
int OtaHistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_records.size();
}

/** @brief 返回列数（固定为 ColCount=6） @param parent 父索引 */
int OtaHistoryModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

/**
 * @brief 返回指定单元格的数据
 *
 * 支持的角色:
 *   - DisplayRole: 格式化显示文本（时间/文件名/协议/大小/耗时/结果）
 *   - ForegroundRole: 结果列使用语义色（成功=Success, 失败=Error）
 *   - ToolTipRole: 失败行显示错误详情
 *
 * @param index 单元格索引 @param role 数据角色 @return 格式化后的数据
 */
QVariant OtaHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size()) return QVariant();

    const OtaRecord& rec = m_records.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColTime:
            return rec.startTime.toString("yyyy-MM-dd HH:mm:ss");
        case ColFileName:
            return rec.fileName;
        case ColProtocol:
            return rec.protocol.toUpper();
        case ColSize: {
            if (rec.fileSize < 1024) return QString("%1 B").arg(rec.fileSize);
            if (rec.fileSize < 1024 * 1024) return QString("%1 KB").arg(rec.fileSize / 1024.0, 0, 'f', 1);
            return QString("%1 MB").arg(rec.fileSize / (1024.0 * 1024.0), 0, 'f', 2);
        }
        case ColDuration:
            if (rec.durationMs < 1000) return QString("%1 ms").arg(rec.durationMs);
            return QString("%1 s").arg(rec.durationMs / 1000.0, 0, 'f', 1);
        case ColResult:
            return rec.success ? QObject::tr("成功") : QObject::tr("失败");
        }
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColResult) {
            return rec.success
                ? ThemeManager::instance().color(ThemeManager::SemanticColor::Success)
                : ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
        }
    }

    if (role == Qt::ToolTipRole && !rec.success) {
        return rec.errorMessage;
    }

    return QVariant();
}

/**
 * @brief 返回水平表头文本
 * @param section 列号 @param orientation 方向 @param role 数据角色
 * @return 列标题文本（仅支持 Horizontal + DisplayRole）
 */
QVariant OtaHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    switch (section) {
    case ColTime:     return tr("时间");
    case ColFileName: return tr("文件");
    case ColProtocol: return tr("协议");
    case ColSize:     return tr("大小");
    case ColDuration: return tr("耗时");
    case ColResult:   return tr("结果");
    default:          return QVariant();
    }
}

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
    }

    beginInsertRows(QModelIndex(), 0, 0);
    m_records.prepend(record);
    endInsertRows();
    saveToSettings();
}

/** @brief 清空所有历史记录并持久化 */
void OtaHistoryModel::clearHistory()
{
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
