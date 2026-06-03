/**
 * @file DashboardSerializer.cpp
 * @brief 仪表盘布局序列化器实现 — JSON读写+QSettings配置文件系统
 *
 * 支持gauge/numeric/led/progressbar/chart五种面板类型的
 * 完整属性序列化、QSettings命名配置文件管理与JSON导入导出。
 */

#include "dashboard/DashboardSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDashboardSerializer, "dashboard.serializer")

/// QSettings配置文件系统常量键
static const QString kSettingsGroup  = QStringLiteral("DashboardProfiles");
static const QString kProfilesKey    = QStringLiteral("profiles");
static const QString kCurrentKey     = QStringLiteral("currentProfile");
static const QString kColumnsKey     = QStringLiteral("columns");
static const QString kItemsKey       = QStringLiteral("items");
static const QString kNameKey        = QStringLiteral("name");

// ─── DashboardItemConfig ────────────────────────────────────────────

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

// ─── DashboardSerializer 核心文件操作 ────────────────────────────────

/** @brief 构造函数 @param parent 父对象 */
DashboardSerializer::DashboardSerializer(QObject* parent) : QObject(parent) {}

/** @brief 保存布局到JSON文件 @return true=成功 */
bool DashboardSerializer::saveToFile(const QString& filePath,
                                     const QString& name, int columns,
                                     const QList<DashboardItemConfig>& items)
{
    const QByteArray data = toJson(name, columns, items);
    if (data.isEmpty() && !items.isEmpty()) { ++m_totalErrors; return false; }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = tr("无法打开文件写入: %1").arg(file.errorString());
        ++m_totalErrors; return false;
    }
    file.write(data);
    if (!file.commit()) {
        m_lastError = tr("写入文件失败: %1").arg(file.errorString());
        ++m_totalErrors; return false;
    }

    qCInfo(lcDashboardSerializer) << "布局已保存至:" << filePath;
    ++m_totalSaves; ++m_totalExports;
    emit layoutSaved(filePath);
    return true;
}

/** @brief 从JSON文件加载布局 @return true=成功 */
bool DashboardSerializer::loadFromFile(const QString& filePath,
                                       QString& name, int& columns,
                                       QList<DashboardItemConfig>& items)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = tr("无法打开文件读取: %1").arg(file.errorString());
        ++m_totalErrors; return false;
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        m_lastError = tr("文件为空: %1").arg(filePath);
        ++m_totalErrors; return false;
    }
    ++m_totalImports;
    return loadFromJson(data, name, columns, items);
}

/** @brief 从JSON字节数组解析布局 @return true=成功 */
bool DashboardSerializer::loadFromJson(const QByteArray& jsonData,
                                       QString& name, int& columns,
                                       QList<DashboardItemConfig>& items)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        m_lastError = tr("JSON解析失败: %1").arg(parseError.errorString());
        ++m_totalErrors;
        return false;
    }

    const QJsonObject root = doc.object();
    const int version = root[QStringLiteral("version")].toInt(0);
    if (version != kVersion) {
        m_lastError = tr("不支持的布局版本: %1 (当前版本: %2)").arg(version).arg(kVersion);
        ++m_totalErrors;
        return false;
    }
    name    = root[QStringLiteral("name")].toString(tr("未命名布局"));
    columns = root[QStringLiteral("columns")].toInt(4);
    items.clear();
    const QJsonArray itemsArray = root[QStringLiteral("items")].toArray();
    items.reserve(itemsArray.size());
    for (const QJsonValue& val : itemsArray)
        items.append(DashboardItemConfig::fromJson(val.toObject()));
    qCInfo(lcDashboardSerializer) << "已加载布局:" << name << "面板数:" << items.size();
    ++m_totalLoads;
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

    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

QString DashboardSerializer::lastError() const { return m_lastError; }
int DashboardSerializer::currentVersion() { return kVersion; }

/** @brief 验证布局配置有效性 @return 错误信息列表（空=通过） */
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

        if (!validTypes.contains(item.widgetType))
            errors.append(tr("面板 #%1: 未知类型 '%2'").arg(i + 1).arg(item.widgetType));
        if (item.row < 0)
            errors.append(tr("面板 #%1 '%2': 行号不能为负").arg(i + 1).arg(item.title));
        if (item.column < 0)
            errors.append(tr("面板 #%1 '%2': 列号不能为负").arg(i + 1).arg(item.title));
        if (item.rowSpan < 1)
            errors.append(tr("面板 #%1 '%2': 行跨度必须 >= 1").arg(i + 1).arg(item.title));
        if (item.columnSpan < 1)
            errors.append(tr("面板 #%1 '%2': 列跨度必须 >= 1").arg(i + 1).arg(item.title));
        if (item.column + item.columnSpan > columns)
            errors.append(tr("面板 #%1 '%2': 超出网格边界 (列 %3+%4 > %5)")
                              .arg(i + 1).arg(item.title)
                              .arg(item.column).arg(item.columnSpan).arg(columns));
    }

    /* 重叠检查 */
    for (int i = 0; i < items.size(); ++i) {
        for (int j = i + 1; j < items.size(); ++j) {
            const auto& a = items.at(i);
            const auto& b = items.at(j);
            bool rowOv = (a.row < b.row + b.rowSpan) && (b.row < a.row + a.rowSpan);
            bool colOv = (a.column < b.column + b.columnSpan) && (b.column < a.column + a.columnSpan);
            if (rowOv && colOv)
                errors.append(tr("面板 #%1 '%2' 与面板 #%3 '%4' 重叠")
                                  .arg(i + 1).arg(a.title).arg(j + 1).arg(b.title));
        }
    }

    if (!errors.isEmpty())
        emit const_cast<DashboardSerializer*>(this)->validationFailed(errors);

    return errors;
}

/** @brief 列出目录中所有.json布局文件 @return 文件路径列表 */
QStringList DashboardSerializer::listLayoutFiles(const QString& dirPath) const
{
    QStringList result;
    QDir dir(dirPath);
    if (!dir.exists()) return result;

    const QFileInfoList entries = dir.entryInfoList(
        QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
    for (const QFileInfo& fi : entries)
        result.append(fi.absoluteFilePath());
    return result;
}

/** @brief 删除布局文件（备份后删除） @return true=成功 */
bool DashboardSerializer::deleteLayout(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        m_lastError = tr("文件不存在: %1").arg(filePath);
        ++m_totalErrors;
        return false;
    }

    const QString backupPath = filePath + QStringLiteral(".bak");
    if (QFile::exists(backupPath)) QFile::remove(backupPath);
    if (!file.copy(backupPath))
        qCWarning(lcDashboardSerializer) << "备份失败:" << file.errorString();

    if (!file.remove()) {
        m_lastError = tr("删除失败: %1").arg(file.errorString());
        ++m_totalErrors;
        return false;
    }

    ++m_totalDeletes;
    return true;
}

// ─── QSettings命名配置文件系统 ──────────────────────────────────────

/** @brief 保存布局到QSettings命名配置文件 @return true=成功 */
bool DashboardSerializer::saveToProfile(const QString& profileName,
                                        const QString& name, int columns,
                                        const QList<DashboardItemConfig>& items)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors; return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    /* 序列化面板配置为JSON数组 */
    QJsonArray itemsArray;
    for (const DashboardItemConfig& item : items)
        itemsArray.append(item.toJson());
    const QByteArray itemsJson = QJsonDocument(itemsArray).toJson(QJsonDocument::Compact);

    /* 写入配置文件数据 */
    settings.beginGroup(profileName);
    settings.setValue(kNameKey, name);
    settings.setValue(kColumnsKey, columns);
    settings.setValue(kItemsKey, QString::fromUtf8(itemsJson));
    settings.endGroup();
    settings.endGroup();

    ensureProfileListContains(profileName);

    qCInfo(lcDashboardSerializer) << "配置文件已保存:" << profileName
                                  << "面板数:" << items.size();
    ++m_totalSaves; ++m_totalProfileSaves;
    emit profileSaved(profileName);
    return true;
}

/** @brief 从QSettings命名配置文件加载布局 @return true=成功 */
bool DashboardSerializer::loadFromProfile(const QString& profileName,
                                          QString& name, int& columns,
                                          QList<DashboardItemConfig>& items)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors; return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(profileName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(profileName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    settings.beginGroup(profileName);
    name    = settings.value(kNameKey, tr("未命名布局")).toString();
    columns = settings.value(kColumnsKey, 4).toInt();
    const QString itemsJson = settings.value(kItemsKey).toString();
    settings.endGroup();
    settings.endGroup();

    /* 解析JSON面板配置 */
    items.clear();
    if (!itemsJson.isEmpty()) {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8(), &parseError);
        if (doc.isNull()) {
            m_lastError = tr("配置文件 '%1' 数据损坏: %2")
                              .arg(profileName).arg(parseError.errorString());
            ++m_totalErrors;
            return false;
        }
        const QJsonArray arr = doc.array();
        items.reserve(arr.size());
        for (const QJsonValue& val : arr)
            items.append(DashboardItemConfig::fromJson(val.toObject()));
    }

    qCInfo(lcDashboardSerializer) << "已加载配置文件:" << profileName
                                  << "面板数:" << items.size();
    ++m_totalLoads; ++m_totalProfileLoads;
    emit profileLoaded(profileName, items.size());
    return true;
}

/** @brief 删除命名配置文件 @return true=成功 */
bool DashboardSerializer::deleteProfile(const QString& profileName)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors;
        return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(profileName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(profileName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    settings.beginGroup(profileName);
    settings.remove(QString());
    settings.endGroup();
    settings.endGroup();

    removeProfileListEntry(profileName);
    if (currentProfile() == profileName)
        setCurrentProfile(QString());

    qCInfo(lcDashboardSerializer) << "已删除配置文件:" << profileName;
    ++m_totalDeletes;
    emit profileDeleted(profileName);
    return true;
}

/** @brief 重命名配置文件 @return true=成功 */
bool DashboardSerializer::renameProfile(const QString& oldName, const QString& newName)
{
    if (oldName.isEmpty() || newName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors;
        return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(oldName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(oldName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    /* 读取旧配置 → 写入新配置 → 删除旧配置 */
    settings.beginGroup(oldName);
    const QString layoutName = settings.value(kNameKey).toString();
    const int columns = settings.value(kColumnsKey).toInt();
    const QString itemsJson = settings.value(kItemsKey).toString();
    settings.endGroup();

    settings.beginGroup(newName);
    settings.setValue(kNameKey, layoutName);
    settings.setValue(kColumnsKey, columns);
    settings.setValue(kItemsKey, itemsJson);
    settings.endGroup();

    settings.beginGroup(oldName);
    settings.remove(QString());
    settings.endGroup();
    settings.endGroup();

    removeProfileListEntry(oldName);
    ensureProfileListContains(newName);
    if (currentProfile() == oldName) setCurrentProfile(newName);

    qCInfo(lcDashboardSerializer) << "已重命名:" << oldName << "->" << newName;
    return true;
}

QStringList DashboardSerializer::listProfiles() const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const QStringList names = settings.value(kProfilesKey).toStringList();
    settings.endGroup();
    return names;
}

void DashboardSerializer::setCurrentProfile(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue(kCurrentKey, profileName);
    settings.endGroup();
    emit currentProfileChanged(profileName);
}

QString DashboardSerializer::currentProfile() const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const QString name = settings.value(kCurrentKey).toString();
    settings.endGroup();
    return name;
}

bool DashboardSerializer::hasProfile(const QString& profileName) const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const bool exists = settings.childGroups().contains(profileName);
    settings.endGroup();
    return exists;
}

void DashboardSerializer::ensureProfileListContains(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    QStringList names = settings.value(kProfilesKey).toStringList();
    if (!names.contains(profileName)) {
        names.append(profileName);
        settings.setValue(kProfilesKey, names);
    }
    settings.endGroup();
}

void DashboardSerializer::removeProfileListEntry(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    QStringList names = settings.value(kProfilesKey).toStringList();
    names.removeAll(profileName);
    settings.setValue(kProfilesKey, names);
    settings.endGroup();
}

// ─── 统计接口 ───────────────────────────────────────────────────────

quint64 DashboardSerializer::totalSaves() const        { return m_totalSaves; }
quint64 DashboardSerializer::totalLoads() const        { return m_totalLoads; }
quint64 DashboardSerializer::totalValidations() const  { return m_totalValidations; }
quint64 DashboardSerializer::totalDeletes() const      { return m_totalDeletes; }
quint64 DashboardSerializer::totalErrors() const       { return m_totalErrors; }
quint64 DashboardSerializer::totalProfileSaves() const { return m_totalProfileSaves; }
quint64 DashboardSerializer::totalProfileLoads() const { return m_totalProfileLoads; }
quint64 DashboardSerializer::totalExports() const      { return m_totalExports; }
quint64 DashboardSerializer::totalImports() const      { return m_totalImports; }

void DashboardSerializer::resetSerializerStatistics()
{
    m_totalSaves = 0; m_totalLoads = 0; m_totalValidations = 0;
    m_totalDeletes = 0; m_totalErrors = 0; m_totalProfileSaves = 0;
    m_totalProfileLoads = 0; m_totalExports = 0; m_totalImports = 0;
}
