/**
 * @file ModbusMaster.cpp
 * @brief Modbus主站(客户端)实现 -- 配置/便捷方法/通用请求
 *
 * 构造函数初始化定时器和TCP套接字，配置接口管理RTU/TCP模式切换，
 * 便捷方法封装8种功能码调用。帧发送/接收/解析见 ModbusMasterProtocol.cpp。
 * 统计接口见 ModbusMasterStats.cpp。
 */
#include "protocol/modbus/ModbusMaster.h"

// ============================================================================
// 构造/析构
// ============================================================================

/** @brief 构造主站: 创建超时定时器+TCP套接字, 设置objectName */
ModbusMaster::ModbusMaster(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_tcpSocket(new QTcpSocket(this))
{
    setObjectName(QStringLiteral("ModbusMaster"));
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ModbusMaster::onTimeout);
    connectTcpSocket();
}

/** @brief 析构: 断开TCP连接 */
ModbusMaster::~ModbusMaster() {
    disconnectTcpSocket();
    if (m_tcpSocket) {
        m_tcpSocket->abort();
    }
}

// ============================================================================
// 配置接口
// ============================================================================

/** @brief 设置RTU串口连接(断开旧连接信号) @param connection IConnection指针 */
void ModbusMaster::setConnection(IConnection* connection) {
    if (m_connection) {
        disconnect(m_connection, &IConnection::dataReceived,
                   this, &ModbusMaster::onRawDataReceived);
    }
    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &IConnection::dataReceived,
                this, &ModbusMaster::onRawDataReceived);
    }
}

/** @brief 兼容旧接口: 同时设置连接和从站地址 */
void ModbusMaster::setConnection(IConnection* connection, int slave) {
    setSlaveAddress(slave);
    setConnection(connection);
}

/** @brief 设置从站地址 @param address 地址(1-247) */
void ModbusMaster::setSlaveAddress(int address) {
    m_slaveAddress = static_cast<quint8>(qBound(1, address, 247));
}

/** @brief 设置传输模式(TCP模式时自动断开/重连套接字) @param mode RTU或TCP */
void ModbusMaster::setMode(Mode mode) {
    if (m_mode == mode) return;
    m_mode = mode;
    /* TCP模式切换时重置缓冲区和事务ID */
    m_rxBuffer.clear();
    m_transactionId = 0;
}

/** @brief 设置超时毫秒数(最小10ms) @param ms 超时值 */
void ModbusMaster::setTimeout(int ms) {
    m_timeoutMs = qMax(10, ms);
}

/** @brief 设置RTU波特率 @param baudRate 波特率值 */
void ModbusMaster::setBaudRate(int baudRate) {
    m_baudRate = qMax(1200, baudRate);
}

/** @brief 设置TCP目标地址 @param host 主机名/IP @param port 端口号 */
void ModbusMaster::setTcpTarget(const QString& host, quint16 port) {
    m_tcpHost = host;
    m_tcpPort = port;
}

/** @brief 获取当前传输模式 @return Mode枚举 */
ModbusMaster::Mode ModbusMaster::mode() const { return m_mode; }

/** @brief 获取当前超时毫秒数 @return 超时值 */
int ModbusMaster::timeout() const { return m_timeoutMs; }

// ============================================================================
// 通用请求接口
// ============================================================================

/**
 * @brief 发送Modbus请求 -- 根据模式自动构建RTU或TCP ADU
 * @param functionCode 功能码(1/2/3/4/5/6/15/16)
 * @param startAddress 起始地址
 * @param quantity 数量
 * @param data 写入数据(读操作忽略)
 * @return true=请求已发送
 */
bool ModbusMaster::sendRequest(quint8 functionCode, quint16 startAddress,
                               quint16 quantity, const QByteArray& data) {
    QByteArray adu;
    if (m_mode == Mode::RTU) {
        adu = buildRtuAdu(m_slaveAddress, functionCode, startAddress, quantity, data);
    } else {
        adu = buildTcpAdu(functionCode, startAddress, quantity, data);
    }
    return sendFrame(adu);
}

// ============================================================================
// 功能码便捷方法
// ============================================================================

/** @brief FC01读线圈 @param start 起始地址 @param count 数量 @return true=已发送 */
bool ModbusMaster::readCoils(int start, int count) {
    return sendRequest(0x01, static_cast<quint16>(start), static_cast<quint16>(count));
}

/** @brief FC02读离散输入 @param start 起始地址 @param count 数量 @return true=已发送 */
bool ModbusMaster::readDiscreteInputs(int start, int count) {
    return sendRequest(0x02, static_cast<quint16>(start), static_cast<quint16>(count));
}

/** @brief FC03读保持寄存器 @param start 起始地址 @param count 数量 @return true=已发送 */
bool ModbusMaster::readHoldingRegisters(int start, int count) {
    return sendRequest(0x03, static_cast<quint16>(start), static_cast<quint16>(count));
}

/** @brief FC04读输入寄存器 @param start 起始地址 @param count 数量 @return true=已发送 */
bool ModbusMaster::readInputRegisters(int start, int count) {
    return sendRequest(0x04, static_cast<quint16>(start), static_cast<quint16>(count));
}

/** @brief FC05写单个线圈 @param addr 地址 @param on ON/OFF @return true=已发送 */
bool ModbusMaster::writeSingleCoil(int addr, bool on) {
    QByteArray data(2, '\0');
    data[0] = on ? static_cast<char>(0xFF) : '\0';
    data[1] = '\0';
    return sendRequest(0x05, static_cast<quint16>(addr), 1, data);
}

/** @brief FC06写单个寄存器 @param addr 地址 @param value 值 @return true=已发送 */
bool ModbusMaster::writeSingleRegister(int addr, quint16 value) {
    QByteArray data;
    data.append(static_cast<char>((value >> 8) & 0xFF));
    data.append(static_cast<char>(value & 0xFF));
    return sendRequest(0x06, static_cast<quint16>(addr), 1, data);
}

/** @brief FC15写多个线圈 @param start 起始地址 @param values 线圈列表 @return true=已发送 */
bool ModbusMaster::writeMultipleCoils(int start, const QList<bool>& values) {
    if (values.isEmpty() || values.size() > 1968) return false;
    /* 字节计算: 每8个线圈占1字节 */
    int byteCount = (values.size() + 7) / 8;
    QByteArray coils(byteCount, '\0');
    for (int i = 0; i < values.size(); ++i) {
        if (values[i]) {
            coils[i / 8] |= static_cast<char>(1 << (i % 8));
        }
    }
    QByteArray data;
    data.append(static_cast<char>(byteCount));
    data.append(coils);
    return sendRequest(0x0F, static_cast<quint16>(start),
                       static_cast<quint16>(values.size()), data);
}

/** @brief FC16写多个寄存器 @param start 起始地址 @param values 值列表 @return true=已发送 */
bool ModbusMaster::writeMultipleRegisters(int start, const QList<quint16>& values) {
    if (values.isEmpty() || values.size() > 123) return false;
    QByteArray data;
    data.append(static_cast<char>(values.size() * 2));
    for (quint16 val : values) {
        data.append(static_cast<char>((val >> 8) & 0xFF));
        data.append(static_cast<char>(val & 0xFF));
    }
    return sendRequest(0x10, static_cast<quint16>(start),
                       static_cast<quint16>(values.size()), data);
}

/** @brief 兼容旧接口: 发送自定义帧 */
bool ModbusMaster::sendCustomFrame(const ModbusFrame& frame) {
    if (m_mode == Mode::RTU) {
        return sendFrame(frameToBytes(frame));
    }
    /* TCP模式: 从ModbusFrame构建TCP ADU */
    QByteArray pdu;
    pdu.append(static_cast<char>(frame.function));
    if (frame.function == ModbusFunction::WriteSingleCoil ||
        frame.function == ModbusFunction::WriteSingleRegister) {
        pdu.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        pdu.append(static_cast<char>(frame.startAddress & 0xFF));
        pdu.append(frame.data);
    } else if (frame.function == ModbusFunction::WriteMultipleCoils ||
               frame.function == ModbusFunction::WriteMultipleRegisters) {
        pdu.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        pdu.append(static_cast<char>(frame.startAddress & 0xFF));
        pdu.append(static_cast<char>((frame.quantity >> 8) & 0xFF));
        pdu.append(static_cast<char>(frame.quantity & 0xFF));
        pdu.append(frame.data);
    } else {
        pdu.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        pdu.append(static_cast<char>(frame.startAddress & 0xFF));
        pdu.append(static_cast<char>((frame.quantity >> 8) & 0xFF));
        pdu.append(static_cast<char>(frame.quantity & 0xFF));
    }
    /* MBAP头: 事务ID(2)+协议ID(2=0)+长度(2)+单元ID(1) */
    quint16 txId = ++m_transactionId;
    m_lastTxId = txId;
    QByteArray mbap;
    mbap.append(static_cast<char>((txId >> 8) & 0xFF));
    mbap.append(static_cast<char>(txId & 0xFF));
    mbap.append('\0'); mbap.append('\0'); /* 协议ID=0 */
    quint16 len = static_cast<quint16>(pdu.size() + 1);
    mbap.append(static_cast<char>((len >> 8) & 0xFF));
    mbap.append(static_cast<char>(len & 0xFF));
    mbap.append(static_cast<char>(frame.slaveAddress));
    return sendFrame(mbap + pdu);
}

/** @brief 兼容旧接口: readRegisters默认FC03 */
bool ModbusMaster::readRegisters(int slave, int start, int count) {
    quint8 saved = m_slaveAddress;
    m_slaveAddress = static_cast<quint8>(slave);
    bool ok = readHoldingRegisters(start, count);
    m_slaveAddress = saved;
    return ok;
}

// ============================================================================
// ADU构建
// ============================================================================

/**
 * @brief 构建RTU ADU: 从站(1)+PDU+CRC(2)
 * @param slave 从站地址 @param fc 功能码 @param start 起始地址
 * @param qty 数量 @param data 写入数据
 * @return 完整ADU字节
 */
QByteArray ModbusMaster::buildRtuAdu(quint8 slave, quint8 fc,
                                     quint16 start, quint16 qty,
                                     const QByteArray& data) {
    QByteArray pdu;
    pdu.append(static_cast<char>(slave));
    pdu.append(static_cast<char>(fc));
    pdu.append(static_cast<char>((start >> 8) & 0xFF));
    pdu.append(static_cast<char>(start & 0xFF));

    if (fc == 0x05 || fc == 0x06) {
        /* 写单个: 起始地址+数据值(2字节) */
        pdu.append(data.isEmpty() ? QByteArray(2, '\0') : data);
    } else if (fc == 0x0F || fc == 0x10) {
        /* 写多个: 起始地址+数量(2)+数据 */
        pdu.append(static_cast<char>((qty >> 8) & 0xFF));
        pdu.append(static_cast<char>(qty & 0xFF));
        pdu.append(data);
    } else {
        /* 读操作: 起始地址+数量(2) */
        pdu.append(static_cast<char>((qty >> 8) & 0xFF));
        pdu.append(static_cast<char>(qty & 0xFF));
    }

    /* 追加CRC16 (低字节在前) */
    quint16 crcVal = crc16(pdu);
    pdu.append(static_cast<char>(crcVal & 0xFF));
    pdu.append(static_cast<char>((crcVal >> 8) & 0xFF));
    return pdu;
}

/**
 * @brief 构建TCP ADU: MBAP头(7)+PDU
 * MBAP: 事务ID(2)+协议ID(2=0)+长度(2)+单元ID(1)
 */
QByteArray ModbusMaster::buildTcpAdu(quint8 fc, quint16 start,
                                     quint16 qty, const QByteArray& data) {
    /* PDU: 功能码(1)+起始地址(2)+数量/数据 */
    QByteArray pdu;
    pdu.append(static_cast<char>(fc));
    pdu.append(static_cast<char>((start >> 8) & 0xFF));
    pdu.append(static_cast<char>(start & 0xFF));

    if (fc == 0x05 || fc == 0x06) {
        pdu.append(data.isEmpty() ? QByteArray(2, '\0') : data);
    } else if (fc == 0x0F || fc == 0x10) {
        pdu.append(static_cast<char>((qty >> 8) & 0xFF));
        pdu.append(static_cast<char>(qty & 0xFF));
        pdu.append(data);
    } else {
        pdu.append(static_cast<char>((qty >> 8) & 0xFF));
        pdu.append(static_cast<char>(qty & 0xFF));
    }

    quint16 txId = ++m_transactionId;
    m_lastTxId = txId;

    QByteArray mbap;
    mbap.append(static_cast<char>((txId >> 8) & 0xFF));
    mbap.append(static_cast<char>(txId & 0xFF));
    mbap.append('\0'); mbap.append('\0'); /* 协议ID=0 */
    quint16 len = static_cast<quint16>(pdu.size() + 1); /* PDU+单元ID */
    mbap.append(static_cast<char>((len >> 8) & 0xFF));
    mbap.append(static_cast<char>(len & 0xFF));
    mbap.append(static_cast<char>(m_slaveAddress));

    return mbap + pdu;
}

// ============================================================================
// TCP辅助
// ============================================================================

/** @brief 连接TCP套接字信号 */
void ModbusMaster::connectTcpSocket() {
    if (!m_tcpSocket) return;
    connect(m_tcpSocket, &QTcpSocket::readyRead,
            this, &ModbusMaster::onTcpReadyRead);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError err) {
                Q_UNUSED(err)
                emit communicationError(m_tcpSocket->errorString());
            });
}

/** @brief 断开TCP套接字信号 */
void ModbusMaster::disconnectTcpSocket() {
    if (!m_tcpSocket) return;
    disconnect(m_tcpSocket, &QTcpSocket::readyRead,
               this, &ModbusMaster::onTcpReadyRead);
}

// sendFrame/onRawDataReceived/onTcpReadyRead/onTimeout/parseRtuResponse/
// parseTcpResponse/handleResponse/isReadFunction/isWriteFunction/fcToIndex
// 已移至 ModbusMasterProtocol.cpp
