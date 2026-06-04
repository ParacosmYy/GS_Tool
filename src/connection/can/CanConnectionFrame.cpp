/**
 * @file CanConnectionFrame.cpp
 * @brief CAN连接 — 帧发送/构建与帧过滤逻辑(拆分自CanConnection.cpp)
 *
 * 实现:
 *   - sendFrame: 发送经典CAN帧(最长8字节)到总线
 *   - sendFdFrame: 发送CAN-FD帧(最长64字节)到总线
 *   - addFilter: 添加帧过滤器(ID掩码+匹配模式)
 *   - clearFilters: 清除所有帧过滤器
 *   - filters: 获取当前帧过滤器列表
 *   - acceptsFilter: 检查帧ID是否通过过滤器
 */

#include "connection/can/CanConnection.h"
#include "connection/can/CanFrameParser.h"

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
