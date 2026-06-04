/**
 * @file CanConnection.cpp
 * @brief CAN/CAN-FD连接实现 — 基于串口CAN适配器(LAWICEL/SLCAN协议)
 *
 * 支持经典CAN(8字节)和CAN-FD(64字节)帧收发，
 * 提供位时序配置、帧过滤(ID掩码+匹配)、DBC信号解码。
 */

#include "connection/can/CanConnection.h"
#include "connection/can/CanFrameParser.h"
#include "protocol/can/DbcParser.h"
#include <QSerialPortInfo>

/** @brief 构造函数，初始化CAN连接基类 @param parent 父对象指针 */
CanConnection::CanConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，关闭连接释放DBC解析器 */
CanConnection::~CanConnection()
{
    close();
    delete m_dbcParser;
    m_dbcParser = nullptr;
}

/** @brief 获取连接类型 @return 固定返回ConnectionType::Can */
ConnectionType CanConnection::type() const
{
    return ConnectionType::Can;
}

/** @brief 获取连接显示名称 @return 适配器名称，未配置时返回"未配置" */
QString CanConnection::name() const
{
    return m_adapterName.isEmpty() ? tr("未配置") : m_adapterName;
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState CanConnection::state() const
{
    return m_state;
}

/** @brief 设置底层串口连接，用于CAN适配器通信 @param serialPort 串口连接实例指针 */
void CanConnection::setSerialPort(IConnection* serialPort)
{
    if (m_serialPort) {
        disconnect(m_serialPort, &IConnection::dataReceived,
                   this, &CanConnection::onSerialDataReceived);
    }
    m_serialPort = serialPort;
    if (m_serialPort) {
        connect(m_serialPort, &IConnection::dataReceived,
                this, &CanConnection::onSerialDataReceived);
    }
}

/** @brief 获取当前串口连接实例 @return 底层串口连接指针 */
IConnection* CanConnection::serialPort() const
{
    return m_serialPort;
}

/** @brief 打开CAN连接，执行LAWICEL初始化序列(关闭→设置波特率→打开) @return 成功返回true */
bool CanConnection::open()
{
    if (!m_serialPort) {
        emit errorOccurred(tr("未设置底层串口连接"));
        ++m_totalErrors;
        return false;
    }
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);

    /* 打开串口 */
    if (!m_serialPort->open()) {
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("串口打开失败"));
        ++m_totalErrors;
        return false;
    }

    /* LAWICEL: 关闭CAN → 设置波特率 → 打开CAN */
    sendCommand("C");   // 关闭
    sendCommand(bitrateToCommand(m_bitTiming.baudrate));

    /* CAN-FD模式 */
    if (m_canFdEnabled) {
        sendCommand(QString("#") + QChar(0x11));  // 启用FD
    }

    /* 设置帧过滤(如果已配置) */
    if (!m_filters.isEmpty()) {
        /* LAWICEL过滤通过M命令设置: MxxxxAAAA(掩码+匹配) */
        for (const CanFilter& f : m_filters) {
            QString maskStr = QStringLiteral("%1")
                .arg(f.mask, 4, 16, QLatin1Char('0')).toUpper();
            QString idStr = QStringLiteral("%1")
                .arg(f.id, 4, 16, QLatin1Char('0')).toUpper();
            sendCommand("M" + maskStr + idStr);
        }
    }

    sendCommand("O");   // 打开CAN通道

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

/** @brief 关闭CAN连接，发送LAWICEL关闭命令并断开 */
void CanConnection::close()
{
    if (m_state == ConnectionState::Disconnected) return;
    if (m_serialPort && m_serialPort->state() == ConnectionState::Connected) {
        sendCommand("C");  // LAWICEL关闭CAN
    }
    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
}

/** @brief 向底层串口写入原始数据 @param data 待发送的字节数据 @return 实际写入字节数，失败返回-1 */
qint64 CanConnection::write(const QByteArray& data)
{
    if (!m_serialPort || m_state != ConnectionState::Connected) {
        ++m_totalErrors;
        return -1;
    }

    qint64 written = m_serialPort->write(data);

    if (written > 0) {
        ++m_totalFramesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_totalErrors;
    }
    return written;
}

/** @brief 通过参数映射配置CAN连接属性(波特率/FD模式/位时序/过滤器) @param params 配置参数键值对 */
void CanConnection::configure(const QVariantMap& params)
{
    if (params.contains("bitrate")) {
        m_bitTiming.baudrate = params.value("bitrate").toInt();
    }
    if (params.contains("canFd")) {
        m_canFdEnabled = params.value("canFd").toBool();
    }
    if (params.contains("adapter")) {
        m_adapterName = params.value("adapter").toString();
    }
    if (params.contains("samplePoint")) {
        m_bitTiming.samplePoint = params.value("samplePoint").toDouble();
    }
    if (params.contains("sjw")) {
        m_bitTiming.sjw = params.value("sjw").toInt();
    }
    if (params.contains("filters")) {
        const QVariantList filterList = params.value("filters").toList();
        for (const QVariant& fv : filterList) {
            const QVariantMap fm = fv.toMap();
            CanFilter f;
            f.id = static_cast<quint32>(fm.value("id").toUInt());
            f.mask = static_cast<quint32>(fm.value("mask").toUInt());
            f.extended = fm.value("extended").toBool();
            addFilter(f);
        }
    }
}

/** @brief 设置CAN总线波特率，已连接时动态切换 @param bitrate 波特率值(bps) */
void CanConnection::setBitrate(int bitrate)
{
    m_bitTiming.baudrate = bitrate;
    /* 如果已连接，动态切换波特率 */
    if (m_state == ConnectionState::Connected && m_serialPort) {
        sendCommand("C");
        sendCommand(bitrateToCommand(bitrate));
        sendCommand("O");
    }
}

/** @brief 设置CAN-FD模式启用状态 @param enabled true启用CAN-FD */
void CanConnection::setCanFdEnabled(bool enabled)
{
    m_canFdEnabled = enabled;
}

/** @brief 设置位时序配置(波特率/采样点/SJW) @param timing 位时序参数 */
void CanConnection::setBitTiming(const CanBitTiming& timing)
{
    m_bitTiming = timing;
}

/** @brief 获取当前位时序配置 @return 位时序参数 */
CanBitTiming CanConnection::bitTiming() const
{
    return m_bitTiming;
}

/** @brief 发送CAN帧到总线 @param id 帧ID @param data 帧数据 @param extended 是否使用扩展帧格式 @return 发送成功返回true */
bool CanConnection::sendFrame(int id, const QByteArray& data, bool extended)
{
    if (m_state != ConnectionState::Connected) {
        ++m_totalErrors;
        return false;
    }

    /* 数据长度检查: 经典CAN最多8字节 */
    if (data.size() > 8) {
        ++m_totalErrors;
        ++m_totalFrameErrors;  ///< 帧构建失败(数据过长)
        return false;
    }

    CanFrame frame;
    frame.id = static_cast<quint32>(id);
    frame.data = data;
    frame.extended = extended;
    frame.rtr = false;
    frame.fd = false;
    frame.dlc = static_cast<quint8>(data.size());

    CanFrameParser parser;
    QByteArray raw = parser.buildFrame(frame);
    if (raw.isEmpty()) {
        ++m_totalErrors;
        ++m_totalFrameErrors;  ///< 帧构建失败(序列化错误)
        return false;
    }

    raw.append('\r');  // LAWICEL命令以\r结尾
    qint64 written = m_serialPort ? m_serialPort->write(raw) : 0;

    if (written > 0) {
        ++m_totalFramesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_totalErrors;
    }
    return written > 0;
}

/** @brief 发送CAN-FD帧到总线(最长64字节) @param id 帧ID @param data 帧数据(最长64字节) @param extended 是否使用扩展帧格式 @return 发送成功返回true */
bool CanConnection::sendFdFrame(int id, const QByteArray& data, bool extended)
{
    if (m_state != ConnectionState::Connected) {
        ++m_totalErrors;
        return false;
    }
    if (!m_canFdEnabled) {
        ++m_totalErrors;
        return false;
    }
    if (data.size() > 64) {
        ++m_totalErrors;
        ++m_totalFrameErrors;  ///< 帧构建失败(FD数据过长)
        return false;
    }

    CanFrame frame;
    frame.id = static_cast<quint32>(id);
    frame.data = data;
    frame.extended = extended;
    frame.rtr = false;
    frame.fd = true;
    frame.dlc = static_cast<quint8>(data.size());

    CanFrameParser parser;
    QByteArray raw = parser.buildFrame(frame);
    if (raw.isEmpty()) {
        ++m_totalErrors;
        ++m_totalFrameErrors;  ///< 帧构建失败(FD序列化错误)
        return false;
    }

    raw.append('\r');
    qint64 written = m_serialPort ? m_serialPort->write(raw) : 0;

    if (written > 0) {
        ++m_totalFramesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_totalErrors;
    }
    return written > 0;
}

/** @brief 添加帧过滤器 @param filter 过滤器(含ID/掩码/扩展标志) */
void CanConnection::addFilter(const CanFilter& filter)
{
    m_filters.append(filter);
}

/** @brief 清除所有帧过滤器 */
void CanConnection::clearFilters()
{
    m_filters.clear();
}

/** @brief 获取当前帧过滤器列表 @return 过滤器列表 */
QList<CanFilter> CanConnection::filters() const
{
    return m_filters;
}

/** @brief 检查帧ID是否通过过滤器 @param id 帧ID @param extended 是否扩展帧 @return true=通过(允许接收) */
bool CanConnection::acceptsFilter(quint32 id, bool extended) const
{
    /* 无过滤器时接收所有帧 */
    if (m_filters.isEmpty()) {
        return true;
    }

    for (const CanFilter& f : m_filters) {
        /* 扩展标志必须匹配 */
        if (f.extended != extended) {
            continue;
        }
        /* 掩码匹配: (id & mask) == (filterId & mask) */
        if ((id & f.mask) == (f.id & f.mask)) {
            return true;
        }
    }
    return false;
}


// DBC/信号解码/命令发送/接收解析/统计重置见 CanConnectionProtocol.cpp
