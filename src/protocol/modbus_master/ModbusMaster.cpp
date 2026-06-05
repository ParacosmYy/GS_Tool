/**
 * @file ModbusMaster.cpp
 * @brief Modbus Master轮询调度器实现 -- RTU帧构建/解析/轮询/重试/超时
 *
 * 构造函数初始化双定时器(轮询+超时)，buildFrame构建RTU ADU，
 * parseResponse解析RTU响应，processNextPoll驱动轮询队列。
 * 统计接口见 ModbusMasterStats.cpp。
 */
#include "protocol/modbus_master/ModbusMaster.h"

// ============================================================================
// 构造/析构
// ============================================================================

/** @brief 构造轮询调度器: 创建轮询+超时定时器 */
ModbusMasterPoller::ModbusMasterPoller(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_timeoutTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("ModbusMasterPoller"));
    m_pollTimer->setSingleShot(false);
    m_timeoutTimer->setSingleShot(true);
    connect(m_pollTimer, &QTimer::timeout, this, &ModbusMasterPoller::onPollTick);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ModbusMasterPoller::onTimeout);
}

/** @brief 析构: 停止定时器 */
ModbusMasterPoller::~ModbusMasterPoller() {
    stopPolling();
    m_timeoutTimer->stop();
}

// ============================================================================
// 配置接口
// ============================================================================

/** @brief 设置串口连接(断开旧信号/连接新信号) @param connection IConnection指针 */
void ModbusMasterPoller::setConnection(IConnection* connection) {
    if (m_connection) {
        disconnect(m_connection, &IConnection::dataReceived,
                   this, &ModbusMasterPoller::onDataReceived);
    }
    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &IConnection::dataReceived,
                this, &ModbusMasterPoller::onDataReceived);
    }
}

/** @brief 设置默认超时(最小10ms) @param ms 超时毫秒数 */
void ModbusMasterPoller::setTimeout(int ms) { m_timeoutMs = qMax(10, ms); }

/** @brief 设置最大重试次数 @param count 次数(0=不重试) */
void ModbusMasterPoller::setMaxRetries(int count) { m_maxRetries = qMax(0, count); }

/** @brief 设置轮询间隔(最小50ms) @param ms 间隔毫秒数 */
void ModbusMasterPoller::setPollInterval(int ms) { m_pollIntervalMs = qMax(50, ms); }

int ModbusMasterPoller::timeout() const     { return m_timeoutMs; }
int ModbusMasterPoller::maxRetries() const  { return m_maxRetries; }
int ModbusMasterPoller::pollInterval() const{ return m_pollIntervalMs; }

// ============================================================================
// 请求接口
// ============================================================================

/** @brief 发送单次请求(不加入轮询队列) @param req 请求 @return true=已发送 */
bool ModbusMasterPoller::sendRequest(const ModbusMasterRequest& req) {
    if (m_waiting) return false;
    m_pendingReq = req;
    m_retryCount = 0;
    QByteArray frame = buildFrame(req);
    if (!transmitFrame(frame)) return false;
    m_timeoutTimer->start(m_timeoutMs);
    emit requestSent(req);
    return true;
}

/** @brief 添加轮询条目(周期性执行) @param req 请求 */
void ModbusMasterPoller::addPollEntry(const ModbusMasterRequest& req) {
    m_pollEntries.enqueue(req);
}

/** @brief 清空所有轮询条目 */
void ModbusMasterPoller::clearPollEntries() {
    m_pollEntries.clear();
    m_pollIndex = 0;
}

/** @brief 启动轮询 */
void ModbusMasterPoller::startPolling() {
    if (m_pollEntries.isEmpty()) return;
    m_pollTimer->start(m_pollIntervalMs);
}

/** @brief 停止轮询 */
void ModbusMasterPoller::stopPolling() {
    m_pollTimer->stop();
    m_timeoutTimer->stop();
    m_waiting = false;
}

/** @brief 轮询是否运行中 @return true=运行中 */
bool ModbusMasterPoller::isPolling() const { return m_pollTimer->isActive(); }

// ============================================================================
// 帧构建
// ============================================================================

/** @brief 从请求结构体构建RTU帧(地址+功能码+数据+CRC16) @param req 请求 @return 完整ADU */
QByteArray ModbusMasterPoller::buildFrame(const ModbusMasterRequest& req) {
    QByteArray pdu;
    pdu.append(static_cast<char>(req.slaveAddr));
    pdu.append(static_cast<char>(req.func));
    pdu.append(static_cast<char>((req.startReg >> 8) & 0xFF));
    pdu.append(static_cast<char>(req.startReg & 0xFF));

    switch (req.func) {
    case ModbusMasterFunction::WriteSingleCoil:
        pdu.append(req.values.isEmpty() ? QByteArray(2, '\0') :
            (req.values[0] ? QByteArray("\xFF\x00", 2) : QByteArray(2, '\0')));
        break;
    case ModbusMasterFunction::WriteSingleRegister:
        if (!req.values.isEmpty()) {
            pdu.append(static_cast<char>((req.values[0] >> 8) & 0xFF));
            pdu.append(static_cast<char>(req.values[0] & 0xFF));
        }
        break;
    case ModbusMasterFunction::WriteMultipleRegisters:
        pdu.append(static_cast<char>((req.count >> 8) & 0xFF));
        pdu.append(static_cast<char>(req.count & 0xFF));
        pdu.append(static_cast<char>(req.values.size() * 2));
        for (quint16 v : req.values) {
            pdu.append(static_cast<char>((v >> 8) & 0xFF));
            pdu.append(static_cast<char>(v & 0xFF));
        }
        break;
    default: /* FC01-04: 数量字段 */
        pdu.append(static_cast<char>((req.count >> 8) & 0xFF));
        pdu.append(static_cast<char>(req.count & 0xFF));
        break;
    }

    quint16 crcVal = crc16(pdu);
    pdu.append(static_cast<char>(crcVal & 0xFF));
    pdu.append(static_cast<char>((crcVal >> 8) & 0xFF));
    return pdu;
}

// ============================================================================
// 响应解析
// ============================================================================

/** @brief 解析RTU响应帧(地址+功能码+数据+CRC校验+寄存器提取) @param raw 原始帧 @return 响应结构体 */
ModbusMasterResponse ModbusMasterPoller::parseResponse(const QByteArray& raw) {
    ModbusMasterResponse resp;
    if (raw.size() < 5) {
        resp.success = false;
        resp.error = tr("帧过短(%1字节)").arg(raw.size());
        return resp;
    }
    /* CRC校验 */
    QByteArray payload = raw.left(raw.size() - 2);
    quint16 recvCrc = static_cast<quint8>(raw[raw.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(raw[raw.size() - 1])) << 8);
    if (crc16(payload) != recvCrc) {
        resp.success = false;
        resp.error = tr("CRC校验失败");
        return resp;
    }

    resp.slaveAddr = static_cast<quint8>(raw[0]);
    quint8 fc = static_cast<quint8>(raw[1]);

    /* 异常响应 */
    if (fc & 0x80) {
        resp.success = false;
        resp.func = static_cast<ModbusMasterFunction>(fc & 0x7F);
        if (raw.size() >= 5) {
            quint8 errCode = static_cast<quint8>(raw[2]);
            resp.error = tr("Modbus异常: FC%1 错误码=%2").arg(fc & 0x7F).arg(errCode);
        }
        return resp;
    }

    resp.func = static_cast<ModbusMasterFunction>(fc);
    resp.startReg = m_pendingReq.startReg;

    /* FC01-04读响应: func(1)+byteCount(1)+data(N) */
    if (fc >= 0x01 && fc <= 0x04) {
        if (raw.size() >= 4) {
            quint8 byteCount = static_cast<quint8>(raw[2]);
            for (int i = 0; i + 1 < byteCount; i += 2) {
                quint16 val = (static_cast<quint8>(raw[3 + i]) << 8) |
                              static_cast<quint8>(raw[3 + i + 1]);
                resp.values.append(val);
            }
        }
        resp.success = true;
    }
    /* FC05/06写响应: 回显地址+值 */
    else if (fc == 0x05 || fc == 0x06) {
        resp.success = true;
    }
    /* FC16写多寄存器响应 */
    else if (fc == 0x10) {
        resp.success = true;
    }

    return resp;
}

// ============================================================================
// 帧发送与接收
// ============================================================================

/** @brief 通过IConnection发送帧 @param frame 完整RTU帧 @return true=写入成功 */
bool ModbusMasterPoller::transmitFrame(const QByteArray& frame) {
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        emit errorOccurred(tr("串口未连接"));
        return false;
    }
    qint64 written = m_connection->write(frame);
    if (written > 0) {
        m_bytesSent += static_cast<quint64>(written);
        m_waiting = true;
        return true;
    }
    emit errorOccurred(tr("发送失败"));
    return false;
}

/** @brief 串口数据到达: 缓冲+帧定界+解析 */
void ModbusMasterPoller::onDataReceived(const QByteArray& data) {
    if (!m_waiting) return;
    m_rxBuffer.append(data);
    m_bytesReceived += static_cast<quint64>(data.size());

    if (m_rxBuffer.size() > 512) {
        m_rxBuffer.clear();
        return;
    }
    if (m_rxBuffer.size() < 5) return;

    quint8 fc = static_cast<quint8>(m_rxBuffer[1]);
    int expectedLen = -1;

    if (fc & 0x80) {
        expectedLen = 5; /* 异常响应: slave+func|0x80+err+CRC(2) */
    } else if (fc == 0x05 || fc == 0x06 || fc == 0x10) {
        expectedLen = 8; /* 写响应: slave+func+addr(2)+val(2)+CRC(2) */
    } else if (fc >= 0x01 && fc <= 0x04) {
        if (m_rxBuffer.size() >= 3) {
            int byteCount = static_cast<quint8>(m_rxBuffer[2]);
            expectedLen = 3 + byteCount + 2;
        }
    }

    if (expectedLen > 0 && m_rxBuffer.size() >= expectedLen) {
        m_timeoutTimer->stop();
        ModbusMasterResponse resp = parseResponse(m_rxBuffer.left(expectedLen));
        m_rxBuffer.clear();
        m_waiting = false;

        if (resp.success) {
            ++m_totalResponses;
        } else {
            ++m_totalErrors;
        }
        emit responseReceived(resp);
    }
}

// ============================================================================
// 超时与重试
// ============================================================================

/** @brief 超时处理: 重试或报告错误 */
void ModbusMasterPoller::onTimeout() {
    ++m_totalTimeouts;
    if (m_retryCount < m_maxRetries) {
        ++m_retryCount;
        ++m_totalRetries;
        QByteArray frame = buildFrame(m_pendingReq);
        if (transmitFrame(frame)) {
            m_timeoutTimer->start(m_timeoutMs);
            return;
        }
    }
    m_waiting = false;
    ModbusMasterResponse resp;
    resp.success = false;
    resp.error = tr("请求超时(重试%1次)").arg(m_retryCount);
    resp.slaveAddr = m_pendingReq.slaveAddr;
    resp.func = m_pendingReq.func;
    emit responseReceived(resp);
    emit errorOccurred(resp.error);
}

// ============================================================================
// 轮询调度
// ============================================================================

/** @brief 轮询定时器触发: 发送队列中下一个请求 */
void ModbusMasterPoller::onPollTick() {
    if (m_waiting || m_pollEntries.isEmpty()) return;
    if (m_pollIndex >= m_pollEntries.size()) m_pollIndex = 0;
    ModbusMasterRequest req = m_pollEntries[m_pollIndex];
    ++m_pollIndex;
    sendRequest(req);
}
