/**
 * @file PluginApi.cpp
 * @brief 插件 API 实现 — 宿主提供给插件的能力接口
 *
 * 插件通过此对象注册面板、添加通道、发送数据。
 * 实际发送动作由上层通过连接 dataSendRequested 信号完成。
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
 * 将插件提供的 QWidget 添加到面板列表并发出通知信号。
 * 宿主 UI 层监听 panelRegistered 信号来实际展示面板。
 *
 * @param name 面板名称（唯一标识）
 * @param panel 面板控件指针
 */
void PluginApi::registerPanel(const QString& name, QWidget* panel)
{
    if (!panel) {
        return;
    }
    m_panels.append(panel);
    emit panelRegistered(name, panel);
}

/**
 * @brief 添加数据通道
 *
 * 在通道列表中注册新通道名，宿主数据层监听此信号
 * 以创建对应的接收/发送通道。
 *
 * @param name 通道名称
 */
void PluginApi::addChannel(const QString& name)
{
    if (name.isEmpty()) {
        return;
    }
    m_channels.append(name);
    emit channelAdded(name);
}

/**
 * @brief 通过宿主发送数据
 *
 * 发出 dataSendRequested 信号，由上层 SendController 连接处理。
 *
 * @param data 要发送的字节数据
 * @return true 请求已发出（始终返回 true）
 */
bool PluginApi::sendData(const QByteArray& data)
{
    if (data.isEmpty()) {
        return false;
    }
    emit dataSendRequested(data);
    return true;
}

/**
 * @brief 订阅接收数据事件
 *
 * 插件调用此方法表示希望接收数据通知。
 * 后续宿主收到串口数据时，将通过 dataReceived 信号转发。
 *
 * 当前版本为标记式实现：上层连接 dataReceived 即可。
 */
void PluginApi::subscribeReceivedData()
{
    /* 标记式调用 — 宿主层通过连接 dataReceived 信号提供数据 */
}
