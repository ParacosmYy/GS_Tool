/**
 * @file IEmbedDebugPlugin.h
 * @brief 插件抽象接口 — 定义第三方插件的扩展契约
 *
 * 设计要点:
 *   1. 纯虚接口，无Q_OBJECT，无实现代码
 *   2. 插件DLL必须导出 embedDebugPluginInit(PluginApi*) 函数
 *   3. 插件通过 PluginApi 访问宿主能力（注册面板、添加通道、发送数据）
 *   4. 生命周期: load → initialize → [运行] → shutdown → unload
 *
 * 协作关系:
 *   - PluginManager: 负责加载/卸载插件DLL
 *   - PluginApi: 提供宿主能力的接口对象
 */

#ifndef IEMBEDDEBUGPLUGIN_H
#define IEMBEDDEBUGPLUGIN_H

#include <QString>

class PluginApi;

/**
 * @brief 插件抽象接口
 *
 * 所有第三方插件必须实现此接口。
 * 插件以动态链接库形式存在，由 PluginManager 在运行时加载。
 *
 * 最小实现示例:
 * @code
 * class MyPlugin : public IEmbedDebugPlugin {
 *     QString name() const override { return "MyPlugin"; }
 *     QString version() const override { return "1.0.0"; }
 *     QString description() const override { return "示例插件"; }
 *     bool initialize(PluginApi* api) override { return true; }
 *     void shutdown() override {}
 * };
 * @endcode
 */
class IEmbedDebugPlugin {
public:
    /** @brief 虚析构函数 */
    virtual ~IEmbedDebugPlugin() = default;

    /**
     * @brief 获取插件名称（必须唯一）
     * @return 插件名称字符串
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取插件版本号
     * @return 版本字符串（建议语义化版本格式，如 "1.0.0"）
     */
    virtual QString version() const = 0;

    /**
     * @brief 获取插件描述
     * @return 插件功能的简短描述
     */
    virtual QString description() const = 0;

    /**
     * @brief 初始化插件
     *
     * 在插件被加载后调用。插件应通过 api 参数注册面板、
     * 添加通道或订阅数据事件。
     *
     * @param api 宿主提供的API接口（非空，生命周期由PluginManager管理）
     * @return true 初始化成功，false 初始化失败（插件将被卸载）
     */
    virtual bool initialize(PluginApi* api) = 0;

    /**
     * @brief 关闭插件
     *
     * 在插件被卸载前调用。插件应释放所有资源、断开信号连接。
     * 此方法返回后，PluginManager 将卸载DLL。
     */
    virtual void shutdown() = 0;
};

#endif // IEMBEDDEBUGPLUGIN_H
