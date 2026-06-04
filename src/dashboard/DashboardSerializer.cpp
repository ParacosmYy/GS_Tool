/**
 * @file DashboardSerializer.cpp
 * @brief 仪表盘布局序列化器实现 — JSON读写+QSettings配置文件系统
 *
 * 支持gauge/numeric/led/progressbar/chart五种面板类型的
 * 完整属性序列化、QSettings命名配置文件管理与JSON导入导出。
 */

#include "dashboard/DashboardSerializer.h"

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

// ─── DashboardItemConfig JSON转换见 DashboardSerializerExport.cpp ───

// ─── DashboardSerializer 核心文件操作 ────────────────────────────────

/** @brief 构造函数 @param parent 父对象 */
DashboardSerializer::DashboardSerializer(QObject* parent) : QObject(parent) {}

/** @brief 保存布局到JSON文件 @return true=成功 */
bool DashboardSerializer::saveToFile(const QString& filePath,
                                     const QString& name, int columns,
                                     const QList<DashboardItemConfig>& items)
{
    const QByteArray data = toJson(name, columns, items);
    if (data.isEmpty() && !items.isEmpty()) { ++m_totalErrors; ++m_serializationErrors; ++m_totalSerializationErrors; return false; }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = tr("无法打开文件写入: %1").arg(file.errorString());
        ++m_totalErrors; ++m_serializationErrors; ++m_totalSerializationErrors; return false;
    }
    file.write(data);
    if (!file.commit()) {
        m_lastError = tr("写入文件失败: %1").arg(file.errorString());
        ++m_totalErrors; ++m_serializationErrors; ++m_totalSerializationErrors; return false;
    }

    qCInfo(lcDashboardSerializer) << "布局已保存至:" << filePath;
    ++m_totalSaves; ++m_totalExports; ++m_totalSerializations;
    m_totalBytesSerialized += static_cast<quint64>(data.size());
    m_totalBytesWritten += static_cast<quint64>(data.size());
    m_maxProfileVersionSaved = qMax(m_maxProfileVersionSaved, kVersion);
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
        ++m_totalErrors; ++m_deserializationErrors; return false;
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        m_lastError = tr("文件为空: %1").arg(filePath);
        ++m_totalErrors; ++m_deserializationErrors; return false;
    }
    ++m_totalImports;
    m_totalBytesRead += static_cast<quint64>(data.size());
    m_totalBytesDeserialized += static_cast<quint64>(data.size());
    return loadFromJson(data, name, columns, items);
}

/** @brief 获取最近一次操作的错误描述 @return 错误信息字符串 */
QString DashboardSerializer::lastError() const { return m_lastError; }

/** @brief 获取当前序列化格式版本号 @return 版本号整数 */
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
        ++m_totalErrors; ++m_deserializationErrors;
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

// ─── QSettings配置文件管理见 DashboardSerializerProfile.cpp ───────

// ─── 统计接口 ───────────────────────────────────────────────────────

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
