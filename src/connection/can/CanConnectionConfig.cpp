/**
 * @file CanConnectionConfig.cpp
 * @brief CAN连接配置方法 — 参数配置/波特率/位时序/FD模式
 *
 * 从 CanConnection.cpp 拆分而来，集中管理CAN连接的配置变更方法，
 * 包括configure()参数映射解析、波特率动态切换、CAN-FD模式设置和
 * 位时序参数更新。
 *
 * @see CanConnection.cpp — 核心生命周期(open/close/write)
 * @see CanConnectionFrame.cpp — 帧发送和帧过滤
 * @see CanConnectionProtocol.cpp — DBC/信号解码/命令/接收解析/统计
 */

#include "connection/can/CanConnection.h"

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
