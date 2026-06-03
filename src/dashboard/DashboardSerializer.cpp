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
#include <QDir>
#include <QFileInfo>
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
    ++m_totalSaves;
    emit layoutSaved(filePath);
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
    ++m_totalLoads;
    emit layoutLoaded(name, items.size());
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

/**
 * @brief 验证布局配置的有效性
 *
 * 检查项:
 *   1. 面板类型是否合法
 *   2. 网格坐标是否为非负值
 *   3. 跨度是否 ≥ 1
 *   4. 面板是否超出网格边界
 *   5. 面板之间是否有重叠
 *
 * @param items 面板配置列表
 * @param columns 网格列数
 * @return 错误信息列表（空列表表示验证通过）
 */
QStringList DashboardSerializer::validateLayout(
    const QList<DashboardItemConfig>& items, int columns) const
{
    ++m_totalValidations;
    QStringList errors;
    const QStringList validTypes = {
        QStringLiteral("gauge"), QStringLiteral("numeric"),
        QStringLiteral("led"), QStringLiteral("progressbar"),
        QStringLiteral("chart")
    };

    for (int i = 0; i < items.size(); ++i) {
        const auto& item = items.at(i);

        /* 类型检查 */
        if (!validTypes.contains(item.widgetType)) {
            errors.append(tr("面板 #%1: 未知类型 '%2'")
                              .arg(i + 1).arg(item.widgetType));
        }

        /* 坐标非负检查 */
        if (item.row < 0) {
            errors.append(tr("面板 #%1 '%2': 行号不能为负")
                              .arg(i + 1).arg(item.title));
        }
        if (item.column < 0) {
            errors.append(tr("面板 #%1 '%2': 列号不能为负")
                              .arg(i + 1).arg(item.title));
        }

        /* 跨度检查 */
        if (item.rowSpan < 1) {
            errors.append(tr("面板 #%1 '%2': 行跨度必须 ≥ 1")
                              .arg(i + 1).arg(item.title));
        }
        if (item.columnSpan < 1) {
            errors.append(tr("面板 #%1 '%2': 列跨度必须 ≥ 1")
                              .arg(i + 1).arg(item.title));
        }

        /* 列边界检查 */
        if (item.column + item.columnSpan > columns) {
            errors.append(tr("面板 #%1 '%2': 超出网格边界 (列 %3+%4 > %5)")
                              .arg(i + 1).arg(item.title)
                              .arg(item.column).arg(item.columnSpan)
                              .arg(columns));
        }
    }

    /* 重叠检查: 比较每对面板 */
    for (int i = 0; i < items.size(); ++i) {
        for (int j = i + 1; j < items.size(); ++j) {
            const auto& a = items.at(i);
            const auto& b = items.at(j);

            bool rowOverlap = (a.row < b.row + b.rowSpan) &&
                              (b.row < a.row + a.rowSpan);
            bool colOverlap = (a.column < b.column + b.columnSpan) &&
                              (b.column < a.column + a.columnSpan);

            if (rowOverlap && colOverlap) {
                errors.append(tr("面板 #%1 '%2' 与面板 #%3 '%4' 重叠")
                                  .arg(i + 1).arg(a.title)
                                  .arg(j + 1).arg(b.title));
            }
        }
    }

    if (!errors.isEmpty()) {
        emit const_cast<DashboardSerializer*>(this)->validationFailed(errors);
    }

    return errors;
}

/**
 * @brief 列出目录中所有布局文件
 *
 * 查找指定目录下所有 .json 文件作为布局文件候选。
 *
 * @param dirPath 目录路径
 * @return 文件路径列表
 */
QStringList DashboardSerializer::listLayoutFiles(const QString& dirPath) const
{
    QStringList result;
    QDir dir(dirPath);
    if (!dir.exists()) {
        return result;
    }

    const QFileInfoList entries = dir.entryInfoList(
        QStringList() << QStringLiteral("*.json"),
        QDir::Files, QDir::Name);

    for (const QFileInfo& fi : entries) {
        result.append(fi.absoluteFilePath());
    }

    return result;
}

/**
 * @brief 删除指定布局文件
 *
 * 创建 .bak 备份后删除原文件。
 *
 * @param filePath 布局文件路径
 * @return true 删除成功
 */
bool DashboardSerializer::deleteLayout(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        m_lastError = tr("文件不存在: %1").arg(filePath);
        return false;
    }

    /* 创建备份 */
    const QString backupPath = filePath + QStringLiteral(".bak");
    if (QFile::exists(backupPath)) {
        QFile::remove(backupPath);
    }
    if (!file.copy(backupPath)) {
        qCWarning(lcDashboardSerializer)
            << "备份失败，继续删除:" << file.errorString();
    }

    /* 删除原文件 */
    if (!file.remove()) {
        m_lastError = tr("删除失败: %1").arg(file.errorString());
        return false;
    }

    qCInfo(lcDashboardSerializer) << "已删除布局:" << filePath;
    ++m_totalDeletes;
    return true;
}

quint64 DashboardSerializer::totalSaves() const { return m_totalSaves; }
quint64 DashboardSerializer::totalLoads() const { return m_totalLoads; }
quint64 DashboardSerializer::totalValidations() const { return m_totalValidations; }
quint64 DashboardSerializer::totalDeletes() const { return m_totalDeletes; }

void DashboardSerializer::resetSerializerStatistics()
{
    m_totalSaves = 0;
    m_totalLoads = 0;
    m_totalValidations = 0;
    m_totalDeletes = 0;
}
