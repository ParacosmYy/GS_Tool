/**
 * @file PluginApi.cpp
 * @brief 插件 API 实现 — 骨架文件
 */

#include "plugin/PluginApi.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PluginApi::PluginApi(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 注册自定义面板
 *
 * 将插件提供的 QWidget 注册到宿主的面板系统中。
 *
 * @param name 面板名称
 * @param panel 面板控件指针
 */
void PluginApi::registerPanel(const QString& name, QWidget* panel)
{
    Q_UNUSED(name)
    Q_UNUSED(panel)
    // TODO: 注册面板并发出 panelRegistered 信号
}

/**
 * @brief 添加数据通道
 * @param name 通道名称
 */
void PluginApi::addChannel(const QString& name)
{
    Q_UNUSED(name)
    // TODO: 添加通道并发出 channelAdded 信号
}

/**
 * @brief 通过宿主发送数据
 * @param data 要发送的数据
 * @return true 发送成功，false 发送失败
 */
bool PluginApi::sendData(const QByteArray& data)
{
    Q_UNUSED(data)
    // TODO: 通过宿主的 SendController 发送数据
    return false;
}

/**
 * @brief 订阅接收数据事件
 *
 * 注册后，每当宿主收到数据时将发出 dataReceived 信号。
 */
void PluginApi::subscribeReceivedData()
{
    // TODO: 连接宿主的 dataReceived 信号到本对象的 dataReceived 信号
}
