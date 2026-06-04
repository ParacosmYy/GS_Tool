/**
 * @file JLinkRttConnection.cpp
 * @brief J-Link RTT 连接实现 — IConnection 接口的 RTT 真实实现
 *
 * 通过 JLinkSdkLoader 单例调用 J-Link SDK 函数实现 RTT 通信。
 * 支持 SWD/JTAG 接口选择、速度配置和多通道数据读写。
 */

#include "rtt/JLinkRttConnection.h"
#include "rtt/JLinkSdkLoader.h"

/**
 * @brief 构造函数
 *
 * 初始化 RTT 连接状态，获取 SDK 加载器单例引用。
 *
 * @param parent 父对象
 */
JLinkRttConnection::JLinkRttConnection(QObject* parent)
    : IConnection(parent)
    , m_channel(0)
    , m_state(ConnectionState::Disconnected)
    , m_sdkLoader(JLinkSdkLoader::instance())
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
 * 完整连接流程:
 * 1. 确认 SDK 已加载（未加载则尝试自动加载）
 * 2. 根据配置选择调试接口（SWD/JTAG）
 * 3. 设置连接速度
 * 4. 连接到目标设备
 * 5. 启动 RTT 通信
 *
 * 任何步骤失败都会发出 errorOccurred 信号并返回 false。
 *
 * @return true 连接成功，false 连接失败
 */
bool JLinkRttConnection::open()
{
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    // 步骤 1: 确保 SDK 已加载
    if (!m_sdkLoader->isLoaded()) {
        if (!m_sdkLoader->load()) {
            ++m_errorCount;
            emit errorOccurred(tr("J-Link SDK 加载失败，请检查 JLinkARM.dll 是否可用"));
            return false;
        }
    }

    // 步骤 2: 选择调试接口类型
    const QString ifType = m_config.value(QStringLiteral("interface")).toString();
    int ifValue = 1;  // 默认 SWD
    if (ifType.compare(QStringLiteral("JTAG"), Qt::CaseInsensitive) == 0) {
        ifValue = 0;
    }
    if (!m_sdkLoader->selectInterface(ifValue)) {
        ++m_errorCount;
        emit errorOccurred(tr("选择调试接口失败（JTAG/SWD）"));
        return false;
    }

    // 步骤 3: 设置连接速度
    const int speed = m_config.value(QStringLiteral("speed")).toInt();
    if (speed > 0) {
        m_sdkLoader->setSpeed(speed);
    }

    // 步骤 4: 连接到目标设备
    const QString deviceId = m_config.value(QStringLiteral("deviceId")).toString();
    if (!m_sdkLoader->connectToDevice(deviceId)) {
        ++m_errorCount;
        emit errorOccurred(tr("连接目标设备失败: %1").arg(deviceId.isEmpty() ? tr("未指定设备") : deviceId));
        return false;
    }

    // 步骤 5: 启动 RTT 通信
    const int rttResult = m_sdkLoader->rttStart();
    if (rttResult != 0) {
        ++m_errorCount;
        emit errorOccurred(tr("启动 RTT 通信失败，错误码: %1").arg(rttResult));
        m_sdkLoader->disconnect();
        return false;
    }

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

/**
 * @brief 关闭 RTT 连接
 *
 * 依次停止 RTT 通信、断开设备连接，并将状态置为 Disconnected。
 */
void JLinkRttConnection::close()
{
    if (m_state == ConnectionState::Disconnected) {
        return;
    }

    // 停止 RTT 通信
    m_sdkLoader->rttStop();

    // 断开设备连接
    m_sdkLoader->disconnect();

    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
}

/**
 * @brief 向 RTT 通道写入数据
 *
 * 调用 JLinkSdkLoader::rttWrite() 向指定 RTT 通道写入数据。
 * 返回 SDK 报告的实际写入字节数。
 *
 * @param data 要发送的字节数据
 * @return 实际写入字节数，-1 表示失败
 */
qint64 JLinkRttConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        return -1;
    }

    ++m_totalWrites;
    const int written = m_sdkLoader->rttWrite(m_channel, data.constData(), data.size());
    if (written < 0) {
        ++m_errorCount;
        emit errorOccurred(tr("RTT 通道 %1 写入失败").arg(m_channel));
        return -1;
    }

    m_totalBytesWritten += static_cast<quint64>(written);
    emit bytesWritten(written);
    return written;
}

// configure/setChannel/channel/totalReads/totalWrites/totalBytesRead/totalBytesWritten/rttErrorCount/resetRttStatistics
// 已移至 JLinkRttConnectionStats.cpp
