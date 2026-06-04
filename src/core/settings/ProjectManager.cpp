/**
 * @file ProjectManager.cpp
 * @brief 工程管理器实现
 */
#include "core/settings/ProjectManager.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>

static const char *kRecentGroup = "ProjectManager";
static const char *kRecentKey = "recentProjects";
static const int kMaxRecentCount = 10;

/** @brief 构造函数 - 从QSettings加载最近工程列表 @param parent 父对象 */
ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent), m_currentProject(ProjectConfig::createDefault())
{
    QSettings settings;
    settings.beginGroup(kRecentGroup);
    m_recentProjects = settings.value(kRecentKey).toStringList();
    settings.endGroup();
}

/** @brief 析构函数 - 持久化最近工程列表到QSettings */
ProjectManager::~ProjectManager()
{
    QSettings settings;
    settings.beginGroup(kRecentGroup);
    settings.setValue(kRecentKey, m_recentProjects);
    settings.endGroup();
}

/** @brief 创建新工程文件 @param name 工程名称 @param filePath 工程文件路径 @return true创建成功 */
bool ProjectManager::createProject(const QString &name, const QString &filePath)
{
    ProjectConfig config = ProjectConfig::createDefault();
    config.name = name;
    config.filePath = filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        ++m_saveErrorCount;
        emit errorOccurred(tr("无法创建工程文件: %1").arg(file.errorString()));
        return false;
    }
    QJsonDocument doc = ProjectConfig::toJson(config);
    if (file.write(doc.toJson()) < 0) {
        ++m_saveErrorCount;
        emit errorOccurred(tr("写入工程文件失败: %1").arg(file.errorString()));
        file.close();
        return false;
    }
    file.close();
    m_currentProject = config;
    addToRecent(filePath);
    emit projectLoaded(filePath);
    return true;
}

/** @brief 加载工程文件 @param filePath 工程文件路径 @return true加载成功 */
bool ProjectManager::loadProject(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("无法打开工程文件: %1").arg(file.errorString()));
        return false;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        ++m_saveErrorCount;
        emit errorOccurred(tr("工程文件JSON解析失败: %1 (偏移: %2)")
            .arg(parseError.errorString()).arg(parseError.offset));
        return false;
    }
    ++m_totalLoads;
    ProjectConfig config;
    config = ProjectConfig::fromJson(doc);
    config.filePath = filePath;
    m_currentProject = config;
    addToRecent(filePath);
    emit projectLoaded(filePath);
    return true;
}

/** @brief 保存当前工程到文件 @return true保存成功 */
bool ProjectManager::saveProject()
{
    if (m_currentProject.filePath.isEmpty()) return false;
    QFile file(m_currentProject.filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        ++m_saveErrorCount;
        return false;
    }
    QJsonDocument doc = ProjectConfig::toJson(m_currentProject);
    file.write(doc.toJson());
    file.close();
    ++m_totalSaves;
    emit projectSaved(m_currentProject.filePath);
    return true;
}

/** @brief 另存为指定路径 @param filePath 新文件路径 @return true保存成功 */
bool ProjectManager::saveProjectAs(const QString &filePath)
{
    m_currentProject.filePath = filePath;
    addToRecent(filePath);
    return saveProject();
}

/** @brief 关闭当前工程，重置为默认配置 */
void ProjectManager::closeProject()
{
    m_currentProject = ProjectConfig::createDefault();
    emit projectClosed();
}

/** @brief 获取当前工程配置 @return 当前工程配置 */
ProjectConfig ProjectManager::currentProject() const { return m_currentProject; }
/** @brief 获取最近打开工程列表 @return 文件路径列表 */
QStringList ProjectManager::recentProjects() const { return m_recentProjects; }

/** @brief 将文件路径添加到最近列表 @param filePath 工程文件路径 */
void ProjectManager::addToRecent(const QString &filePath)
{
    m_recentProjects.removeAll(filePath);
    m_recentProjects.prepend(filePath);
    while (m_recentProjects.size() > kMaxRecentCount)
        m_recentProjects.removeLast();
}

/** @brief 获取累计手动保存次数 @return 保存总次数 */
quint64 ProjectManager::totalSaves() const { return m_totalSaves; }
/** @brief 获取累计工程加载次数 @return 加载总次数 */
quint64 ProjectManager::totalLoads() const { return m_totalLoads; }
/** @brief 获取累计自动保存次数 @return 自动保存总次数 */
quint64 ProjectManager::totalAutoSaves() const { return m_totalAutoSaves; }
/** @brief 获取累计保存错误次数 @return 错误总次数 */
quint64 ProjectManager::saveErrorCount() const { return m_saveErrorCount; }
/** @brief 重置所有工程统计计数器(保存/加载/自动保存/错误)归零 */
void ProjectManager::resetProjectStatistics() { m_totalSaves = 0; m_totalLoads = 0; m_totalAutoSaves = 0; m_saveErrorCount = 0; }
