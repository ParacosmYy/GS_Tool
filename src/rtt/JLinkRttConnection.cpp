/**
 * @file JLinkRttConnection.cpp
 * @brief J-Link RTT 连接实现 — IConnection 接口的 RTT 连接桩实现
 *
 * 提供 RTT 连接的状态管理、通道配置和桩级别的数据读写。
 * 实际 J-Link SDK 调用将在后续迭代中集成。
 */

#include "rtt/JLinkRttConnection.h"

/**
 * @brief 构造函数
 *
 * 初始化 RTT 连接状态，设置对象名称用于 QSS 样式匹配。
 *
 * @param parent 父对象
 */
JLinkRttConnection::JLinkRttConnection(QObject* parent)
    : IConnection(parent)
    , m_channel(0)
    , m_state(ConnectionState::Disconnected)
{
    setObjectName(QStringLiteral("JLinkRttConnection"));
}

/**
 * @brief 析构函数
 *
 * 析构时自动关闭连接，释放资源。
 */
JLinkRttConnection::~JLinkRttConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return 固定返回 ConnectionType::Rtt
 */
ConnectionType JLinkRttConnection::type() const
{
    return ConnectionType::Rtt;
}

/**
 * @brief 获取连接名称
 * @return 格式 "RTT CH<通道号>"，如 "RTT CH0"
 */
QString JLinkRttConnection::name() const
{
    return tr("RTT CH%1").arg(m_channel);
}

/**
 * @brief 获取当前连接状态
 * @return 连接状态枚举值
 */
ConnectionState JLinkRttConnection::state() const
{
    return m_state;
}

/**
 * @brief 打开 RTT 连接
 *
 * 桩实现：设置状态为 Connected 并发出状态变更信号。
 * 实际 J-Link SDK 连接逻辑将在后续迭代中实现。
 *
 * @return true（桩实现始终成功）
 */
bool JLinkRttConnection::open()
{
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    // TODO: 调用 JLinkSdkLoader 加载 SDK
    // TODO: 连接 J-Link 调试器并启动 RTT 通信

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

/**
 * @brief 关闭 RTT 连接
 *
 * 设置状态为 Disconnected 并发出状态变更信号。
 * 实际 J-Link SDK 断开逻辑将在后续迭代中实现。
 */
void JLinkRttConnection::close()
{
    if (m_state == ConnectionState::Disconnected) {
        return;
    }

    // TODO: 调用 J-Link SDK 停止 RTT 并断开连接

    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
}

/**
 * @brief 向 RTT 通道写入数据
 *
 * 桩实现：返回数据大小，表示所有数据已被"发送"。
 *
 * @param data 要发送的字节数据
 * @return 实际写入字节数（桩实现返回 data.size()）
 */
qint64 JLinkRttConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        return -1;
    }

    // TODO: 调用 J-Link SDK 写入 RTT 缓冲区
    qint64 written = data.size();
    emit bytesWritten(written);
    return written;
}

/**
 * @brief 配置 RTT 连接参数
 *
 * 解析并存储配置参数。支持 deviceId、interface、speed、channel。
 * 如果参数中包含 channel 键，自动更新通道号。
 *
 * @param params 参数键值对
 */
void JLinkRttConnection::configure(const QVariantMap& params)
{
    m_config = params;

    if (params.contains(QStringLiteral("channel"))) {
        bool ok = false;
        const int ch = params.value(QStringLiteral("channel")).toInt(&ok);
        if (ok && ch >= 0 && ch <= 15) {
            m_channel = ch;
        }
    }
}

/**
 * @brief 设置 RTT 通道号
 * @param ch 通道号（0-15），超出范围不做修改
 */
void JLinkRttConnection::setChannel(int ch)
{
    if (ch >= 0 && ch <= 15) {
        m_channel = ch;
    }
}

/**
 * @brief 获取当前 RTT 通道号
 * @return 通道号
 */
int JLinkRttConnection::channel() const
{
    return m_channel;
}
