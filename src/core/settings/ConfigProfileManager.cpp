/**
 * @file ConfigProfileManager.cpp
 * @brief 设备配置档案管理器实现
 *
 * 实现档案的创建/更新/删除/切换/搜索/导入导出/持久化。
 * 使用QJsonDocument进行序列化，QStandardPaths定位存储目录。
 */

#include "core/settings/ConfigProfileManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QSettings>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

// ── 单例实现 (Q_GLOBAL_STATIC线程安全) ──

/** @brief 单例持有者，确保线程安全初始化 */
static struct ConfigProfileManagerHolder {
    ConfigProfileManager* instance = nullptr;

    ConfigProfileManagerHolder() {
        instance = new ConfigProfileManager(QCoreApplication::instance());
        instance->load();
    }
} s_holder;

/** @brief 获取全局唯一实例 @return 档案管理器引用 */
ConfigProfileManager& ConfigProfileManager::instance()
{
    return *s_holder.instance;
}

// ── 构造函数 ──

/**
 * @brief 构造函数 (私有，单例模式)
 * @param parent 父对象
 */
ConfigProfileManager::ConfigProfileManager(QObject* parent)
    : QObject(parent)
    , m_currentIndex(-1)
{
    // 确定存储目录路径
    m_storagePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(m_storagePath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
}

// ── 档案CRUD ──

/**
 * @brief 创建新档案
 * @param profile 档案数据
 * @return 新档案索引，-1表示失败(名称重复)
 *
 * 验证名称唯一性，自动设置createdAt/lastUsed时间戳，
 * 追加到列表末尾，发射profileCreated信号并自动保存。
 */
int ConfigProfileManager::createProfile(const DeviceProfile& profile)
{
    // 验证名称唯一性
    if (profile.name.isEmpty()) {
        return -1;
    }
    if (findByName(profile.name) >= 0) {
        return -1; // 名称重复
    }

    DeviceProfile p = profile;
    p.createdAt = QDateTime::currentDateTime();
    p.lastUsed = p.createdAt;

    m_profiles.append(p);

    // 更新统计
    ++m_stats.totalProfilesCreated;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());

    int newIndex = m_profiles.size() - 1;
    emit profileCreated(newIndex);

    save();
    return newIndex;
}

/**
 * @brief 更新已有档案
 * @param index 档案索引
 * @param profile 新的档案数据
 * @return 是否成功
 *
 * 验证索引范围，更新档案数据，发射profileUpdated信号并保存。
 * 如果新名称与其他档案冲突则拒绝更新。
 */
bool ConfigProfileManager::updateProfile(int index, const DeviceProfile& profile)
{
    if (index < 0 || index >= m_profiles.size()) {
        return false;
    }

    // 如果名称发生变化，检查新名称是否与其他档案冲突
    if (profile.name != m_profiles[index].name) {
        int conflictIdx = findByName(profile.name);
        if (conflictIdx >= 0 && conflictIdx != index) {
            return false; // 新名称与其他档案冲突
        }
    }

    // 保留原始createdAt
    QDateTime originalCreatedAt = m_profiles[index].createdAt;
    m_profiles[index] = profile;
    m_profiles[index].createdAt = originalCreatedAt;

    emit profileUpdated(index);

    // 如果更新的是当前活跃档案，也发射切换信号以通知外部刷新
    if (index == m_currentIndex) {
        emit profileSwitched(index);
    }

    save();
    return true;
}

/**
 * @brief 删除档案
 * @param index 档案索引
 * @return 是否成功
 *
 * 验证索引范围，移除档案，调整currentIndex，
 * 发射profileRemoved信号并保存。
 */
bool ConfigProfileManager::removeProfile(int index)
{
    if (index < 0 || index >= m_profiles.size()) {
        return false;
    }

    m_profiles.removeAt(index);

    // 更新统计
    ++m_stats.totalProfilesDeleted;

    // 调整当前活跃索引
    if (m_currentIndex == index) {
        // 删除的是当前活跃档案，重置为-1
        m_currentIndex = -1;
        m_stats.activeProfileIndex = -1;
    } else if (m_currentIndex > index) {
        // 删除的档案在当前活跃档案之前，索引前移
        --m_currentIndex;
        m_stats.activeProfileIndex = m_currentIndex;
    }

    emit profileRemoved(index);
    save();
    return true;
}

/**
 * @brief 获取指定索引的档案
 * @param index 档案索引
 * @return 档案数据 (索引无效返回默认构造的DeviceProfile)
 */
ConfigProfileManager::DeviceProfile ConfigProfileManager::profile(int index) const
{
    if (index < 0 || index >= m_profiles.size()) {
        return DeviceProfile{};
    }
    return m_profiles[index];
}

/**
 * @brief 获取所有档案
 * @return 档案列表
 */
QList<ConfigProfileManager::DeviceProfile> ConfigProfileManager::allProfiles() const
{
    return m_profiles;
}

// ── 档案切换 ──

/**
 * @brief 获取当前活跃档案索引
 * @return 索引，-1表示无活跃档案
 */
int ConfigProfileManager::currentIndex() const
{
    return m_currentIndex;
}

/**
 * @brief 切换到指定档案
 * @param index 目标档案索引
 * @return 是否成功
 *
 * 验证索引范围，更新当前索引和lastUsed时间戳，
 * 发射profileSwitched信号。
 */
bool ConfigProfileManager::switchToProfile(int index)
{
    if (index < 0 || index >= m_profiles.size()) {
        return false;
    }

    m_currentIndex = index;
    m_profiles[index].lastUsed = QDateTime::currentDateTime();

    // 更新统计
    ++m_stats.totalSwitches;
    m_stats.activeProfileIndex = index;

    emit profileSwitched(index);
    save();
    return true;
}

/**
 * @brief 获取当前活跃档案
 * @return 当前档案数据 (无活跃档案返回默认构造)
 */
ConfigProfileManager::DeviceProfile ConfigProfileManager::currentProfile() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_profiles.size()) {
        return DeviceProfile{};
    }
    return m_profiles[m_currentIndex];
}

// ── 搜索 ──

/**
 * @brief 按名称查找档案
 * @param name 档案名称
 * @return 档案索引，-1表示未找到
 *
 * 线性搜索匹配第一个同名档案。
 */
int ConfigProfileManager::findByName(const QString& name) const
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].name == name) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 按设备类型查找档案
 * @param type 设备类型
 * @return 匹配的档案列表
 *
 * 过滤所有匹配指定设备类型的档案。
 */
QList<ConfigProfileManager::DeviceProfile> ConfigProfileManager::findByDeviceType(const QString& type) const
{
    QList<DeviceProfile> result;
    for (const auto& p : m_profiles) {
        if (p.deviceType == type) {
            result.append(p);
        }
    }
    return result;
}

// ── 导入导出 ──

/**
 * @brief 导出单个档案到JSON文件
 * @param index 档案索引
 * @param filePath 目标文件路径
 * @return 是否成功
 *
 * 将档案序列化为JSON格式写入文件。
 * JSON结构: { "name", "description", "deviceType", "serial", "protocol",
 *             "customFields", "createdAt", "lastUsed" }
 */
bool ConfigProfileManager::exportProfile(int index, const QString& filePath)
{
    if (index < 0 || index >= m_profiles.size()) {
        return false;
    }

    QJsonObject root = deviceProfileToJson(m_profiles[index]);
    // 添加导出标记，便于导入时识别
    root[QStringLiteral("exportVersion")] = 1;
    root[QStringLiteral("exportSource")] = QStringLiteral("EmbedDebug");

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_stats.totalExports;
    return true;
}

/**
 * @brief 从JSON文件导入档案
 * @param filePath 源文件路径
 * @return 是否成功
 *
 * 读取JSON文件，反序列化为DeviceProfile，追加到列表。
 * 如果名称冲突则自动添加后缀 "_imported_序号"。
 */
bool ConfigProfileManager::importProfile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }
    if (!doc.isObject()) {
        return false;
    }

    DeviceProfile prof = jsonToDeviceProfile(doc.object());

    // 处理名称冲突: 添加后缀
    QString baseName = prof.name;
    int suffix = 1;
    while (findByName(prof.name) >= 0) {
        prof.name = baseName + QStringLiteral("_imported_%1").arg(suffix);
        ++suffix;
    }

    // 设置新的时间戳
    prof.createdAt = QDateTime::currentDateTime();
    prof.lastUsed = prof.createdAt;

    m_profiles.append(prof);

    // 更新统计
    ++m_stats.totalImports;
    ++m_stats.totalProfilesCreated;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());

    int newIndex = m_profiles.size() - 1;
    emit profileCreated(newIndex);

    save();
    return true;
}

/**
 * @brief 导出所有档案到指定目录
 * @param dirPath 目标目录路径
 * @return 是否成功
 *
 * 每个档案导出为独立的JSON文件，文件名为 "profile_<名称>.json"。
 * 自动创建目标目录。
 */
bool ConfigProfileManager::exportAll(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }

    for (int i = 0; i < m_profiles.size(); ++i) {
        // 文件名使用档案名称，替换不合法字符
        QString safeName = m_profiles[i].name;
        safeName.replace(QStringLiteral(" "), QStringLiteral("_"));
        safeName.replace(QRegularExpression(QStringLiteral("[<>:\"/\\\\|?*]")),
                         QStringLiteral("_"));
        QString filePath = dir.filePath(
            QStringLiteral("profile_%1.json").arg(safeName));

        if (!exportProfile(i, filePath)) {
            return false;
        }
    }

    return true;
}

// ── 持久化 ──

/**
 * @brief 保存所有档案到磁盘
 * @return 是否成功
 *
 * 将所有档案序列化为JSON数组，写入AppData目录下的配置文件。
 */
bool ConfigProfileManager::save()
{
    QJsonArray arr;
    for (const auto& prof : m_profiles) {
        arr.append(deviceProfileToJson(prof));
    }

    // 根对象包含档案数组和元数据
    QJsonObject root;
    root[QStringLiteral("profiles")] = arr;
    root[QStringLiteral("currentIndex")] = m_currentIndex;
    root[QStringLiteral("version")] = 1;

    QJsonDocument doc(root);

    QString path = profileFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/**
 * @brief 从磁盘加载所有档案
 * @return 是否成功
 *
 * 从AppData目录读取配置文件，反序列化所有档案。
 * 文件不存在时视为空列表，返回true。
 */
bool ConfigProfileManager::load()
{
    QString path = profileFilePath();
    QFile file(path);
    if (!file.exists()) {
        // 首次运行，无配置文件，视为正常
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray arr = root[QStringLiteral("profiles")].toArray();

    m_profiles.clear();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            m_profiles.append(jsonToDeviceProfile(val.toObject()));
        }
    }

    // 恢复当前活跃索引
    m_currentIndex = root[QStringLiteral("currentIndex")].toInt(-1);
    if (m_currentIndex < 0 || m_currentIndex >= m_profiles.size()) {
        m_currentIndex = -1;
    }

    // 更新统计
    ++m_stats.totalProfilesLoaded;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());
    m_stats.activeProfileIndex = m_currentIndex;

    return true;
}

// ── 统计 ──

/**
 * @brief 获取运行时统计信息
 * @return 统计数据常引用
 */
const ConfigProfileManager::Stats& ConfigProfileManager::stats() const
{
    return m_stats;
}

// ── 序列化辅助 ──

/**
 * @brief 将SerialConfig序列化为JSON对象
 * @param config 串口配置
 * @return JSON对象
 */
QJsonObject ConfigProfileManager::serialConfigToJson(const SerialConfig& config) const
{
    QJsonObject obj;
    obj[QStringLiteral("portName")] = config.portName;
    obj[QStringLiteral("baudRate")] = config.baudRate;
    obj[QStringLiteral("dataBits")] = config.dataBits;
    obj[QStringLiteral("stopBits")] = config.stopBits;
    obj[QStringLiteral("parity")] = config.parity;
    obj[QStringLiteral("flowControl")] = config.flowControl;
    return obj;
}

/**
 * @brief 从JSON对象反序列化SerialConfig
 * @param obj JSON对象
 * @return 串口配置
 */
ConfigProfileManager::SerialConfig ConfigProfileManager::jsonToSerialConfig(const QJsonObject& obj) const
{
    SerialConfig config;
    config.portName = obj[QStringLiteral("portName")].toString();
    config.baudRate = static_cast<qint32>(obj[QStringLiteral("baudRate")].toInt(115200));
    config.dataBits = obj[QStringLiteral("dataBits")].toInt(8);
    config.stopBits = obj[QStringLiteral("stopBits")].toInt(1);
    config.parity = obj[QStringLiteral("parity")].toString(QStringLiteral("None"));
    config.flowControl = obj[QStringLiteral("flowControl")].toString(QStringLiteral("None"));
    return config;
}

/**
 * @brief 将ProtocolConfig序列化为JSON对象
 * @param config 协议配置
 * @return JSON对象
 */
QJsonObject ConfigProfileManager::protocolConfigToJson(const ProtocolConfig& config) const
{
    QJsonObject obj;
    obj[QStringLiteral("protocolName")] = config.protocolName;
    obj[QStringLiteral("headerPattern")] = QString::fromLatin1(config.headerPattern.toHex());
    obj[QStringLiteral("footerPattern")] = QString::fromLatin1(config.footerPattern.toHex());
    obj[QStringLiteral("frameLength")] = config.frameLength;

    // 序列化自定义设置为JSON对象
    QJsonObject customObj;
    for (auto it = config.customSettings.constBegin();
         it != config.customSettings.constEnd(); ++it) {
        customObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    obj[QStringLiteral("customSettings")] = customObj;
    return obj;
}

/**
 * @brief 从JSON对象反序列化ProtocolConfig
 * @param obj JSON对象
 * @return 协议配置
 */
ConfigProfileManager::ProtocolConfig ConfigProfileManager::jsonToProtocolConfig(const QJsonObject& obj) const
{
    ProtocolConfig config;
    config.protocolName = obj[QStringLiteral("protocolName")].toString();
    config.headerPattern = QByteArray::fromHex(
        obj[QStringLiteral("headerPattern")].toString().toLatin1());
    config.footerPattern = QByteArray::fromHex(
        obj[QStringLiteral("footerPattern")].toString().toLatin1());
    config.frameLength = obj[QStringLiteral("frameLength")].toInt(0);

    // 反序列化自定义设置
    QJsonObject customObj = obj[QStringLiteral("customSettings")].toObject();
    for (auto it = customObj.constBegin(); it != customObj.constEnd(); ++it) {
        config.customSettings.insert(it.key(), it.value().toVariant());
    }
    return config;
}

/**
 * @brief 将DeviceProfile序列化为JSON对象
 * @param prof 设备档案
 * @return JSON对象
 */
QJsonObject ConfigProfileManager::deviceProfileToJson(const DeviceProfile& prof) const
{
    QJsonObject obj;
    obj[QStringLiteral("name")] = prof.name;
    obj[QStringLiteral("description")] = prof.description;
    obj[QStringLiteral("deviceType")] = prof.deviceType;
    obj[QStringLiteral("serial")] = serialConfigToJson(prof.serial);
    obj[QStringLiteral("protocol")] = protocolConfigToJson(prof.protocol);

    // 序列化自定义字段
    QJsonObject customObj;
    for (auto it = prof.customFields.constBegin();
         it != prof.customFields.constEnd(); ++it) {
        customObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    obj[QStringLiteral("customFields")] = customObj;

    // 时间戳使用ISO格式字符串
    obj[QStringLiteral("createdAt")] = prof.createdAt.toString(Qt::ISODate);
    obj[QStringLiteral("lastUsed")] = prof.lastUsed.toString(Qt::ISODate);
    return obj;
}

/**
 * @brief 从JSON对象反序列化DeviceProfile
 * @param obj JSON对象
 * @return 设备档案
 */
ConfigProfileManager::DeviceProfile ConfigProfileManager::jsonToDeviceProfile(const QJsonObject& obj) const
{
    DeviceProfile prof;
    prof.name = obj[QStringLiteral("name")].toString();
    prof.description = obj[QStringLiteral("description")].toString();
    prof.deviceType = obj[QStringLiteral("deviceType")].toString();
    prof.serial = jsonToSerialConfig(obj[QStringLiteral("serial")].toObject());
    prof.protocol = jsonToProtocolConfig(obj[QStringLiteral("protocol")].toObject());

    // 反序列化自定义字段
    QJsonObject customObj = obj[QStringLiteral("customFields")].toObject();
    for (auto it = customObj.constBegin(); it != customObj.constEnd(); ++it) {
        prof.customFields.insert(it.key(), it.value().toVariant());
    }

    // 解析时间戳
    prof.createdAt = QDateTime::fromString(
        obj[QStringLiteral("createdAt")].toString(), Qt::ISODate);
    prof.lastUsed = QDateTime::fromString(
        obj[QStringLiteral("lastUsed")].toString(), Qt::ISODate);

    // 兜底: 如果时间戳解析失败则使用当前时间
    if (!prof.createdAt.isValid()) {
        prof.createdAt = QDateTime::currentDateTime();
    }
    if (!prof.lastUsed.isValid()) {
        prof.lastUsed = prof.createdAt;
    }

    return prof;
}

/**
 * @brief 获取持久化文件路径
 * @return AppData目录下的档案文件路径
 */
QString ConfigProfileManager::profileFilePath() const
{
    return QDir(m_storagePath).filePath(QStringLiteral("config_profiles.json"));
}
