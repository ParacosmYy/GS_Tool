/**
 * @file PluginApi.h
 * @brief 插件 API — 宿主提供给插件的接口对象
 *
 * 插件通过此 API 与宿主程序交互，包括注册自定义面板、
 * 添加数据通道、发送数据等功能。
 *
 * 协作关系:
 *   - IEmbedDebugPlugin: 插件通过 initialize() 获取此 API
 *   - PluginManager: 创建和管理此 API 的生命周期
 */
#ifndef PLUGINAPI_H
#define PLUGINAPI_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QWidget>

/**
 * @brief 插件 API
 *
 * 宿主程序提供给插件的能力接口。
 * 每个加载的插件通过 initialize() 获取此对象的指针。
 */
class PluginApi : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit PluginApi(QObject* parent = nullptr);

    /**
     * @brief 注册自定义面板
     * @param name 面板名称（必须唯一）
     * @param panel 面板控件指针（宿主接管显示）
     */
    void registerPanel(const QString& name, QWidget* panel);

    /**
     * @brief 添加数据通道
     * @param name 通道名称
     */
    void addChannel(const QString& name);

    /**
     * @brief 通过宿主发送数据
     * @param data 要发送的数据
     * @return true 发送成功，false 发送失败
     */
    bool sendData(const QByteArray& data);

    /** @brief 订阅接收数据事件 */
    void subscribeReceivedData();

signals:
    /** @brief 接收到数据信号（插件订阅后触发） */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 面板已注册信号
     * @param name 面板名称
     * @param panel 面板控件指针
     */
    void panelRegistered(const QString& name, QWidget* panel);

    /**
     * @brief 通道已添加信号
     * @param name 通道名称
     */
    void channelAdded(const QString& name);

private:
    QList<QWidget*> m_panels;       ///< 已注册的面板列表
    QStringList m_channels;         ///< 已添加的通道名称列表
};

#endif // PLUGINAPI_H
