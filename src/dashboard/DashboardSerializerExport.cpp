/**
 * @file DashboardSerializerExport.cpp
 * @brief 仪表盘布局JSON序列化/反序列化实现 — toJson与fromJson方法
 *
 * 从DashboardSerializer.cpp拆分而来，集中管理DashboardItemConfig与
 * DashboardSerializer的JSON转换逻辑，降低单文件体积。
 */

#include "dashboard/DashboardSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcDashboardSerializer)

// ─── DashboardItemConfig JSON转换 ─────────────────────────────────────

/** @brief 序列化为JSON对象 @return QJsonObject */
QJsonObject DashboardItemConfig::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("widgetType")] = widgetType;
    obj[QStringLiteral("title")]      = title;
    obj[QStringLiteral("row")]        = row;
    obj[QStringLiteral("column")]     = column;
    obj[QStringLiteral("rowSpan")]    = rowSpan;
    obj[QStringLiteral("columnSpan")] = columnSpan;
    QJsonObject propsObj;
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
        propsObj[it.key()] = QJsonValue::fromVariant(it.value());
    obj[QStringLiteral("properties")] = propsObj;
    return obj;
}

/** @brief 从JSON对象反序列化 @param obj JSON对象 @return DashboardItemConfig */
DashboardItemConfig DashboardItemConfig::fromJson(const QJsonObject& obj)
{
    DashboardItemConfig cfg;
    cfg.widgetType = obj[QStringLiteral("widgetType")].toString();
    cfg.title      = obj[QStringLiteral("title")].toString();
    cfg.row        = obj[QStringLiteral("row")].toInt(0);
    cfg.column     = obj[QStringLiteral("column")].toInt(0);
    cfg.rowSpan    = obj[QStringLiteral("rowSpan")].toInt(1);
    cfg.columnSpan = obj[QStringLiteral("columnSpan")].toInt(1);
    const QJsonObject propsObj = obj[QStringLiteral("properties")].toObject();
    for (auto it = propsObj.constBegin(); it != propsObj.constEnd(); ++it)
        cfg.properties.insert(it.key(), it.value().toVariant());
    return cfg;
}

// ─── DashboardSerializer JSON解析与生成 ───────────────────────────────

/** @brief 从JSON字节数组解析布局 @return true=成功 */
bool DashboardSerializer::loadFromJson(const QByteArray& jsonData,
                                       QString& name, int& columns,
                                       QList<DashboardItemConfig>& items)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        m_lastError = tr("JSON解析失败: %1").arg(parseError.errorString());
        ++m_stats.totalErrors; ++m_stats.deserializationErrors; ++m_stats.totalLoadFailures;
        return false;
    }

    const QJsonObject root = doc.object();
    const int version = root[QStringLiteral("version")].toInt(0);
    if (version != kVersion) {
        m_lastError = tr("不支持的布局版本: %1 (当前版本: %2)").arg(version).arg(kVersion);
        ++m_stats.totalErrors; ++m_stats.deserializationErrors; ++m_stats.totalLoadFailures;
        return false;
    }

    /* 统计：跟踪已加载配置的版本范围 */
    if (!m_stats.hasLoadedVersion) {
        m_stats.minProfileVersionLoaded = version;
        m_stats.hasLoadedVersion = true;
    } else {
        m_stats.minProfileVersionLoaded = qMin(m_stats.minProfileVersionLoaded, version);
    }
    name    = root[QStringLiteral("name")].toString(tr("未命名布局"));
    columns = root[QStringLiteral("columns")].toInt(4);
    items.clear();
    const QJsonArray itemsArray = root[QStringLiteral("items")].toArray();
    items.reserve(itemsArray.size());
    for (const QJsonValue& val : itemsArray)
        items.append(DashboardItemConfig::fromJson(val.toObject()));
    qCInfo(lcDashboardSerializer) << "已加载布局:" << name << "面板数:" << items.size();
    ++m_stats.totalLoads; ++m_stats.totalDeserializations;
    emit layoutLoaded(name, items.size());
    return true;
}

/** @brief 序列化为JSON字节数组 @return JSON字节数组 */
QByteArray DashboardSerializer::toJson(const QString& name, int columns,
                                       const QList<DashboardItemConfig>& items)
{
    QJsonObject root;
    root[QStringLiteral("version")] = kVersion;
    root[QStringLiteral("name")]    = name;
    root[QStringLiteral("columns")] = columns;

    QJsonArray itemsArray;
    for (const DashboardItemConfig& item : items) {
        itemsArray.append(item.toJson());
    }
    root[QStringLiteral("items")] = itemsArray;

    ++m_stats.totalSerializations;
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}
