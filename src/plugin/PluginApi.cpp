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
    ++m_sendCount;
    m_sendBytes += data.size();
    emit dataSendRequested(data);
    return true;
}

/**
 * @brief 订阅接收数据事件
 *
 * 插件调用此方法表示希望接收数据通知。
 * 后续宿主收到串口数据时，将通过 dataReceived 信号转发。
 */
void PluginApi::subscribeReceivedData()
{
    m_subscribed = true;
}

/**
 * @brief 取消订阅接收数据事件
 *
 * 插件调用此方法停止接收数据通知。
 * 取消订阅后宿主不再通过 dataReceived 信号转发数据。
 */
void PluginApi::unsubscribeReceivedData()
{
    m_subscribed = false;
}

/**
 * @brief 查询是否已订阅数据接收
 * @return true 已订阅，false 未订阅
 */
bool PluginApi::isSubscribed() const
{
    return m_subscribed;
}

/**
 * @brief 获取所有已注册的面板列表
 * @return 面板控件指针列表
 */
QList<QWidget*> PluginApi::registeredPanels() const
{
    return m_panels;
}

/**
 * @brief 获取所有已添加的通道名称列表
 * @return 通道名称列表
 */
QStringList PluginApi::channels() const
{
    return m_channels;
}

/**
 * @brief 获取已注册面板数量
 * @return 面板数量
 */
int PluginApi::panelCount() const
{
    return m_panels.count();
}

/**
 * @brief 获取已添加通道数量
 * @return 通道数量
 */
int PluginApi::channelCount() const
{
    return m_channels.count();
}

/**
 * @brief 获取已发送数据次数
 */
quint64 PluginApi::sendDataCount() const
{
    return m_sendCount;
}

/**
 * @brief 获取已发送字节总数
 */
qint64 PluginApi::totalBytesSent() const
{
    return m_sendBytes;
}

/**
 * @brief 重置发送统计
 */
void PluginApi::resetSendStatistics()
{
    m_sendCount = 0;
    m_sendBytes = 0;
}
