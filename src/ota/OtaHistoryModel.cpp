#include "ota/OtaHistoryModel.h"
#include "utils/SettingsManager.h"
#include "core/ThemeManager.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QColor>

OtaHistoryModel::OtaHistoryModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    loadFromSettings();
}

int OtaHistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_records.size();
}

int OtaHistoryModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

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
            return rec.success ? QObject::tr("Success") : QObject::tr("Failed");
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

QVariant OtaHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    switch (section) {
    case ColTime:     return tr("Time");
    case ColFileName: return tr("File");
    case ColProtocol: return tr("Protocol");
    case ColSize:     return tr("Size");
    case ColDuration: return tr("Duration");
    case ColResult:   return tr("Result");
    default:          return QVariant();
    }
}

void OtaHistoryModel::addRecord(const OtaRecord& record)
{
    // 先移除超出上限的旧记录（在插入之前），避免嵌套 beginInsert/beginRemove 导致视图混乱
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

void OtaHistoryModel::clearHistory()
{
    beginResetModel();
    m_records.clear();
    endResetModel();
    saveToSettings();
}

const OtaRecord& OtaHistoryModel::record(int row) const
{
    if (row < 0 || row >= m_records.size()) {
        static const OtaRecord empty;
        return empty;
    }
    return m_records.at(row);
}

int OtaHistoryModel::count() const
{
    return m_records.size();
}

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
