/**
 * @file DashboardSerializer.cpp
 * @brief 仪表盘布局序列化器实现
 *
 * JSON格式读写仪表盘面板配置，支持gauge/numeric/led/progressbar/chart
 * 五种面板类型的属性序列化与反序列化。
 */

#include "dashboard/DashboardSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QSaveFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDashboardSerializer, "dashboard.serializer")

// ─── DashboardItemConfig ────────────────────────────────────────────

QJsonObject DashboardItemConfig::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("widgetType")] = widgetType;
    obj[QStringLiteral("title")]      = title;
    obj[QStringLiteral("row")]        = row;
    obj[QStringLiteral("column")]     = column;
    obj[QStringLiteral("rowSpan")]    = rowSpan;
    obj[QStringLiteral("columnSpan")] = columnSpan;

    // 将QMap<QString, QVariant>展开为嵌套JSON对象
    QJsonObject propsObj;
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        propsObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    obj[QStringLiteral("properties")] = propsObj;

    return obj;
}

DashboardItemConfig DashboardItemConfig::fromJson(const QJsonObject& obj)
{
    DashboardItemConfig cfg;
    cfg.widgetType = obj[QStringLiteral("widgetType")].toString();
    cfg.title      = obj[QStringLiteral("title")].toString();
    cfg.row        = obj[QStringLiteral("row")].toInt(0);
    cfg.column     = obj[QStringLiteral("column")].toInt(0);
    cfg.rowSpan    = obj[QStringLiteral("rowSpan")].toInt(1);
    cfg.columnSpan = obj[QStringLiteral("columnSpan")].toInt(1);

    // 解析嵌套属性对象
    const QJsonObject propsObj = obj[QStringLiteral("properties")].toObject();
    for (auto it = propsObj.constBegin(); it != propsObj.constEnd(); ++it) {
        cfg.properties.insert(it.key(), it.value().toVariant());
    }

    return cfg;
}

// ─── DashboardSerializer ────────────────────────────────────────────

DashboardSerializer::DashboardSerializer(QObject* parent)
    : QObject(parent)
{
}

bool DashboardSerializer::saveToFile(const QString& filePath,
                                     const QString& name,
                                     int columns,
                                     const QList<DashboardItemConfig>& items)
{
    const QByteArray data = toJson(name, columns, items);
    if (data.isEmpty() && !items.isEmpty()) {
        // toJson失败且非空布局
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = tr("无法打开文件写入: %1").arg(file.errorString());
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    file.write(data);
    if (!file.commit()) {
        m_lastError = tr("写入文件失败: %1").arg(file.errorString());
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    qCInfo(lcDashboardSerializer) << "布局已保存至:" << filePath;
    return true;
}

bool DashboardSerializer::loadFromFile(const QString& filePath,
                                       QString& name,
                                       int& columns,
                                       QList<DashboardItemConfig>& items)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = tr("无法打开文件读取: %1").arg(file.errorString());
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        m_lastError = tr("文件为空: %1").arg(filePath);
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    return loadFromJson(data, name, columns, items);
}

bool DashboardSerializer::loadFromJson(const QByteArray& jsonData,
                                       QString& name,
                                       int& columns,
                                       QList<DashboardItemConfig>& items)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        m_lastError = tr("JSON解析失败: %1").arg(parseError.errorString());
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    const QJsonObject root = doc.object();

    // 版本校验
    const int version = root[QStringLiteral("version")].toInt(0);
    if (version != kVersion) {
        m_lastError = tr("不支持的布局版本: %1 (当前版本: %2)")
                          .arg(version)
                          .arg(kVersion);
        qCWarning(lcDashboardSerializer) << m_lastError;
        return false;
    }

    name    = root[QStringLiteral("name")].toString(tr("未命名布局"));
    columns = root[QStringLiteral("columns")].toInt(4);

    // 解析面板列表
    items.clear();
    const QJsonArray itemsArray = root[QStringLiteral("items")].toArray();
    items.reserve(itemsArray.size());
    for (const QJsonValue& val : itemsArray) {
        items.append(DashboardItemConfig::fromJson(val.toObject()));
    }

    qCInfo(lcDashboardSerializer) << "已加载布局:" << name
                                  << "面板数:" << items.size();
    return true;
}

QByteArray DashboardSerializer::toJson(const QString& name,
                                       int columns,
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

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

QString DashboardSerializer::lastError() const
{
    return m_lastError;
}

int DashboardSerializer::currentVersion()
{
    return kVersion;
}
