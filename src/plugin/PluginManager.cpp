/**
 * @file PluginManager.cpp
 * @brief 插件管理器实现 — 动态加载/卸载第三方插件
 *
 * 使用 QLibrary 实现运行时插件发现与加载。
 * 导出符号约定: extern "C" IEmbedDebugPlugin* createPlugin()
 */

#include "plugin/PluginManager.h"
#include "plugin/PluginApi.h"

#include <QDir>
#include <QLibrary>

/**
 * @brief createPlugin 导出函数签名
 *
 * 每个插件 DLL 必须导出此符号:
 * @code
 * extern "C" IEmbedDebugPlugin* createPlugin() {
 *     return new MyPlugin();
 * }
 * @endcode
 */
using CreatePluginFunc = IEmbedDebugPlugin* (*)();

/**
 * @brief 构造函数
 *
 * 创建 PluginApi 实例供插件使用。
 *
 * @param parent 父对象
 */
PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , m_api(new PluginApi(this))
{
}

/** @brief 析构函数，自动卸载所有插件 */
PluginManager::~PluginManager()
{
    unloadAll();
}

/**
 * @brief 扫描目录中的插件文件
 *
 * 遍历指定目录，查找所有 DLL（Windows）或 SO（Linux）文件，
 * 尝试解析 createPlugin 导出符号以判断是否为合法插件。
 *
 * @param pluginDir 插件目录路径
 * @return 找到的插件候选文件路径列表
 */
QStringList PluginManager::scanPlugins(const QString& pluginDir)
{
    QStringList candidates;
    QDir dir(pluginDir);
    if (!dir.exists()) {
        emit pluginError(pluginDir, tr("插件目录不存在: %1").arg(pluginDir));
        return candidates;
    }

    /* 平台扩展名过滤 */
#ifdef Q_OS_WIN
    const QString filter = QStringLiteral("*.dll");
#else
    const QString filter = QStringLiteral("*.so");
#endif

    const QFileInfoList entries = dir.entryInfoList(
        QStringList() << filter, QDir::Files, QDir::Name);

    for (const QFileInfo& fi : entries) {
        const QString absPath = fi.absoluteFilePath();
        QLibrary lib(absPath);
        if (!lib.load()) {
            continue;
        }
        auto createFn = reinterpret_cast<CreatePluginFunc>(
            lib.resolve("createPlugin"));
        lib.unload();

        if (createFn) {
            candidates.append(absPath);
        }
    }

    return candidates;
}

/**
 * @brief 加载指定插件
 *
 * 加载 DLL，调用 createPlugin 导出函数获取插件接口，
 * 然后调用 initialize() 初始化插件。
 *
 * @param filePath 插件 DLL 文件路径
 * @return true 加载成功，false 加载失败
 */
bool PluginManager::loadPlugin(const QString& filePath)
{
    QLibrary* lib = new QLibrary(filePath, this);
    if (!lib->load()) {
        const QString err = lib->errorString();
        emit pluginError(filePath, tr("无法加载库: %1").arg(err));
        delete lib;
        return false;
    }

    auto createFn = reinterpret_cast<CreatePluginFunc>(
        lib->resolve("createPlugin"));
    if (!createFn) {
        emit pluginError(filePath, tr("未找到 createPlugin 导出符号"));
        lib->unload();
        delete lib;
        return false;
    }

    /* 调用工厂函数创建插件实例 */
    IEmbedDebugPlugin* pluginInstance = createFn();
    if (!pluginInstance) {
        emit pluginError(filePath, tr("createPlugin 返回空指针"));
        lib->unload();
        delete lib;
        return false;
    }

    /* 初始化插件（注入 API） */
    if (!pluginInstance->initialize(m_api)) {
        emit pluginError(filePath, tr("插件初始化失败: %1")
                              .arg(pluginInstance->name()));
        pluginInstance->shutdown();
        delete pluginInstance;
        lib->unload();
        delete lib;
        return false;
    }

    const QString plugName = pluginInstance->name();

    /* 若同名插件已存在，先卸载旧的 */
    if (m_plugins.contains(plugName)) {
        unloadPlugin(plugName);
    }

    m_plugins.insert(plugName, pluginInstance);
    /* 保存 QLibrary 指针以便后续卸载 */
    lib->setProperty("_embedPluginName", plugName);

    emit pluginLoaded(plugName);
    return true;
}

/**
 * @brief 卸载指定插件
 *
 * 调用插件的 shutdown() 方法，删除插件实例。
 * 注意: QLibrary 在 PluginManager 析构时随 QObject 树自动销毁，
 * 此处仅 unload。
 *
 * @param name 插件名称
 */
void PluginManager::unloadPlugin(const QString& name)
{
    IEmbedDebugPlugin* plug = m_plugins.value(name, nullptr);
    if (!plug) {
        return;
    }

    plug->shutdown();
    delete plug;
    m_plugins.remove(name);

    /* 查找并卸载对应的 QLibrary */
    const auto children = findChildren<QLibrary*>();
    for (QLibrary* lib : children) {
        if (lib->property("_embedPluginName").toString() == name) {
            lib->unload();
            lib->deleteLater();
            break;
        }
    }

    emit pluginUnloaded(name);
}

/**
 * @brief 卸载所有已加载的插件
 *
 * 遍历插件映射表，逐个调用 shutdown() 并删除。
 */
void PluginManager::unloadAll()
{
    /* 取出所有 key 避免迭代中修改容器 */
    const QStringList names = m_plugins.keys();
    for (const QString& name : names) {
        unloadPlugin(name);
    }
}

/**
 * @brief 获取所有已加载插件的名称列表
 * @return 插件名称列表
 */
QStringList PluginManager::loadedPluginNames() const
{
    return m_plugins.keys();
}

/**
 * @brief 获取指定插件的接口指针
 * @param name 插件名称
 * @return 插件接口指针，未找到返回 nullptr
 */
IEmbedDebugPlugin* PluginManager::plugin(const QString& name) const
{
    return m_plugins.value(name, nullptr);
}
