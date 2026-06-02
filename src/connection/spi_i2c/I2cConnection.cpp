/**
 * @file I2cConnection.cpp
 * @brief I2C总线连接实现 - 通过串口桥接协议
 */

#include "connection/spi_i2c/I2cConnection.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
I2cConnection::I2cConnection(QObject* parent)
    : IConnection(parent)
{
}

/**
 * @brief 析构函数
 */
I2cConnection::~I2cConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return Serial类型(I2C归入串行总线)
 */
ConnectionType I2cConnection::type() const
{
    return ConnectionType::Serial;
}

/**
 * @brief 获取连接显示名称
 * @return "I2C:适配器@地址" 格式
 */
QString I2cConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return QString("I2C:%1@0x%2")
            .arg(m_adapterDevice)
            .arg(m_deviceAddress, 2, 16, QChar('0'));
    }
    return tr("I2C (未连接)");
}

/**
 * @brief 获取当前状态
 */
ConnectionState I2cConnection::state() const
{
    return m_state;
}

/**
 * @brief 设置底层串口传输通道
 * @param serial 串口IConnection实例
 */
void I2cConnection::setTransport(IConnection* serial)
{
    if (m_serial) {
        disconnect(m_serial, nullptr, this, nullptr);
    }
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &IConnection::dataReceived,
                this, &I2cConnection::onTransportData);
    }
}

/**
 * @brief 打开I2C连接 - 通过串口桥接器初始化
 * @return true=成功
 */
bool I2cConnection::open()
{
    if (!m_serial) {
        emit errorOccurred(tr("未设置串口传输通道"));
        updateState(ConnectionState::Error);
        return false;
    }

    if (!m_serial->open()) {
        emit errorOccurred(tr("串口打开失败"));
        updateState(ConnectionState::Error);
        return false;
    }

    /// 发送I2C配置命令
    QByteArray configPayload;
    configPayload.append(static_cast<char>(m_deviceAddress));
    /// 时钟频率4字节小端
    configPayload.append(static_cast<char>(m_clockSpeed & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 8) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 16) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 24) & 0xFF));

    sendCommand(CMD_I2C_CONFIG, configPayload);
    updateState(ConnectionState::Connected);
    return true;
}

/**
 * @brief 关闭I2C连接
 */
void I2cConnection::close()
{
    updateState(ConnectionState::Disconnected);
}

/**
 * @brief 发送数据(I2C写操作)
 * @param data 待发送数据
 * @return 发送字节数
 */
qint64 I2cConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) return -1;
    return sendCommand(CMD_I2C_WRITE, data);
}

/**
 * @brief 配置I2C参数
 * @param params 参数映射
 */
void I2cConnection::configure(const QVariantMap& params)
{
    if (params.contains("deviceAddress")) {
        m_deviceAddress = params["deviceAddress"].toInt();
    }
    if (params.contains("adapter")) {
        m_adapterDevice = params["adapter"].toString();
    }
    if (params.contains("clockSpeed")) {
        m_clockSpeed = params["clockSpeed"].toInt();
    }
}

/**
 * @brief 扫描I2C总线
 * @return 发现的设备地址列表
 *
 * 向适配器发送扫描命令，遍历标准地址范围0x03~0x77。
 * 适配器对每个地址尝试ACK，有响应的地址加入结果列表。
 */
QList<int> I2cConnection::scanBus()
{
    QList<int> found;
    if (m_state != ConnectionState::Connected || !m_serial) {
        return found;
    }

    /// 发送扫描命令
    QByteArray scanPayload;
    scanPayload.append(static_cast<char>(0x03));  ///< 起始地址
    scanPayload.append(static_cast<char>(0x77));  ///< 结束地址
    sendCommand(CMD_I2C_SCAN, scanPayload);

    /// 解析响应中的ACK地址列表
    if (m_responseBuffer.size() >= 3) {
        /// 响应格式: [CMD][LEN][addr1, addr2, ...]
        quint16 len = static_cast<quint8>(m_responseBuffer[1]) |
                      (static_cast<quint8>(m_responseBuffer[2]) << 8);
        int dataStart = 3;
        for (int i = 0; i < len && (dataStart + i) < m_responseBuffer.size(); ++i) {
            int addr = static_cast<quint8>(m_responseBuffer[dataStart + i]);
            found.append(addr);
            emit deviceFound(addr);
        }
    }

    return found;
}

/**
 * @brief 从寄存器读取数据
 * @param deviceAddr 设备地址
 * @param regAddr 寄存器地址
 * @param length 读取长度
 * @return 读取的数据
 *
 * I2C读时序: [START][ADDR+W][REG][RESTART][ADDR+R][DATA...][STOP]
 */
QByteArray I2cConnection::readRegister(int deviceAddr, int regAddr, int length)
{
    QByteArray data;
    if (m_state != ConnectionState::Connected || !m_serial) {
        emit registerRead(regAddr, data);
        return data;
    }

    m_responseBuffer.clear();

    /// 构建并发送I2C读命令帧
    QByteArray frame = buildReadFrame(deviceAddr, regAddr, length);
    m_serial->write(frame);

    /// 解析响应数据
    if (m_responseBuffer.size() >= 3) {
        quint16 len = static_cast<quint8>(m_responseBuffer[1]) |
                      (static_cast<quint8>(m_responseBuffer[2]) << 8);
        int dataStart = 3;
        int avail = qMin(static_cast<int>(len), m_responseBuffer.size() - dataStart);
        if (avail > 0) {
            data = m_responseBuffer.mid(dataStart, avail);
        }
    }

    emit registerRead(regAddr, data);
    return data;
}

/**
 * @brief 向寄存器写入数据
 * @param deviceAddr 设备地址
 * @param regAddr 寄存器地址
 * @param data 待写入数据
 * @return true=成功
 *
 * I2C写时序: [START][ADDR+W][REG][DATA...][STOP]
 */
bool I2cConnection::writeRegister(int deviceAddr, int regAddr, const QByteArray& data)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        return false;
    }

    QByteArray frame = buildWriteFrame(deviceAddr, regAddr, data);
    qint64 written = m_serial->write(frame);
    return written > 0;
}

/**
 * @brief 底层串口数据到达回调
 * @param data 从串口适配器收到的响应数据
 */
void I2cConnection::onTransportData(const QByteArray& data)
{
    m_responseBuffer.append(data);
}

/**
 * @brief 更新连接状态
 */
void I2cConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/**
 * @brief 发送协议命令帧
 * @param cmd 命令字节
 * @param payload 负载数据
 * @return 发送字节数
 */
qint64 I2cConnection::sendCommand(quint8 cmd, const QByteArray& payload)
{
    if (!m_serial) return -1;

    QByteArray frame;
    frame.append(static_cast<char>(cmd));
    quint16 len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload);

    return m_serial->write(frame);
}

/**
 * @brief 构建I2C读命令帧
 * @param deviceAddr 设备7位地址
 * @param regAddr 寄存器地址
 * @param length 读取长度
 * @return 完整协议帧
 */
QByteArray I2cConnection::buildReadFrame(int deviceAddr, int regAddr, int length)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(static_cast<char>(length));
    return QByteArray().append(static_cast<char>(CMD_I2C_READ))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/**
 * @brief 构建I2C写命令帧
 * @param deviceAddr 设备7位地址
 * @param regAddr 寄存器地址
 * @param data 写入数据
 * @return 完整协议帧
 */
QByteArray I2cConnection::buildWriteFrame(int deviceAddr, int regAddr, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(data);
    return QByteArray().append(static_cast<char>(CMD_I2C_WRITE))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}
